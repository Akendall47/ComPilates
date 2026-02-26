#include "ast_statements.hpp"

namespace ast {

// IF

void IfStatement::EmitRISC(std::ostream& stream, Context& context) const {
  int label = context.AllocateLabel();

  condition_->EmitRISC(stream, context);

  if (else_ != nullptr) {
    stream << "beqz    a5, .Lelse_" << label << "\n";
    then_->EmitRISC(stream, context);
    stream << "j       .Lend_" << label << "\n";
    stream << ".Lelse_" << label << ":\n";
    else_->EmitRISC(stream, context);
  } else {
    stream << "beqz    a5, .Lend_" << label << "\n";
    then_->EmitRISC(stream, context);
  }
  stream << ".Lend_" << label << ":\n";
}

void IfStatement::Print(std::ostream& stream) const {
  stream << "if (";
  condition_->Print(stream);
  stream << ") {\n";
  then_->Print(stream);
  stream << "}";
  if (else_ != nullptr) {
    stream << " else {\n";
    else_->Print(stream);
    stream << "}";
  }
  stream << "\n";
}

// WHILE

void WhileStatement::EmitRISC(std::ostream& stream, Context& context) const {
  int label = context.AllocateLabel();

  stream << ".Lwhile_" << label << ":\n";
  condition_->EmitRISC(stream, context);  // result going to a5
  stream << "beqz    a5, .Lend_" << label << "\n";
  if (body_ != nullptr) {
    body_->EmitRISC(stream, context);
  }
  stream << "j       .Lwhile_" << label << "\n";
  stream << ".Lend_" << label << ":\n";
}

void WhileStatement::Print(std::ostream& stream) const {
  stream << "while (";
  condition_->Print(stream);
  stream << ") {\n";
  if (body_ != nullptr) {
    body_->Print(stream);
  }
  stream << "}\n";
}

}  // namespace ast
