#include "ast_identifier.hpp"

namespace ast {

void Identifier::EmitRISC(std::ostream& stream, Context& context) const {
  // load variable from its stack slot into scratch reg
  int offset = context.locals.at(identifier_);
  stream << "lw      a5, " << offset << "(s0)\n";
}

void Identifier::Print(std::ostream& stream) const { stream << identifier_; };

}  // namespace ast
