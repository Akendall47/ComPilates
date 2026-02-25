#include "ast_function_definition.hpp"

#include "ast_direct_declarator.hpp"

namespace ast {

void FunctionDefinition::EmitRISC(std::ostream& stream,
                                  Context& context) const {
  // extract func name via GetName() — declarator_ is always a DirectDeclarator
  auto& dd = dynamic_cast<const DirectDeclarator&>(*declarator_);
  context.current_function = dd.GetName();

  stream << ".text\n";
  stream << ".globl " << context.current_function << "\n";

  declarator_->EmitRISC(stream, context);  // emits "funcname:\n"

  // prepass count locals to calculate correct stack frame size
  int num_locals =
      compound_statement_ ? compound_statement_->CountDeclarations() : 0;
  context.stack_frame_size = (16 + num_locals * 4 + 15) & ~15;
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
