#include "ast_function_call.hpp"


namespace ast {

void FunctionCall::EmitRISC(std::ostream& stream,
                                  Context& context) const {
  int arg_reg = 0;
  if (arguments_ != nullptr){
    const auto* argument_list = dynamic_cast<const NodeList*>(arguments_.get());


    for (const auto& arg : argument_list->GetNodes()){
      arg->EmitRISC(stream, context);
      arg_reg++;
    }
  }

  for (int i = arg_reg - 1; i >= 0; i--){
    stream << "  lw a" << i << ", 0(sp)\n";
    stream << "  addi sp, sp, 4\n";
  }

  // Call function
  stream << " call " << name_ << "\n";
}

void FunctionCall::Print(std::ostream& stream) const {
  stream << name_ << "(";
  if (arguments_){
    arguments_->Print(stream);}
  stream << ")";
}

}  // namespace ast
