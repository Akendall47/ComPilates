#include "ast_operations.hpp"

#include <iostream>

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

void UnaryOp::EmitRISC(std::ostream& stream, ast::Context& context) const {
  operand_->EmitRISC(stream, context);

  if (op_ == "-") {
    stream << "  neg a0, a0" << std::endl;
  } else if (op_ == "!") {
    stream << "  seqz a0, a0" << std::endl;
  } else if (op_ == "~") {
    stream << "  not a0, a0" << std::endl;
  }
}

void BinaryOp::EmitRISC(std::ostream& stream, ast::Context& context) const {
  // Short circuits
  if (op_ == "&&") {
    std::string label_false = context.MakeLabel("L_and_false");
    std::string label_end = context.MakeLabel("L_and_end");

    left_->EmitRISC(stream, context);
    stream << "  beq a0, zero, " << label_false << std::endl;

    right_->EmitRISC(stream, context);
    stream << "  snez a0, a0" << std::endl;
    stream << "  j " << label_end << std::endl;

    stream << label_false << ":" << std::endl;
    stream << "  li a0, 0" << std::endl;

    stream << label_end << ":" << std::endl;
    return;
  }

  if (op_ == "||") {
    std::string label_true = context.MakeLabel("L_or_true");
    std::string label_end = context.MakeLabel("L_or_end");

    left_->EmitRISC(stream, context);
    stream << "  bnez a0, " << label_true << std::endl;

    right_->EmitRISC(stream, context);
    stream << "  snez a0, a0" << std::endl;  // Normalize result to 0 or 1
    stream << "  j " << label_end << std::endl;

    stream << label_true << ":" << std::endl;
    stream << "  li a0, 1" << std::endl;

    stream << label_end << ":" << std::endl;
    return;
  }


  left_->EmitRISC(stream, context);

  stream << "  addi sp, sp, -4" << std::endl;
  stream << "  sw a0, 0(sp)" << std::endl;

  right_->EmitRISC(stream, context);

  stream << "  lw t1, 0(sp)" << std::endl;
  stream << "  addi sp, sp, 4" << std::endl;

  if (op_ == "+") {
    stream << "  add a0, t1, a0" << std::endl;
  } else if (op_ == "-") {
    stream << "  sub a0, t1, a0" << std::endl;
  } else if (op_ == "*") {
    stream << "  mul a0, t1, a0" << std::endl;
  } else if (op_ == "/") {
    stream << "  div a0, t1, a0" << std::endl;
  } else if (op_ == "%") {
    stream << "  rem a0, t1, a0" << std::endl;
  } else if (op_ == "<<") {
    stream << "  sll a0, t1, a0" << std::endl;
  } else if (op_ == ">>") {
    stream << "  sra a0, t1, a0" << std::endl;
  } else if (op_ == "&") {
    stream << "  and a0, t1, a0" << std::endl;
  } else if (op_ == "^") {
    stream << "  xor a0, t1, a0" << std::endl;
  } else if (op_ == "|") {
    stream << "  or a0, t1, a0" << std::endl;
  }
  // Relational Operators so returnign true or false
  else if (op_ == "<") {
    stream << "  slt a0, t1, a0" << std::endl;
  } else if (op_ == ">") {
    stream << "  slt a0, a0, t1" << std::endl;
  } else if (op_ == "<=") {
    stream << "  sgt a0, t1, a0" << std::endl;  // 1 if t1 > a0
    stream << "  xori a0, a0, 1" << std::endl;  // not
  } else if (op_ == ">=") {
    // (a >= b) == !(a < b)
    stream << "  slt a0, t1, a0" << std::endl;  // 1 if t1 < a0
    stream << "  xori a0, a0, 1" << std::endl;  // not
  } else if (op_ == "==") {
    stream << "  sub t1, t1, a0" << std::endl;
    stream << "  seqz a0, t1" << std::endl;
  } else if (op_ == "!=") {
    stream << "  sub t1, t1, a0" << std::endl;
    stream << "  snez a0, t1" << std::endl;
  }
}
