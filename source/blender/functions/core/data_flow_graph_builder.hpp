#pragma once

#include "function.hpp"
#include "source_info.hpp"

#include "BLI_optional.hpp"
#include "BLI_small_set_vector.hpp"
#include "BLI_multipool.hpp"
#include "BLI_multimap.hpp"

namespace FN {

class DFGB_Socket;
class DFGB_Node;
class DataFlowGraphBuilder;
class CompactDataFlowGraph;

using DFGB_SocketSet = SmallSet<DFGB_Socket>;
using DFGB_SocketVector = SmallVector<DFGB_Socket>;
using DFGB_SocketSetVector = SmallSetVector<DFGB_Socket>;

class DFGB_Socket {
 public:
  DFGB_Socket(DFGB_Node *node, bool is_output, uint index)
      : m_node(node), m_is_output(is_output), m_index(index)
  {
  }

  inline DFGB_Node *node() const;
  inline bool is_input() const;
  inline bool is_output() const;
  inline uint index() const;
  inline DataFlowGraphBuilder &builder();

  SharedType &type() const;
  std::string name() const;

  inline Optional<DFGB_Socket> origin();
  inline ArrayRef<DFGB_Socket> targets();
  inline bool is_linked();

  friend bool operator==(const DFGB_Socket &a, const DFGB_Socket &b);
  friend std::ostream &operator<<(std::ostream &stream, DFGB_Socket socket);

 private:
  DFGB_Node *m_node;
  bool m_is_output;
  uint m_index;
};

class DFGB_Node {
 public:
  DFGB_Node(DataFlowGraphBuilder &builder, SharedFunction fn, SourceInfo *source)
      : m_builder(builder), m_function(fn), m_source(source)
  {
  }

  DataFlowGraphBuilder &builder() const;
  Signature &signature();
  SharedFunction &function();

  DFGB_Socket input(uint index);
  DFGB_Socket output(uint index);

  uint input_amount() const;
  uint output_amount() const;

  SourceInfo *source() const;

  class SocketIt {
   private:
    DFGB_Node *m_node;
    bool m_is_output;
    uint m_index;

   public:
    SocketIt(DFGB_Node *node, bool is_output, uint index)
        : m_node(node), m_is_output(is_output), m_index(index)
    {
    }

    SocketIt begin() const
    {
      return SocketIt(m_node, m_is_output, 0);
    }
    SocketIt end() const
    {
      Signature &sig = m_node->signature();
      uint size = m_is_output ? sig.outputs().size() : sig.inputs().size();
      return SocketIt(m_node, m_is_output, size);
    }

    SocketIt &operator++()
    {
      m_index++;
      return *this;
    }

    bool operator!=(const SocketIt &other) const
    {
      BLI_assert(m_is_output == other.m_is_output);
      BLI_assert(m_node == other.m_node);
      return m_index != other.m_index;
    }

    DFGB_Socket operator*() const
    {
      return DFGB_Socket(m_node, m_is_output, m_index);
    }
  };

  SocketIt inputs()
  {
    return SocketIt(this, false, 0);
  }

  SocketIt outputs()
  {
    return SocketIt(this, true, 0);
  }

 private:
  DataFlowGraphBuilder &m_builder;
  SharedFunction m_function;
  SourceInfo *m_source;
};

class DFGB_Link {
 public:
  DFGB_Link(DFGB_Socket from, DFGB_Socket to) : m_from(from), m_to(to)
  {
    BLI_assert(from.is_output());
    BLI_assert(to.is_input());
  }

  DFGB_Socket from()
  {
    return m_from;
  }

  DFGB_Socket to()
  {
    return m_to;
  }

  friend bool operator==(const DFGB_Link &a, const DFGB_Link &b)
  {
    return a.m_from == b.m_from && a.m_to == b.m_to;
  }

 private:
  DFGB_Socket m_from, m_to;
};

class DataFlowGraphBuilder {
 public:
  DataFlowGraphBuilder();
  DataFlowGraphBuilder(DataFlowGraphBuilder &other) = delete;

  DFGB_Node *insert_function(SharedFunction &fn, SourceInfo *source = nullptr);
  void insert_link(DFGB_Socket a, DFGB_Socket b);

  SmallVector<DFGB_Link> links();
  SmallVector<DFGB_Node *> nodes();

  template<typename T, typename... Args> T *new_source_info(Args &&... args)
  {
    static_assert(std::is_base_of<SourceInfo, T>::value, "");
    void *ptr = m_source_info_pool->allocate(sizeof(T));
    T *source = new (ptr) T(std::forward<Args>(args)...);
    return source;
  }

  inline bool is_mutable() const
  {
    /* This pool is stolen as soon, as the actual graph is build. */
    return m_source_info_pool.get() != nullptr;
  }

  std::string to_dot();
  void to_dot__clipboard();

 private:
  SmallSet<DFGB_Node *> m_nodes;
  SmallMap<DFGB_Socket, DFGB_Socket> m_input_origins;
  MultiMap<DFGB_Socket, DFGB_Socket> m_output_targets;
  MemPool m_node_pool;
  std::unique_ptr<MemMultiPool> m_source_info_pool;

  friend DFGB_Socket;
  friend CompactDataFlowGraph;
};

/* Inline methods of Socket
 ********************************************/

inline DFGB_Node *DFGB_Socket::node() const
{
  return m_node;
}

inline DataFlowGraphBuilder &DFGB_Socket::builder()
{
  return m_node->builder();
}

inline bool DFGB_Socket::is_input() const
{
  return !m_is_output;
}

inline bool DFGB_Socket::is_output() const
{
  return m_is_output;
}

inline uint DFGB_Socket::index() const
{
  return m_index;
}

inline Optional<DFGB_Socket> DFGB_Socket::origin()
{
  BLI_assert(this->is_input());
  DFGB_Socket *socket = this->builder().m_input_origins.lookup_ptr(*this);
  return Optional<DFGB_Socket>::FromPointer(socket);
}
inline ArrayRef<DFGB_Socket> DFGB_Socket::targets()
{
  BLI_assert(this->is_output());
  auto targets = this->builder().m_output_targets.lookup_default(*this);
  return targets;
}

inline bool DFGB_Socket::is_linked()
{
  if (this->is_input()) {
    return this->builder().m_input_origins.contains(*this);
  }
  else {
    return this->builder().m_output_targets.has_at_least_one_value(*this);
  }
}

inline bool operator==(const DFGB_Socket &a, const DFGB_Socket &b)
{
  return (a.m_node == b.m_node && a.m_is_output == b.m_is_output && a.m_index == b.m_index);
}

inline std::ostream &operator<<(std::ostream &stream, DFGB_Socket socket)
{
  stream << "<" << socket.node()->function()->name();
  stream << ", " << ((socket.is_input()) ? "Input" : "Output");
  stream << ":" << socket.index() << ">";
  return stream;
}

/* Inline methods of Node
 ********************************************/

inline DataFlowGraphBuilder &DFGB_Node::builder() const
{
  return m_builder;
}

inline Signature &DFGB_Node::signature()
{
  return m_function->signature();
}

inline SharedFunction &DFGB_Node::function()
{
  return m_function;
}

inline uint DFGB_Node::input_amount() const
{
  return m_function->signature().inputs().size();
}

inline uint DFGB_Node::output_amount() const
{
  return m_function->signature().outputs().size();
}

inline SourceInfo *DFGB_Node::source() const
{
  return m_source;
}

inline DFGB_Socket DFGB_Node::input(uint index)
{
  BLI_assert(index < this->input_amount());
  return DFGB_Socket(this, false, index);
}

inline DFGB_Socket DFGB_Node::output(uint index)
{
  BLI_assert(index < this->output_amount());
  return DFGB_Socket(this, true, index);
}

}  // namespace FN

namespace std {
template<> struct hash<FN::DFGB_Socket> {
  typedef FN::DFGB_Socket argument_type;
  typedef size_t result_type;

  result_type operator()(argument_type const &v) const noexcept
  {
    size_t h1 = std::hash<FN::DFGB_Node *>{}(v.node());
    size_t h2 = std::hash<bool>{}(v.is_input());
    size_t h3 = std::hash<uint>{}(v.index());
    return h1 ^ h2 ^ h3;
  }
};
}  // namespace std
