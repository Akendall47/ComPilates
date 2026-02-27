#include "ast_statements.hpp"

namespace ast {

// IF

void IfStatement::EmitRISC(std::ostream& stream, Context& context) const {
  int label = context.AllocateLabel();

  condition_->EmitRISC(stream, context);

  if (else_ != nullptr) {
    stream << "beqz    a0, .Lelse_" << label << "\n";
    then_->EmitRISC(stream, context);
    stream << "j       .Lend_" << label << "\n";
    stream << ".Lelse_" << label << ":\n";
    else_->EmitRISC(stream, context);
  } else {
    stream << "beqz    a0, .Lend_" << label << "\n";
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
  condition_->EmitRISC(stream, context);  // result in a0
  stream << "beqz    a0, .Lend_" << label << "\n";
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

// FOR

void ForStatement::EmitRISC(std::ostream& stream, Context& context) const {
  int label = context.AllocateLabel();

  // eg. x=0, run once
  if (init_ != nullptr) {
    init_->EmitRISC(stream, context);
  }

  stream << ".Lfor_" << label << ":\n";

  // condition: result in a0; beqz a0==0 , exits loop if false (0)
  if (condition_ != nullptr) {  // eg for(;;) - infinite
    condition_->EmitRISC(stream, context);
    stream << "beqz    a0, .Lend_" << label << "\n";
  }

  if (body_ != nullptr) {
    body_->EmitRISC(stream, context);
  }

  // eg  x=x+1
  if (update_ != nullptr) {
    update_->EmitRISC(stream, context);
  }

  stream << "j       .Lfor_" << label << "\n";
  stream << ".Lend_" << label << ":\n";
}

void ForStatement::Print(std::ostream& stream) const {
  stream << "for (";
  if (init_ != nullptr) init_->Print(stream);
  stream << "; ";
  if (condition_ != nullptr) condition_->Print(stream);
  stream << "; ";
  if (update_ != nullptr) update_->Print(stream);
  stream << ") {\n";
  if (body_ != nullptr) body_->Print(stream);
  stream << "}\n";
}

}  // namespace ast
