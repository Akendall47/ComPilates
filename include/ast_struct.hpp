#pragma once

#include <string>
#include <vector>

#include "ast_context.hpp"
#include "ast_node.hpp"

namespace ast {

// build structDef, calc byte offsets, and register struct types in
// context.structtypes
class StructDefinition : public Node {
  std::string struct_tag_;
  std::vector<std::string> field_names_;

 public:
  StructDefinition(std::string struct_tag, std::vector<std::string> field_names)
      : struct_tag_(std::move(struct_tag)),
        field_names_(std::move(field_names)) {}

  void EmitRISC(std::ostream&, Context& context) const override {
    StructDef layout;
    int byte_offset = 0;
    for (const auto& field : field_names_) {
      layout.members.push_back({field, byte_offset, 4});
      byte_offset += 4;
    }
    layout.total_size = byte_offset;
    context.struct_types[struct_tag_] = layout;
  }

  void Print(std::ostream& stream) const override {
    stream << "struct " << struct_tag_ << " { ";
    for (const auto& field : field_names_) stream << "int " << field << "; ";
    stream << "};\n";
  }
};

// struct x z; call allocs struct instance on the stack
class StructDeclaration : public Node {
  std::string struct_tag_;     // type name e.g. "x"
  std::string instance_name_;  // variable name e.g. "z"
 public:
  StructDeclaration(std::string struct_tag, std::string instance_name)
      : struct_tag_(std::move(struct_tag)),
        instance_name_(std::move(instance_name)) {}

  void EmitRISC(std::ostream&, Context& context) const override {
    context.AllocateStruct(instance_name_, struct_tag_);
  }
  void Print(std::ostream& stream) const override {
    stream << "struct " << struct_tag_ << " " << instance_name_ << ";\n";
  }

  int CountDeclarations() const override {
    return 1;  // stack frame sizing — actual byte count handled by
               // AllocateStruct
  }
};

// read a field, load a struct member (rvalue)
class StructMemberGet : public Node {
  std::string instance_name_;
  std::string field_name_;

 public:
  StructMemberGet(std::string instance_name, std::string field_name)
      : instance_name_(std::move(instance_name)),
        field_name_(std::move(field_name)) {}

  void EmitRISC(std::ostream& stream, Context& context) const override {
    int instance_base = context.locals.at(instance_name_);
    const std::string& struct_tag =
        context.local_struct_type.at(instance_name_);
    const StructDef& layout = context.struct_types.at(struct_tag);
    for (const auto& member : layout.members) {
      if (member.name == field_name_) {
        stream << "  lw      a0, " << (instance_base + member.offset)
               << "(s0)\n";
        return;
      }
    }
  }

  void Print(std::ostream& stream) const override {
    stream << instance_name_ << "." << field_name_;
  }
};

// write a field, z.field = expr , lvalue
class StructMemberSet : public Node {
  std::string instance_name_;
  std::string field_name_;
  NodePtr rhs_;

 public:
  StructMemberSet(std::string instance_name, std::string field_name,
                  NodePtr rhs)
      : instance_name_(std::move(instance_name)),
        field_name_(std::move(field_name)),
        rhs_(std::move(rhs)) {}

  void EmitRISC(std::ostream& stream, Context& context) const override {
    rhs_->EmitRISC(stream, context);  // a0 = value to store
    int instance_base = context.locals.at(instance_name_);
    const std::string& struct_tag =
        context.local_struct_type.at(instance_name_);
    const StructDef& layout = context.struct_types.at(struct_tag);
    for (const auto& member : layout.members) {
      if (member.name == field_name_) {
        stream << "  sw      a0, " << (instance_base + member.offset)
               << "(s0)\n";
        return;
      }
    }
  }

  void Print(std::ostream& stream) const override {
    stream << instance_name_ << "." << field_name_ << " = ";
    rhs_->Print(stream);
    stream << ";\n";
  }
};

}  // namespace ast
