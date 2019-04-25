#pragma once

#include "BLI_utildefines.h"

#include <algorithm>
#include <memory>

namespace BLI {

template<typename T> class Optional {
 private:
  char m_raw_data[sizeof(T)];
  bool m_set;

 public:
  static Optional FromPointer(T *ptr)
  {
    if (ptr == nullptr) {
      return Optional();
    }
    else {
      return Optional(*ptr);
    }
  }

  Optional() : m_set(false)
  {
  }

  ~Optional()
  {
    this->reset();
  }

  Optional(T &value) : Optional()
  {
    this->set(value);
  }

  Optional(T &&value) : Optional()
  {
    this->set(value);
  }

  Optional(const Optional &other)
  {
    if (other.has_value()) {
      this->set(other.value());
    }
    else {
      m_set = false;
    }
  }

  Optional(Optional &&other)
  {
    if (other.has_value()) {
      this->set(std::move(other.value()));
    }
    else {
      m_set = false;
    }
  }

  Optional &operator=(const Optional &other)
  {
    if (this == &other) {
      return *this;
    }
    if (other.has_value()) {
      this->set(other.value());
    }
    else {
      this->reset();
    }
    return *this;
  }

  Optional &operator=(Optional &&other)
  {
    if (this == &other) {
      return *this;
    }
    if (other.has_value()) {
      this->set(std::move(other.value()));
    }
    else {
      this->reset();
    }
    return *this;
  }

  bool has_value() const
  {
    return m_set;
  }

  T &value() const
  {
    if (m_set) {
      return *this->value_ptr();
    }
    else {
      BLI_assert(false);
      return *(T *)nullptr;
    }
  }

  void set(T &value)
  {
    if (m_set) {
      std::copy_n(&value, 1, this->value_ptr());
    }
    else {
      std::uninitialized_copy_n(&value, 1, this->value_ptr());
      m_set = true;
    }
  }

  void set(T &&value)
  {
    if (m_set) {
      std::copy_n(&value, 1, this->value_ptr());
    }
    else {
      std::uninitialized_copy_n(&value, 1, this->value_ptr());
    }
  }

  void reset()
  {
    if (m_set) {
      this->value_ptr()->~T();
      m_set = false;
    }
  }

 private:
  T *value_ptr() const
  {
    return (T *)m_raw_data;
  }
};

} /* namespace BLI */
