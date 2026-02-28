#include "ast_array.hpp"

#include "ast_constant.hpp"

namespace ast {

// reserve n*4 bytes on stack
void ArrayDeclaration::EmitRISC(std::ostream&, Context& context) const {
  context.AllocateLocal(name_, array_size_ * 4);
}

void ArrayDeclaration::Print(std::ostream& stream) const {
  stream << type_ << " " << name_ << "[" << array_size_ << "];\n";
}

// compute abs addr of x[i] into t2
//  fast path, const index folds offset at compile time, 1 instr
//  general path: evaluate index at runtime, 3 instrS
static void EmitArrayAddress(std::ostream& stream, Context& context,
                             const std::string& name, const Node& index) {
  int base = context.locals.at(name);

  // constant index check: eg. x[0], find final offset at compile time
  // eg. -44 + 0*4 = -44
  // otherwise fall through via nullptr
  // optimised as we do CPP arithemtic not RISCV
  if (auto* c = dynamic_cast<const IntConstant*>(&index)) {
    int offset = base + c->GetValue() * 4;
    stream << "addi    t2, s0, " << offset << "\n";
    return;
  }

  // index*4; offset from s0; absolute addr, nut just s0 relative
  index.EmitRISC(stream, context);
  stream << "slli    t2, a0, 2\n";
  stream << "addi    t2, t2, " << base << "\n";
  stream << "add     t2, t2, s0\n";
}

// (rvalue) load x[i] into a0
// const fast path: lw a0, offset(s0), 1 instr, no t2 needed
void ArraySubscript::EmitRISC(std::ostream& stream, Context& context) const {
  int base = context.locals.at(name_);
  if (auto* c = dynamic_cast<const IntConstant*>(index_.get())) {
    int offset = base + c->GetValue() * 4;
    stream << "lw      a0, " << offset << "(s0)\n";
    return;
  }
  EmitArrayAddress(stream, context, name_, *index_);
  stream << "lw      a0, 0(t2)\n";
}

void ArraySubscript::Print(std::ostream& stream) const {
  stream << name_ << "[";
  index_->Print(stream);
  stream << "]";
}

// x[i] = rhs
// t2 holds addr, BinaryOp only uses t1, so t2 should be safe
void ArrayAssignment::EmitRISC(std::ostream& stream, Context& context) const {
  EmitArrayAddress(stream, context, name_, *index_);
  // t2 = addr of x[i]; evaluate rhs freely (uses t1, not t2)

  rhs_->EmitRISC(stream, context);  // result in a0
  stream << "sw      a0, 0(t2)\n";
}

void ArrayAssignment::Print(std::ostream& stream) const {
  stream << name_ << "[";
  index_->Print(stream);
  stream << "] = ";
  rhs_->Print(stream);
  stream << ";\n";
}

}  // namespace ast
