#pragma once
#include <iostream>
#include <memory>
#include <vector>

#include "ast_context.hpp"
#include "ast_type_specifier.hpp"

namespace ast {

class Node {
 public:
  virtual ~Node() {}
  virtual void EmitRISC(std::ostream& stream, Context& context) const = 0;
  virtual void Print(std::ostream& stream) const = 0;
  virtual int CountDeclarations() const { return 0; }
  virtual TypeSpecifier GetType(Context&) const { return TypeSpecifier::INT; }
};

using NodePtr = std::unique_ptr<const Node>;

class NodeList : public Node {
 private:
  std::vector<NodePtr> nodes_;

 public:
  NodeList(NodePtr first_node) { nodes_.push_back(std::move(first_node)); }

  const std::vector<NodePtr>& GetNodes() const { return nodes_; }

  void PushBack(NodePtr item);
  virtual void EmitRISC(std::ostream& stream, Context& context) const override;
  virtual void Print(std::ostream& stream) const override;
  virtual int CountDeclarations() const override;
};

}  // namespace ast
