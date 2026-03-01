#include "ast_assignment.hpp"

namespace ast {

void Assignment::EmitRISC(std::ostream& stream, Context& context) const {
  rhs_->EmitRISC(stream, context);

  // store into lhs variable's stack slot
  // currently using a5
  // (Samuel) should switch to a0 as it's standard result
  int offset = context.locals.at(lhs_name_);
  stream << "sw      a0, " << offset << "(s0)" <<std::endl;

  TypeSpecifier type = context.local_types.at(identifier_);

  if (type == FLOAT) {
      stream << "  fsw fa0, " << offset << "(s0)\n";
  } else if (type == DOUBLE) {
      stream << "  fsd fa0, " << offset << "(s0)\n";
  } else {
      stream << "  lw a0, " << offset << "(s0)\n";
  }
}

void Assignment::Print(std::ostream& stream) const {
  stream << lhs_name_ << " = ";
  rhs_->Print(stream);
  stream << ";" << "\n";
}

}  // namespace ast
