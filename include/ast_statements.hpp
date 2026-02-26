#pragma once

#include "ast_node.hpp"

namespace ast {

class IfStatement : public Node {
 private:  // the data
  NodePtr condition_;
  NodePtr then_;
  NodePtr else_;

 public:  // what to do with data
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

class ForStatement : public Node {
 private:
  NodePtr init_;
  NodePtr condition_;
  NodePtr update_;
  NodePtr body_;

 public:
  ForStatement(NodePtr init, NodePtr condition, NodePtr update, NodePtr body)
      : init_(std::move(init)),
        condition_(std::move(condition)),
        update_(std::move(update)),
        body_(std::move(body)) {}

  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
};

}  // namespace ast
