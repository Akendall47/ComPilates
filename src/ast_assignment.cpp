#include "ast_assignment.hpp"

namespace ast {

void Assignment::EmitRISC(std::ostream& stream, Context& context) const {
  rhs_->EmitRISC(stream, context);

  int offset = context.locals.at(lhs_name_);
  TypeSpecifier type = context.local_types.at(lhs_name_).base;

  if (type == TypeSpecifier::FLOAT) {
    stream << "  fsw fa0, " << offset << "(s0)\n";
  } else if (type == TypeSpecifier::DOUBLE) {
    stream << "  fsd fa0, " << offset << "(s0)\n";
  } else {
    stream << "  sw a0, " << offset << "(s0)\n";
  }
}

void Assignment::Print(std::ostream& stream) const {
  stream << lhs_name_ << " = ";
  rhs_->Print(stream);
  stream << ";" << "\n";
}

}  // namespace ast
