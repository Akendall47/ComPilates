#pragma once

#include <string>

#include "ast_node.hpp"

class BinaryOp : public ast::Node {
 private:
  ast::NodePtr left_;
  ast::NodePtr right_;
  std::string op_;

 public:
  BinaryOp(ast::NodePtr left, const std::string& op, ast::NodePtr right)
      : left_(std::move(left)), right_(std::move(right)), op_(op) {}

  void EmitRISC(std::ostream& stream, ast::Context& context) const override;
  void Print(std::ostream& stream) const override;
};

class UnaryOp : public ast::Node {
 private:
  ast::NodePtr operand_;
  std::string op_;

 public:
  UnaryOp(const std::string& op, ast::NodePtr operand)
      : operand_(std::move(operand)), op_(op) {}

  void EmitRISC(std::ostream& stream, ast::Context& context) const override;
  void Print(std::ostream& stream) const override;
};
