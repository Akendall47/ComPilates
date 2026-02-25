#include "ast_direct_declarator.hpp"

namespace ast {

void DirectDeclarator::EmitRISC(std::ostream& stream, Context& context) const {
  // emit function label using GetName() avoids calling Identifier::EmitRISC
  // which now does a stack load
  stream << GetName() << ":\n";
  (void)context;
}

void DirectDeclarator::Print(std::ostream& stream) const {
  identifier_->Print(stream);
}

}  // namespace ast
