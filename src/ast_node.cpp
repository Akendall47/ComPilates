#include "ast_node.hpp"

namespace ast {

// transfer ownership of item into the list
void NodeList::PushBack(NodePtr item) { nodes_.push_back(std::move(item)); }

// pre pass; sum declartions across all children for stack frame sizing
int NodeList::CountDeclarations() const {
  int count = 0;
  for (const auto& node : nodes_) {
    if (node != nullptr) {
      count += node->CountDeclarations();
    }
  }
  return count;
}

void NodeList::EmitRISC(std::ostream& stream, Context& context) const {
  for (const auto& node : nodes_) {
    if (node == nullptr) {
      continue;
    }
    node->EmitRISC(stream, context);
  }
}

void NodeList::Print(std::ostream& stream) const {
  for (const auto& node : nodes_) {
    if (node == nullptr) {
      continue;
    }
    node->Print(stream);
  }
}

}  // namespace ast
