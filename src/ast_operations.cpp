#include "ast_operations.hpp"

#include <iostream>

#include "ast_context.hpp"
#include "ast_identifier.hpp"

namespace ast {

void PostfixOp::Print(std::ostream& stream) const {
  operand_->Print(stream);
  stream << op_;
}

void PostfixOp::EmitRISC(std::ostream& stream, Context& context) const {
  const auto& id = dynamic_cast<const Identifier&>(*operand_);
  int offset = context.locals.at(id.GetName());

  stream << "lw      a0, " << offset << "(s0)\n";
  if (op_ == "++") {
    stream << "addi    a0, a0, 1\n";
  } else {
    stream << "addi    a0, a0, -1\n";
  }
  stream << "sw      a0, " << offset << "(s0)\n";
}

void UnaryOp::Print(std::ostream& stream) const {
  stream << "(" << op_ << " ";
  operand_->Print(stream);
  stream << ")";
}

void BinaryOp::Print(std::ostream& stream) const {
  stream << "(";
  left_->Print(stream);
  stream << " " << op_ << " ";
  right_->Print(stream);
  stream << ")";
}

// &x, put the stack address of x into a0
void AddressOf::EmitRISC(std::ostream& stream, Context& context) const {
  int offset = context.locals.at(name_);
  stream << "  addi    a0, s0, " << offset << "\n";
}

void AddressOf::Print(std::ostream& stream) const {
  stream << "(&" << name_ << ")";
}

// *p; load p's value (an address), then load the int at that address
void Deref::EmitRISC(std::ostream& stream, Context& context) const {
  operand_->EmitRISC(stream, context);  // a0 = the pointer value
  bool is_char = false;
  if (auto* id = dynamic_cast<const Identifier*>(operand_.get())) {
    auto it = context.local_types.find(id->GetName());
    if (it != context.local_types.end())
      is_char = (it->second.base == TypeSpecifier::CHAR);
  }
  stream << (is_char ? "  lbu     a0, 0(a0)\n" : "  lw      a0, 0(a0)\n");
}

void Deref::Print(std::ostream& stream) const {
  stream << "(*";
  operand_->Print(stream);
  stream << ")";
}

// *p = v  OR  **pp = v  (any depth)
void DerefAssign::EmitRISC(std::ostream& stream, Context& context) const {
  rhs_->EmitRISC(stream, context);
  stream << "  mv      t2, a0\n";  // t2 = value (save before ptr eval)
  ptr_->EmitRISC(stream, context);
  stream << "  sw      t2, 0(a0)\n";
}

void DerefAssign::Print(std::ostream& stream) const {
  stream << "*(";
  ptr_->Print(stream);
  stream << ") = ";
  rhs_->Print(stream);
  stream << ";\n";
}

void UnaryOp::EmitRISC(std::ostream& stream, Context& context) const {
  operand_->EmitRISC(stream, context);
  TypeSpecifier type = operand_->GetType(context);

  if (type == TypeSpecifier::FLOAT) {
    if (op_ == "-") stream << "  fneg.s fa0, fa0\n";
  } else if (type == TypeSpecifier::DOUBLE) {
    if (op_ == "-") stream << "  fneg.d fa0, fa0\n";
  } else {
    if (op_ == "-")
      stream << "  neg a0, a0\n";
    else if (op_ == "!")
      stream << "  seqz a0, a0\n";
    else if (op_ == "~")
      stream << "  not a0, a0\n";
  }
}

void BinaryOp::EmitRISC(std::ostream& stream, Context& context) const {
  TypeSpecifier left_type = left_->GetType(context);
  TypeSpecifier op_type = this->GetType(context);

  // Short circuits
  if (op_ == "&&") {
    int label = context.AllocateLabel();
    left_->EmitRISC(stream, context);
    stream << "  beq a0, zero, .Land_false_" << label << "\n";
    right_->EmitRISC(stream, context);
    stream << "  snez a0, a0\n";
    stream << "  j .Land_end_" << label << "\n";
    stream << ".Land_false_" << label << ":\n";
    stream << "  li a0, 0\n";
    stream << ".Land_end_" << label << ":\n";
    return;
  }

  if (op_ == "||") {
    int label = context.AllocateLabel();
    left_->EmitRISC(stream, context);
    stream << "  bnez a0, .Lor_true_" << label << "\n";
    right_->EmitRISC(stream, context);
    stream << "  snez a0, a0\n";
    stream << "  j .Lor_end_" << label << "\n";
    stream << ".Lor_true_" << label << ":\n";
    stream << "  li a0, 1\n";
    stream << ".Lor_end_" << label << ":\n";
    return;
  }

  left_->EmitRISC(stream, context);

  stream << "  addi sp, sp, -8\n";  // allocate max (double needs 8)
  if (left_type == TypeSpecifier::FLOAT) {
    stream << "  fsw fa0, 0(sp)\n";
  } else if (left_type == TypeSpecifier::DOUBLE) {
    stream << "  fsd fa0, 0(sp)\n";
  } else {
    stream << "  sw a0, 0(sp)\n";
  }

  right_->EmitRISC(stream, context);

  if (left_type == TypeSpecifier::FLOAT) {
    stream << "  flw ft1, 0(sp)\n";
  } else if (left_type == TypeSpecifier::DOUBLE) {
    stream << "  fld ft1, 0(sp)\n";
  } else {
    stream << "  lw t1, 0(sp)\n";
  }
  stream << "  addi sp, sp, 8\n";

  if (op_type == TypeSpecifier::FLOAT) {
    if (op_ == "+")
      stream << "  fadd.s fa0, ft1, fa0\n";
    else if (op_ == "-")
      stream << "  fsub.s fa0, ft1, fa0\n";
    else if (op_ == "*")
      stream << "  fmul.s fa0, ft1, fa0\n";
    else if (op_ == "/")
      stream << "  fdiv.s fa0, ft1, fa0\n";
    else if (op_ == "<")
      stream << "  flt.s a0, ft1, fa0\n";
    else if (op_ == "<=")
      stream << "  fle.s a0, ft1, fa0\n";
    else if (op_ == "==")
      stream << "  feq.s a0, ft1, fa0\n";
  } else if (op_type == TypeSpecifier::DOUBLE) {
    if (op_ == "+")
      stream << "  fadd.d fa0, ft1, fa0\n";
    else if (op_ == "-")
      stream << "  fsub.d fa0, ft1, fa0\n";
    else if (op_ == "*")
      stream << "  fmul.d fa0, ft1, fa0\n";
    else if (op_ == "/")
      stream << "  fdiv.d fa0, ft1, fa0\n";
    else if (op_ == "<")
      stream << "  flt.d a0, ft1, fa0\n";
    else if (op_ == "<=")
      stream << "  fle.d a0, ft1, fa0\n";
    else if (op_ == "==")
      stream << "  feq.d a0, ft1, fa0\n";
  } else {
    // integer ops — check for pointer stride scaling on + and -
    if (op_ == "+" || op_ == "-") {
      int stride = 1;
      if (auto* id = dynamic_cast<const Identifier*>(left_.get())) {
        auto it = context.local_types.find(id->GetName());
        if (it != context.local_types.end() && it->second.is_pointer()) {
          stride = it->second.pointee_size();
        }
      }
      if (stride != 1) {
        stream << "  li t2, " << stride << "\n";
        stream << "  mul a0, a0, t2\n";
      }
      if (op_ == "+")
        stream << "  add a0, t1, a0\n";
      else
        stream << "  sub a0, t1, a0\n";
      return;
    }

    bool is_unsigned = IsUnsigned(op_type);
    if (op_ == "+")
      stream << "  add a0, t1, a0\n";
    else if (op_ == "-")
      stream << "  sub a0, t1, a0\n";
    else if (op_ == "*")
      stream << "  mul a0, t1, a0\n";
    else if (op_ == "/")
      stream << (is_unsigned ? "  divu a0, t1, a0\n" : "  div a0, t1, a0\n");
    else if (op_ == "%")
      stream << (is_unsigned ? "  remu a0, t1, a0\n" : "  rem a0, t1, a0\n");
    else if (op_ == "<<")
      stream << "  sll a0, t1, a0\n";
    else if (op_ == ">>")
      stream << (is_unsigned ? "  srl a0, t1, a0\n" : "  sra a0, t1, a0\n");
    else if (op_ == "&")
      stream << "  and a0, t1, a0\n";
    else if (op_ == "^")
      stream << "  xor a0, t1, a0\n";
    else if (op_ == "|")
      stream << "  or a0, t1, a0\n";
    else if (op_ == "<")
      stream << (is_unsigned ? "  sltu a0, t1, a0\n" : "  slt a0, t1, a0\n");
    else if (op_ == ">")
      stream << (is_unsigned ? "  sltu a0, a0, t1\n" : "  slt a0, a0, t1\n");
    else if (op_ == "<=") {
      stream << (is_unsigned ? "  sgtu a0, t1, a0\n" : "  sgt a0, t1, a0\n");
      stream << "  xori a0, a0, 1\n";
    } else if (op_ == ">=") {
      stream << (is_unsigned ? "  sltu a0, t1, a0\n" : "  slt a0, t1, a0\n");
      stream << "  xori a0, a0, 1\n";
    } else if (op_ == "==") {
      stream << "  sub t1, t1, a0\n";
      stream << "  seqz a0, t1\n";
    } else if (op_ == "!=") {
      stream << "  sub t1, t1, a0\n";
      stream << "  snez a0, t1\n";
    }
  }
}

TypeSpecifier BinaryOp::GetType(Context& context) const {
  TypeSpecifier left_type = left_->GetType(context);
  TypeSpecifier right_type = right_->GetType(context);

  if (left_type == TypeSpecifier::DOUBLE || right_type == TypeSpecifier::DOUBLE)
    return TypeSpecifier::DOUBLE;
  if (left_type == TypeSpecifier::FLOAT || right_type == TypeSpecifier::FLOAT)
    return TypeSpecifier::FLOAT;
  if (IsUnsigned(left_type) || IsUnsigned(right_type))
    return TypeSpecifier::UINT;
  return TypeSpecifier::INT;
}

}  // namespace ast
