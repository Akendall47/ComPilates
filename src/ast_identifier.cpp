#include "ast_identifier.hpp"

namespace ast {

void Identifier::EmitRISC(std::ostream& stream, Context& context) const {
  // load variable from its stack slot into result reg
  int offset = context.locals.at(identifier_);

  TypeSpecifier type = context.local_type.at(identifier_);

  // Different loads for different types
  if (type == TypeSpecifier::FLOAT) {
      stream << "  flw fa0, " << offset << "(s0)\n";
  } else if (type == TypeSpecifier::DOUBLE) {
      stream << "  fld fa0, " << offset << "(s0)\n";
  } else {
      stream << "  lw a0, " << offset << "(s0)\n";
  }
}

void Identifier::Print(std::ostream& stream) const { stream << identifier_; }

TypeSpecifier Identifier::GetType(Context& context) const { return context.local_type.at(identifier_); }

}  // namespace ast
