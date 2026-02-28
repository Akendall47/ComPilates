#include "ast_function_call.hpp"


namespace ast {

void FunctionCall::EmitRISC(std::ostream& stream, Context& context) const {
  // Vector of argument registers, will clean up later
  std::vector<std::string> arg_regs = {"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};

  int num_args = 0;
  if (arguments_ != nullptr) {
    const auto* argument_list = dynamic_cast<const NodeList*>(arguments_.get());
    num_args = argument_list->GetNodes().size();

    // Check each argument and put on stack
    for (const auto& arg : argument_list->GetNodes()) {
      arg->EmitRISC(stream, context);
      stream << "  addi sp, sp, -4\n";
      stream << "  sw a0, 0(sp)\n";
    }

    // Load registers off stack 1 by 1 in opposite order to ensure a0 is for final result
    for (int i = num_args - 1; i >= 0; i--) {
        if (i < 8) {
            stream << "  lw " << arg_regs[i] << ", 0(sp)\n";
            stream << "  addi sp, sp, 4\n";
        }
    }
  }

  stream << "  call " << name_ << "\n";
}

void FunctionCall::Print(std::ostream& stream) const {
  stream << name_ << "(";
  if (arguments_){
    arguments_->Print(stream);}
  stream << ")";
}

}  // namespace ast
