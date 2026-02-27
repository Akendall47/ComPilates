#include "ast_jump_statement.hpp"

namespace ast {

void ReturnStatement::EmitRISC(std::ostream& stream, Context& context) const {
  if (expression_ != nullptr) {
    expression_->EmitRISC(stream, context);  // result is already in a0
  }
  // Rewrote some return code to fix it to a0 rather than a5 

  context.EmitEpilogue(stream);
  stream << "ret" <<std::endl;  // epilogue completes, jump back to caller via ra
}

void ReturnStatement::Print(std::ostream& stream) const {
  stream << "return";
  if (expression_ != nullptr) {
    stream << " ";
    expression_->Print(stream);
  }
  stream << ";" << std::endl;
}

}  // namespace ast
