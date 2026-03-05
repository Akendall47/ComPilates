#include "ast_declaration.hpp"

namespace ast {

// allocate a stack slot for this variable in the context, no instructions
// emitted; records pointer depth in VarType for later arithmetic
void Declaration::EmitRISC(std::ostream&, Context& context) const {
  VarType vt;
  vt.base = type_;
  vt.pointer_depth = pointer_depth_;
  context.AllocateLocal(name_, 4, vt);
}

void Declaration::Print(std::ostream& stream) const {
  stream << type_ << " " << name_ << ";" << "\n";
}

// alloc the slot then evaluate initialiser and store it
void InitDeclaration::EmitRISC(std::ostream& stream, Context& context) const {
  VarType vt;
  vt.base = type_;
  vt.pointer_depth = pointer_depth_;
  int offset = context.AllocateLocal(name_, 4, vt);
  init_->EmitRISC(stream, context);
  stream << "  sw      a0, " << offset << "(s0)\n";
}

void InitDeclaration::Print(std::ostream& stream) const {
  stream << type_ << " " << name_ << " = ";
  init_->Print(stream);
  stream << ";\n";
}

}  // namespace ast
