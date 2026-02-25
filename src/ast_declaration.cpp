#include "ast_declaration.hpp"

namespace ast {

// allocate a stack slot for this variable in the context — no instructions
// emitted
void Declaration::EmitRISC(std::ostream&, Context& context) const {
  context.AllocateLocal(name_);
}

void Declaration::Print(std::ostream& stream) const {
  stream << type_ << " " << name_ << ";" << std::endl;
}

}  // namespace ast
