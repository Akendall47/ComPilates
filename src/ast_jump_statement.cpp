#include "ast_jump_statement.hpp"

namespace ast {

void ReturnStatement::EmitRISC(std::ostream& stream, Context& context) const {
  if (expression_ != nullptr) {
    expression_->EmitRISC(stream, context);  // result lands in a5
    stream << "mv      a0, a5\n";            // move to return reg
  }
  context.EmitEpilogue(stream);
  stream << "ret\n";  // epilogue completes, jump back to caller via ra
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
