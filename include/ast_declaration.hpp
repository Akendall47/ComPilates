#pragma once

#include "ast_node.hpp"
#include "ast_type_specifier.hpp"

namespace ast {

class Declaration : public Node {
 private:
  TypeSpecifier type_;
  std::string name_;

 public:
  Declaration(TypeSpecifier type, std::string name)
      : type_(type), name_(std::move(name)) {}

  // prepass;  this node is a declaration, so count as 1
  int CountDeclarations() const override { return 1; }
  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
};
}  // namespace ast
