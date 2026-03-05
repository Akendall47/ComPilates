#include "ast_declaration.hpp"

#include "ast_context.hpp"

namespace ast {

void Declaration::EmitRISC(std::ostream&, Context& context) const {
  context.AllocateLocal(name_, type_, 1, pointer_depth_);
}

void Declaration::Print(std::ostream& stream) const {
  stream << type_ << " " << name_ << ";\n";
}

TypeSpecifier Declaration::GetType(Context&) const { return type_; }

// alloc the slot then evaluate initialiser and store it
void InitDeclaration::EmitRISC(std::ostream& stream, Context& context) const {
  int offset = context.AllocateLocal(name_, type_, 1, pointer_depth_);
  init_->EmitRISC(stream, context);
  stream << "  sw      a0, " << offset << "(s0)\n";
}

void InitDeclaration::Print(std::ostream& stream) const {
  stream << type_ << " " << name_ << " = ";
  init_->Print(stream);
  stream << ";\n";
}

}  // namespace ast
