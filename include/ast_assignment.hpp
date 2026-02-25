#pragma once

#include "ast_node.hpp"

namespace ast {

class Assignment : public Node {
 private:
  std::string lhs_name_;
  NodePtr rhs_;

 public:
  Assignment(std::string lhs_name, NodePtr rhs)
      : lhs_name_(std::move(lhs_name)), rhs_(std::move(rhs)) {}

  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
};

}  // namespace ast
