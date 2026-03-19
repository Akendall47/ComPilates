#pragma once

#include "ast_node.hpp"

namespace ast {

class CompoundStatement : public Node {
 private:
  NodePtr statements_;

 public:
  CompoundStatement(NodePtr statements)
      : statements_(std::move(statements)) {}

  const Node* GetStatements() const { return statements_.get(); }

  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
  int CountDeclarations() const override;
};

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
  int CountDeclarations() const override {
    return then_->CountDeclarations() +
           (else_ ? else_->CountDeclarations() : 0);
  }
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
  int CountDeclarations() const override {
    return body_ ? body_->CountDeclarations() : 0;
  }
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
  int CountDeclarations() const override {
    return (init_ ? init_->CountDeclarations() : 0) +
           (body_ ? body_->CountDeclarations() : 0);
  }
};

class CaseStatement : public Node {
 private:
  NodePtr expression_;
  NodePtr statement_;

 public:
  CaseStatement(NodePtr expression, NodePtr statement)
      : expression_(std::move(expression)), statement_(std::move(statement)) {}

  const Node* GetExpression() const { return expression_.get(); }
  const Node* GetStatement() const { return statement_.get(); }

  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
};

class DefaultStatement : public Node {
 private:
  NodePtr statement_;

 public:
  explicit DefaultStatement(NodePtr statement)
      : statement_(std::move(statement)) {}

  const Node* GetStatement() const { return statement_.get(); }

  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
};

class SwitchStatement : public Node {
 private:
  NodePtr expression_;
  NodePtr statement_;

 public:
  SwitchStatement(NodePtr expression, NodePtr statement)
      : expression_(std::move(expression)), statement_(std::move(statement)) {}

  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
  int CountDeclarations() const override {
    return statement_ ? statement_->CountDeclarations() : 0;
  }
};

}  // namespace ast
