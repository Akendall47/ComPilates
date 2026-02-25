#pragma once

#include "ast_node.hpp"

namespace ast {

class Identifier : public Node {
 private:
  std::string identifier_;

 public:
  Identifier(std::string identifier) : identifier_(std::move(identifier)) {};

  const std::string& GetName() const { return identifier_; }
  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
};

}  // namespace ast
