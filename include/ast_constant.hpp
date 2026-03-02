#pragma once

#include "ast_node.hpp"

namespace ast {

class IntConstant : public Node {
 private:
  int value_;

 public:
  IntConstant(int value) : value_(value) {}

  int GetValue() const { return value_; }

  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
};

class FloatConstant : public Node {
 private:
  double value_;

 public:
  FloatConstant(double value) : value_(value) {}

  double GetValue() const { return value_; }

  void EmitRISC(std::ostream& stream, Context& context) const override;
  void Print(std::ostream& stream) const override;
  virtual TypeSpecifier GetType(Context& context) const override;
};

}  // namespace ast
