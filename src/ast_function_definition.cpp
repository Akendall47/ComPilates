#include "ast_function_definition.hpp"

#include "ast_direct_declarator.hpp"
#include "ast_declaration.hpp"

namespace ast {

void FunctionDefinition::EmitRISC(std::ostream& stream,
                                  Context& context) const {
  // extract func name via GetName() — declarator_ is always a DirectDeclarator
  auto& dd = dynamic_cast<const DirectDeclarator&>(*declarator_);
  context.current_function = dd.GetName();

  stream << ".text\n";
  stream << ".globl " << context.current_function << "\n";

  declarator_->EmitRISC(stream, context);  // emits "funcname:\n"

  // Get Function parameters
  const NodeList* params = nullptr;
  if (auto params_node = dd.GetParameters()) {
    params = dynamic_cast<const NodeList*>(params_node);
  }

  // Get number of paramets as well as locals to take off stake frame
  int num_params = params ? params->CountDeclarations() : 0;
  int num_locals =(compound_statement_ ? compound_statement_->CountDeclarations() : 0) + num_params;

  context.stack_frame_size = (16 + num_locals * 4 + 15) & ~15;
  context.EmitPrologue(stream);

  // Stack/register fixing code to fix over spilling
  if (params) {
    int arg_reg = 0;
    for (const auto& node : params->GetNodes()) {
      node->EmitRISC(stream, context);

      if (arg_reg < 8) {
        auto decl = dynamic_cast<const Declaration*>(node.get());
        if (decl) {
          int offset = context.locals.at(decl->GetName());
          stream << "  sw a" << arg_reg++ << ", " << offset << "(s0)"
                 << std::endl;
        }
      }
    }
  }

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
