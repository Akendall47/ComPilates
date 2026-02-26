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

  int CountDeclarations() const override { return 1; }
  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
};

class DirectDeclarator : public Node {
 private:
  NodePtr identifier_;

 public:
  DirectDeclarator(NodePtr identifier) : identifier_(std::move(identifier)) {}

  // safe downcast — identifier_ is always an Identifier
  const std::string& GetName() const;

  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
};

class FunctionDefinition : public Node {
 private:
  TypeSpecifier declaration_specifiers_;
  NodePtr declarator_;
  NodePtr compound_statement_;

 public:
  FunctionDefinition(TypeSpecifier declaration_specifiers, NodePtr declarator,
                     NodePtr compound_statement)
      : declaration_specifiers_(declaration_specifiers),
        declarator_(std::move(declarator)),
        compound_statement_(std::move(compound_statement)) {}

  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
};

}  // namespace ast
