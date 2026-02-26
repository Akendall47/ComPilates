#pragma once

#include "ast_node.hpp"

namespace ast {

class ReturnStatement : public Node {
 private:
  NodePtr expression_;

 public:
  ReturnStatement(NodePtr expression) : expression_(std::move(expression)) {}

  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
};

// --- skeletons ---

class IfStatement : public Node {
 private:
  NodePtr condition_;
  NodePtr then_;
  NodePtr else_;  // nullptr if no else

 public:
  IfStatement(NodePtr condition, NodePtr then, NodePtr els)
      : condition_(std::move(condition)),
        then_(std::move(then)),
        else_(std::move(els)) {}

  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
};

class WhileStatement : public Node {
 private:
  NodePtr condition_;
  NodePtr body_;

 public:
  WhileStatement(NodePtr condition, NodePtr body)
      : condition_(std::move(condition)), body_(std::move(body)) {}

  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
};

}  // namespace ast
