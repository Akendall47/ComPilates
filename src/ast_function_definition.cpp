#include "ast_function_definition.hpp"

#include <sstream>

namespace ast {

void FunctionDefinition::EmitRISC(std::ostream& stream,
                                  Context& context) const {
  // extract func name by get declarators print output
  std::ostringstream name_stream;
  declarator_->Print(name_stream);
  context.current_function = name_stream.str();

  stream << ".text\n";
  stream << ".globl " << context.current_function << "\n";

  declarator_->EmitRISC(stream, context);  // emits "funcname:\n"
  context.EmitPrologue(stream);

  if (compound_statement_ != nullptr) {
    compound_statement_->EmitRISC(stream, context);
  }
}

void FunctionDefinition::Print(std::ostream& stream) const {
  stream << declaration_specifiers_ << " ";

  declarator_->Print(stream);
  stream << "() {" << std::endl;

  if (compound_statement_ != nullptr) {
    compound_statement_->Print(stream);
  }
  stream << "}" << std::endl;
}

}  // namespace ast
