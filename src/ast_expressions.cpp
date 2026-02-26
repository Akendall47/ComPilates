#include "ast_expressions.hpp"

namespace ast {

// --- IntConstant ---

void IntConstant::EmitRISC(std::ostream& stream, Context&) const {
  stream << "li      a5, " << value_ << "\n";
}

void IntConstant::Print(std::ostream& stream) const { stream << value_; }

// --- Identifier ---

void Identifier::EmitRISC(std::ostream& stream, Context& context) const {
  int offset = context.locals.at(identifier_);
  stream << "lw      a5, " << offset << "(s0)\n";
}

void Identifier::Print(std::ostream& stream) const { stream << identifier_; }

// --- Assignment ---

void Assignment::EmitRISC(std::ostream& stream, Context& context) const {
  rhs_->EmitRISC(stream, context);
  int offset = context.locals.at(lhs_name_);
  stream << "sw      a5, " << offset << "(s0)\n";
}

void Assignment::Print(std::ostream& stream) const {
  stream << lhs_name_ << " = ";
  rhs_->Print(stream);
  stream << ";" << std::endl;
}

// --- BinaryOp ---

void BinaryOp::EmitRISC(std::ostream& stream, Context& context) const {
  // TODO: implement stack spill pattern
  (void)stream;
  (void)context;
}

void BinaryOp::Print(std::ostream& stream) const {
  left_->Print(stream);
  stream << " " << op_ << " ";
  right_->Print(stream);
}

}  // namespace ast
