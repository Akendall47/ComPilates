#include "ast_constant.hpp"

namespace ast {

void IntConstant::EmitRISC(std::ostream& stream, Context&) const {
  stream << "li      a0, " << value_ << "\n";
}

void IntConstant::Print(std::ostream& stream) const { stream << value_; }
  // namespace ast


void FloatConstant::EmitRISC(std::ostream& stream, Context&) const {
  // Cannot directly store float so must convert into interger representation first

  float float_val = static_cast<float>(value_);
  uint32_t bits = std::bit_cast<uint32_t>(float_val);
  stream << "li t0, "<< bits << "\n";
  stream << "  fmv.w.x fa0, t0\n";

}

void FloatConstant::Print(std::ostream& stream) const { stream << value_; }


TypeSpecifier FloatConstant::GetType(Context&) const { return TypeSpecifier::FLOAT; }
}  // namespace ast

