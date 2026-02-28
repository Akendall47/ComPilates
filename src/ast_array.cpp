#include "ast_array.hpp"

#include "ast_constant.hpp"

namespace ast {

// global; emit .bss/.space   local; reserve stack space
// if current func empty its globabl, not inside anything, top delcaration
//.align 2 == 2^2 bytes aligned.; sw/lw addrs need 4 bytes aligned
void ArrayDeclaration::EmitRISC(std::ostream& stream, Context& context) const {
  if (context.current_function.empty()) {
    context.RegisterGlobal(name_);
    stream << ".bss\n";
    stream << ".globl " << name_ << "\n";
    stream << ".align 2\n";
    stream << name_ << ":\n";
    stream << "    .space " << (array_size_ * 4) << "\n";
  } else {
    context.AllocateLocal(name_, array_size_ * 4);
  }
}

void ArrayDeclaration::Print(std::ostream& stream) const {
  stream << type_ << " " << name_ << "[" << array_size_ << "];\n";
}

// compute abs addr of x[i] into t2
//  fast path, const index folds offset at compile time
// ie is it atcually just an IntConst
//  general path: evaluate index at runtime, 3 instrS
static void EmitArrayAddress(std::ostream& stream, Context& context,
                             const std::string& name, const Node& index) {
  if (context.globals.count(name)) {
    // global; load base addr via lui/addi %hi/%lo, then add index*4
    // lui; only way too set set [31:12] + addi to set final 12 (its constarined
    // to 12 anyways)
    // with %hi(x); gives 31;12      %low gives 11;0
    //// const index; fold element offset into %lo at compile time isnt safe
    //// (linker fills %lo, can't add to it), so use runtime path even for const

    if (auto* c = dynamic_cast<const IntConstant*>(&index)) {
      stream << "lui     t2, %hi(" << name << ")\n";
      stream << "addi    t2, t2, %lo(" << name << ")\n";
      int byte_off = c->GetValue() * 4;
      if (byte_off != 0) stream << "addi    t2, t2, " << byte_off << "\n";
      return;
    }

    index.EmitRISC(stream, context);  // index in a0
    stream << "slli    a0, a0, 2\n";
    stream << "lui     t2, %hi(" << name << ")\n";
    stream << "addi    t2, t2, %lo(" << name << ")\n";
    stream << "add     t2, t2, a0\n";
    return;
  }

  // local: s0-relative addressing
  int base = context.locals.at(name);
  if (auto* c = dynamic_cast<const IntConstant*>(&index)) {
    int offset = base + c->GetValue() * 4;
    stream << "addi    t2, s0, " << offset << "\n";
    return;
  }
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
