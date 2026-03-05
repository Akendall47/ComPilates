#include "ast_operations.hpp"

#include <iostream>

#include "ast_context.hpp"
#include "ast_identifier.hpp"

void PostfixOp::Print(std::ostream& stream) const {
  operand_->Print(stream);
  stream << op_;
}

// dynamic cast again; compiler only knows 'some' node not which, allow GetName
// call
// reachhes into vtable and see identifier, can then give it identifeir*, if no
// throw
// operand must be an lvalue identifier
void PostfixOp::EmitRISC(std::ostream& stream, ast::Context& context) const {
  const auto& id = dynamic_cast<const ast::Identifier&>(*operand_);
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
// s0 + offset (offset is negative).
// addi can encode neg imms directly
void AddressOf::EmitRISC(std::ostream& stream, ast::Context& context) const {
  int offset = context.locals.at(name_);
  stream << "  addi    a0, s0, " << offset << "\n";
}

void AddressOf::Print(std::ostream& stream) const {
  stream << "(&" << name_ << ")";
}

// *p; p's value (an address), then load the int at that address
void Deref::EmitRISC(std::ostream& stream, ast::Context& context) const {
  operand_->EmitRISC(stream, context);
  stream << "  lw      a0, 0(a0)\n";
}

void Deref::Print(std::ostream& stream) const {
  stream << "(*";
  operand_->Print(stream);
  stream << ")";
}

// *p = v; pattern-match ptr as Identifier, load its address directly from frame
void DerefAssign::EmitRISC(std::ostream& stream, ast::Context& context) const {
  rhs_->EmitRISC(stream, context);  // a0 = value to store
  const auto& id = dynamic_cast<const ast::Identifier&>(*ptr_);
  int offset = context.locals.at(id.GetName());
  stream << "  lw      t1, " << offset << "(s0)\n";  // t1 = ptr value (address)
  stream << "  sw      a0, 0(t1)\n";                 // *ptr = value
}

void DerefAssign::Print(std::ostream& stream) const {
  stream << "(*";
  ptr_->Print(stream);
  stream << " = ";
  rhs_->Print(stream);
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

  // Ptr maths; if left side is ptr, scale right side by stride
  // detect by check if left_ an Identifier with ptr type
  if (op_ == "+" || op_ == "-") {
    int stride = 1;
    if (auto* id = dynamic_cast<const ast::Identifier*>(left_.get())) {
      auto it = context.local_types.find(id->GetName());
      if (it != context.local_types.end() && it->second.is_pointer()) {
        stride = it->second.pointee_size();  // 4 for int*
      }
    }
    if (stride != 1) {
      // scale offset (a0) by stride before add/subbing
      stream << "  li t2, " << stride << "\n";
      stream << "  mul a0, a0, t2\n";
    }
    if (op_ == "+") {
      stream << "  add a0, t1, a0\n";
    } else {
      stream << "  sub a0, t1, a0\n";
    }
    return;
  }

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
