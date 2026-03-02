#pragma once

#include <string>

#include "ast_node.hpp"

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
