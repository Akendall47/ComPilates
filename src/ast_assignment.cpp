#include "ast_assignment.hpp"

namespace ast {

void Assignment::EmitRISC(std::ostream& stream, Context& context) const {
  rhs_->EmitRISC(stream, context);

  // store into lhs variable's stack slot
  // currently using a5
  int offset = context.locals.at(lhs_name_);
  stream << "sw      a5, " << offset << "(s0)\n";
}

void Assignment::Print(std::ostream& stream) const {
  stream << lhs_name_ << " = ";
  rhs_->Print(stream);
  stream << ";" << std::endl;
}

}  // namespace ast
