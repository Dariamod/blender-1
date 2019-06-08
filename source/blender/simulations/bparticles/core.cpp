#include "core.hpp"

namespace BParticles {

NamedBuffers::~NamedBuffers()
{
}

Description::~Description()
{
  for (Force *force : m_forces) {
    delete force;
  }
  for (Emitter *emitter : m_emitters) {
    delete emitter;
  }
}

Solver::~Solver()
{
}

StateBase::~StateBase()
{
}

Force::~Force()
{
}

Emitter::~Emitter()
{
}

void adapt_state(Solver *new_solver, WrappedState *wrapped_state)
{
  wrapped_state->m_solver = new_solver;
}

}  // namespace BParticles