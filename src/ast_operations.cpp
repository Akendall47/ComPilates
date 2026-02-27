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
    stream << "  neg a0, a0" << "\n";
  } else if (op_ == "!") {
    stream << "  seqz a0, a0" << "\n";
  } else if (op_ == "~") {
    stream << "  not a0, a0" << "\n";
  }
}

void BinaryOp::EmitRISC(std::ostream& stream, ast::Context& context) const {
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

  stream << "  addi sp, sp, -4" << "\n";
  stream << "  sw a0, 0(sp)" << "\n";

  right_->EmitRISC(stream, context);

  stream << "  lw t1, 0(sp)" << "\n";
  stream << "  addi sp, sp, 4" << "\n";

  if (op_ == "+") {
    stream << "  add a0, t1, a0" << "\n";
  } else if (op_ == "-") {
    stream << "  sub a0, t1, a0" << "\n";
  } else if (op_ == "*") {
    stream << "  mul a0, t1, a0" << "\n";
  } else if (op_ == "/") {
    stream << "  div a0, t1, a0" << "\n";
  } else if (op_ == "%") {
    stream << "  rem a0, t1, a0" << "\n";
  } else if (op_ == "<<") {
    stream << "  sll a0, t1, a0" << "\n";
  } else if (op_ == ">>") {
    stream << "  sra a0, t1, a0" << "\n";
  } else if (op_ == "&") {
    stream << "  and a0, t1, a0" << "\n";
  } else if (op_ == "^") {
    stream << "  xor a0, t1, a0" << "\n";
  } else if (op_ == "|") {
    stream << "  or a0, t1, a0" << "\n";
  }
  // Relational Operators so returnign true or false
  else if (op_ == "<") {
    stream << "  slt a0, t1, a0" << "\n";
  } else if (op_ == ">") {
    stream << "  slt a0, a0, t1" << "\n";
  } else if (op_ == "<=") {
    stream << "  sgt a0, t1, a0" << "\n";  // 1 if t1 > a0
    stream << "  xori a0, a0, 1" << "\n";  // not
  } else if (op_ == ">=") {
    // (a >= b) == !(a < b)
    stream << "  slt a0, t1, a0" << "\n";  // 1 if t1 < a0
    stream << "  xori a0, a0, 1" << "\n";  // not
  } else if (op_ == "==") {
    stream << "  sub t1, t1, a0" << "\n";
    stream << "  seqz a0, t1" << "\n";
  } else if (op_ == "!=") {
    stream << "  sub t1, t1, a0" << "\n";
    stream << "  snez a0, t1" << "\n";
  }
}
