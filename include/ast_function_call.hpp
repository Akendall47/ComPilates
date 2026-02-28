#pragma once

#include <string>
#include "ast_node.hpp"

namespace ast {

class FunctionCall : public Node {
 private:
  std::string name_;
  NodePtr arguments_;

 public:
  FunctionCall(std::string name, NodePtr arguments) : name_(std::move(name)), arguments_(std::move(arguments)) {}

  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
};

}  // namespace ast
