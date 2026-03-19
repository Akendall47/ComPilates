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
    context.AllocateLocal(name_, type_, array_size_);
    // NOTE and maybe change: assuming all arrays int atm
  }
}

void ArrayDeclaration::Print(std::ostream& stream) const {
  stream << type_ << " " << name_ << "[" << array_size_ << "];\n";
}

// fixed char array indexing: use element size (1 for char, 4 for int)
// instead of hardcoded 4 for offset
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
  int elem_size = 4;
  if (auto it = context.local_types.find(name); it != context.local_types.end())
    elem_size = context.GetTypeSize(it->second.base);
  if (auto* c = dynamic_cast<const IntConstant*>(&index)) {
    int offset = base + c->GetValue() * elem_size;
    stream << "addi    t2, s0, " << offset << "\n";
    return;
  }
  index.EmitRISC(stream, context);
  if (elem_size == 4) {
    stream << "slli    t2, a0, 2\n";
  } else if (elem_size == 8) {
    stream << "slli    t2, a0, 3\n";
  } else {
    // elem_size == 1 (char): no scaling needed
    stream << "mv      t2, a0\n";
  }
  stream << "addi    t2, t2, " << base << "\n";
  stream << "add     t2, t2, s0\n";
}

// If name_ is a pointer variable, p[i] == *(p + i*4): load p, scale i, add, lw
// If name_ is a local array, fast path with base+offset(s0).
void ArraySubscript::EmitRISC(std::ostream& stream, Context& context) const {
  auto type_it = context.local_types.find(name_);
  bool is_ptr =
      (type_it != context.local_types.end() && type_it->second.is_pointer());

  if (is_ptr) {
    bool is_char = (type_it->second.base == TypeSpecifier::CHAR);
    const char* load = is_char ? "lbu" : "lw";
    int ptr_offset = context.locals.at(name_);
    stream << "lw      t2, " << ptr_offset << "(s0)\n";  // t2 = ptr value
    if (auto* c = dynamic_cast<const IntConstant*>(index_.get())) {
      int byte_off = c->GetValue() * type_it->second.pointee_size();
      if (byte_off != 0) stream << "addi    t2, t2, " << byte_off << "\n";
    } else {
      index_.get()->EmitRISC(stream, context);  // index in a0
      stream << "li      t1, " << type_it->second.pointee_size() << "\n";
      stream << "mul     a0, a0, t1\n";
      stream << "add     t2, t2, a0\n";
    }
    stream << load << "     a0, 0(t2)\n";
    return;
  }

  // local array; base is s0-relative
  bool is_char = (type_it != context.local_types.end() &&
                  type_it->second.base == TypeSpecifier::CHAR);
  const char* load = is_char ? "lbu" : "lw";
  int base = context.locals.at(name_);
  int elem_size = (type_it != context.local_types.end())
                      ? context.GetTypeSize(type_it->second.base)
                      : 4;
  if (auto* c = dynamic_cast<const IntConstant*>(index_.get())) {
    int offset = base + c->GetValue() * elem_size;
    stream << load << "     a0, " << offset << "(s0)\n";
    return;
  }
  EmitArrayAddress(stream, context, name_, *index_);
  stream << load << "     a0, 0(t2)\n";
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
  auto type_it = context.local_types.find(name_);
  bool is_char = (type_it != context.local_types.end() &&
                  type_it->second.base == TypeSpecifier::CHAR);
  stream << (is_char ? "sb      a0, 0(t2)\n" : "sw      a0, 0(t2)\n");
}

void ArrayAssignment::Print(std::ostream& stream) const {
  stream << name_ << "[";
  index_->Print(stream);
  stream << "] = ";
  rhs_->Print(stream);
  stream << ";\n";
}

}  // namespace ast
