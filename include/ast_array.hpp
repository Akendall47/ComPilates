#pragma once

#include "ast_node.hpp"
#include "ast_type_specifier.hpp"

namespace ast {

// allocs N*4 bytes on the stack
class ArrayDeclaration : public Node {
 private:
  TypeSpecifier type_;
  std::string name_;
  int array_size_;

 public:
  ArrayDeclaration(TypeSpecifier type, std::string name, int array_size)
      : type_(type), name_(std::move(name)), array_size_(array_size) {}

  int CountDeclarations() const override { return array_size_; }

  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
};

// x[index] as rvalue ,compute addresss, loads value into a0
class ArraySubscript : public Node {
 private:
  std::string name_;
  NodePtr index_;  // index expression, e.g. Identifier("i") or BinaryOp

 public:
  ArraySubscript(std::string name, NodePtr index)
      : name_(std::move(name)), index_(std::move(index)) {}

  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
};

// x[index] = rhs, computes address, stores rhs value
class ArrayAssignment : public Node {
 private:
  std::string name_;
  NodePtr index_;
  NodePtr rhs_;

 public:
  ArrayAssignment(std::string name, NodePtr index, NodePtr rhs)
      : name_(std::move(name)),
        index_(std::move(index)),
        rhs_(std::move(rhs)) {}
  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
};

}  // namespace ast
