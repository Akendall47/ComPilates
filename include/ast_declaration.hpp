#pragma once

#include "ast_node.hpp"
#include "ast_type_specifier.hpp"

namespace ast {

class Declaration : public Node {
 private:
  TypeSpecifier type_;
  std::string name_;
  int pointer_depth_;  // 0 = plain, 1 = int*, 2 = int** etc.

 public:
  Declaration(TypeSpecifier type, std::string name, int pointer_depth = 0)
      : type_(type), name_(std::move(name)), pointer_depth_(pointer_depth) {}

  // prepass;  this node is a declaration, so count as 1
  int CountDeclarations() const override { return 1; }
  const std::string& GetName() const { return name_; }

  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
  virtual TypeSpecifier GetType(Context& context) const override;
};
// int x = expr;  or  int *p = expr;
// allocs the stack slot then stores the initial value
class InitDeclaration : public Node {
 private:
  TypeSpecifier type_;
  std::string name_;
  int pointer_depth_;
  NodePtr init_;

 public:
  InitDeclaration(TypeSpecifier type, std::string name, int pointer_depth,
                  NodePtr init)
      : type_(type),
        name_(std::move(name)),
        pointer_depth_(pointer_depth),
        init_(std::move(init)) {}

  int CountDeclarations() const override { return 1; }

  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
};

}  // namespace ast
