#pragma once

#include "ast_identifier.hpp"
#include "ast_node.hpp"

namespace ast {

class DirectDeclarator : public Node {
 private:
  NodePtr identifier_;

 public:
  DirectDeclarator(NodePtr identifier) : identifier_(std::move(identifier)){};

  // returns the declard name, safe downcast since identifier_ is always an
  // Identifier
  const std::string& GetName() const {
    return dynamic_cast<const Identifier&>(*identifier_).GetName();
  }

  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
};

}  // namespace ast
