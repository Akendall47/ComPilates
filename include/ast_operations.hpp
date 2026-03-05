#pragma once

#include <string>

#include "ast_node.hpp"
#include "ast_type_specifier.hpp"

namespace ast {

class BinaryOp : public Node {
 private:
  NodePtr left_;
  NodePtr right_;
  std::string op_;

 public:
  BinaryOp(NodePtr left, const std::string& op, NodePtr right)
      : left_(std::move(left)), right_(std::move(right)), op_(op) {}

  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
  virtual TypeSpecifier GetType(Context& context) const override;
};

class UnaryOp : public Node {
 private:
  NodePtr operand_;
  std::string op_;

 public:
  UnaryOp(const std::string& op, NodePtr operand)
      : operand_(std::move(operand)), op_(op) {}

  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
};

// &x — computes the address of its operand (must be an Identifier lvalue)
// EmitRISC: addi a0, s0, <offset>  (no memory load — just the address itself)
class AddressOf : public ast::Node {
 private:
  std::string name_;

 public:
  explicit AddressOf(std::string name) : name_(std::move(name)) {}

  void EmitRISC(std::ostream& stream, ast::Context& context) const override;
  void Print(std::ostream& stream) const override;
};

class Deref : public ast::Node {
 private:
  ast::NodePtr operand_;

 public:
  explicit Deref(ast::NodePtr operand) : operand_(std::move(operand)) {}

  // transfer ownership of the inner operand out so DerefAssign can own it
  ast::NodePtr TakeOperand() { return std::move(operand_); }

  void EmitRISC(std::ostream& stream, ast::Context& context) const override;
  void Print(std::ostream& stream) const override;
};

class DerefAssign : public ast::Node {
 private:
  ast::NodePtr ptr_;
  ast::NodePtr rhs_;

 public:
  DerefAssign(ast::NodePtr ptr, ast::NodePtr rhs)
      : ptr_(std::move(ptr)), rhs_(std::move(rhs)) {}

  void EmitRISC(std::ostream& stream, ast::Context& context) const override;
  void Print(std::ostream& stream) const override;
};

// Posts:  x++, x--
//  Operand must be an Identifier (lvalue). Loads, modifies, stores back.
class PostfixOp : public Node {
 private:
  NodePtr operand_;
  std::string op_;  // "++" or "--"

 public:
  PostfixOp(const std::string& op, NodePtr operand)
      : operand_(std::move(operand)), op_(op) {}

  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
};

}
