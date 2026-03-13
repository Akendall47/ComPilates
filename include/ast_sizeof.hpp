#pragma once

#include <string>

#include "ast_context.hpp"
#include "ast_node.hpp"

namespace ast {

// sizeof(variable) — handles scalars, pointers, and structs
class SizeOf : public Node {
  std::string operand_name_;

 public:
  explicit SizeOf(std::string operand_name)
      : operand_name_(std::move(operand_name)) {}

  void EmitRISC(std::ostream& stream, Context& context) const override {
    int byte_size = 4;  // default for int/float/pointer
    if (context.local_struct_type.count(operand_name_)) {
      const std::string& struct_tag =
          context.local_struct_type.at(operand_name_);
      byte_size = context.struct_types.at(struct_tag).total_size;
    } else if (context.local_types.count(operand_name_)) {
      byte_size = context.GetTypeSize(context.local_types.at(operand_name_).base);
    }
    stream << "  li      a0, " << byte_size << "\n";
  }

  void Print(std::ostream& stream) const override {
    stream << "sizeof(" << operand_name_ << ")";
  }
};

// sizeof(type) — e.g. sizeof(int), sizeof(char)
class SizeOfType : public Node {
  TypeSpecifier type_;

 public:
  explicit SizeOfType(TypeSpecifier type) : type_(type) {}

  void EmitRISC(std::ostream& stream, Context& context) const override {
    stream << "  li      a0, " << context.GetTypeSize(type_) << "\n";
  }

  void Print(std::ostream& stream) const override {
    stream << "sizeof(" << type_ << ")";
  }
};

// sizeof(type *) — all pointers are 4 bytes on 32-bit RISC-V
class SizeOfPointer : public Node {
 public:
  void EmitRISC(std::ostream& stream, Context&) const override {
    stream << "  li      a0, 4\n";
  }

  void Print(std::ostream& stream) const override {
    stream << "sizeof(ptr)";
  }
};

}  // namespace ast
