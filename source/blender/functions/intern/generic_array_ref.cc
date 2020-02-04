#include "FN_generic_array_ref.h"

namespace FN {

void GenericMutableArrayRef::RelocateUninitialized(GenericMutableArrayRef from,
                                                   GenericMutableArrayRef to)
{
  BLI::assert_same_size(from, to);
  BLI_assert(from.type() == to.type());

  from.m_type->relocate_to_uninitialized_n(from.buffer(), to.buffer(), from.size());
}

}  // namespace FN
