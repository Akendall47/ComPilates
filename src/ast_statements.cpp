#include "ast_statements.hpp"

#include <vector>

namespace ast {

struct CaseTarget {
    const CaseStatement* node;
    int label;
  };

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
  context.PushBreakTarget(label);
  context.PushContinueTarget(label, false);
  if (body_ != nullptr) {
    body_->EmitRISC(stream, context);
  }
  context.PopContinueTarget();
  context.PopBreakTarget();
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
  int update_label = context.AllocateLabel();

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

  context.PushBreakTarget(label);
  context.PushContinueTarget(update_label, true);
  if (body_ != nullptr) {
    body_->EmitRISC(stream, context);
  }
  context.PopContinueTarget();
  context.PopBreakTarget();

  // eg  x=x+1
  stream << ".Lfor_update_" << update_label << ":\n";
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

void CaseStatement::EmitRISC(std::ostream& stream, Context& context) const {
  if (statement_ != nullptr) {
    statement_->EmitRISC(stream, context);
  }
}

void CaseStatement::Print(std::ostream& stream) const {
  stream << "case ";
  if (expression_ != nullptr) {
    expression_->Print(stream);
  }
  stream << ":\n";
  if (statement_ != nullptr) {
    statement_->Print(stream);
  }
}

void DefaultStatement::EmitRISC(std::ostream& stream, Context& context) const {
  if (statement_ != nullptr) {
    statement_->EmitRISC(stream, context);
  }
}

void DefaultStatement::Print(std::ostream& stream) const {
  stream << "default:\n";
  if (statement_ != nullptr) {
    statement_->Print(stream);
  }
}

void SwitchStatement::EmitRISC(std::ostream& stream, Context& context) const {
  int end_label = context.AllocateLabel();

  const DefaultStatement* default_node = nullptr;
  int default_label = -1;
  std::vector<CaseTarget> cases;

  // small stack to get labels
  std::vector<const Node*> collect_stack;
  collect_stack.push_back(statement_.get());
  while (!collect_stack.empty()) {
    const Node* node = collect_stack.back();
    collect_stack.pop_back();

    if (node == nullptr){
      continue;
    }

    if (const auto* list = dynamic_cast<const NodeList*>(node)){
      const auto& nodes = list->GetNodes();
      for (size_t i = nodes.size(); i > 0; --i) {
        collect_stack.push_back(nodes[i - 1].get());
      }
      continue;
    }

    if (const auto* statement = dynamic_cast<const CaseStatement*>(node)) {
      cases.push_back(CaseTarget{statement, context.AllocateLabel()});
      collect_stack.push_back(statement->GetStatement());
      continue;
    }

    if (const auto* default_stmt = dynamic_cast<const DefaultStatement*>(node)) {
      if (default_node == nullptr) {
        default_node = default_stmt;
        default_label = context.AllocateLabel();
      }
      collect_stack.push_back(default_stmt->GetStatement());
    }
  }

  expression_->EmitRISC(stream, context);
  stream << "addi    sp, sp, -4\n";
  stream << "sw      a0, 0(sp)\n";

  for (const auto& item : cases) {
    stream << "lw      t1, 0(sp)\n";
    const Node* case_expression = item.node->GetExpression();
    if (case_expression != nullptr) {
      case_expression->EmitRISC(stream, context);
      stream << "beq     t1, a0, .Lcase_match_" << item.label << "\n";
    }
  }

  stream << "addi    sp, sp, 4\n";
  if (default_label >= 0) {
    stream << "j       .Ldefault_" << default_label << "\n";
  } else {
    stream << "j       .Lend_" << end_label << "\n";
  }

  for (const auto& item : cases) {
    stream << ".Lcase_match_" << item.label << ":\n";
    stream << "addi    sp, sp, 4\n";
    stream << "j       .Lcase_" << item.label << "\n";
  }

  context.PushBreakTarget(end_label);
  std::vector<const Node*> emit_stack;
  emit_stack.push_back(statement_.get());
  while (!emit_stack.empty()) {
    const Node* node = emit_stack.back();
    emit_stack.pop_back();

    if (node == nullptr) {continue;}

    if (const auto* list = dynamic_cast<const NodeList*>(node)){
      const auto& nodes = list->GetNodes();

      for (size_t i = nodes.size(); i > 0; --i) {
        emit_stack.push_back(nodes[i - 1].get());
      } continue;
    }

    if (const auto* statement = dynamic_cast<const CaseStatement*>(node)) {
      int case_label = -1;
      for (const auto& item : cases) {
        if (item.node == statement) {
          case_label = item.label;
          break;
        }
      }
      if (case_label >= 0) {
        stream << ".Lcase_" << case_label << ":\n";
      }
      emit_stack.push_back(statement->GetStatement());
      continue;
    }

    if (const auto* default_stmt = dynamic_cast<const DefaultStatement*>(node)) {
      if (default_node == default_stmt && default_label >= 0) {
        stream << ".Ldefault_" << default_label << ":\n";
      }
      emit_stack.push_back(default_stmt->GetStatement());
      continue;
    }

    node->EmitRISC(stream, context);
  }

  context.PopBreakTarget();
  stream << ".Lend_" << end_label << ":\n";
}

void SwitchStatement::Print(std::ostream& stream) const {
  stream << "switch (";
  expression_->Print(stream);
  stream << ") ";
  if (statement_ != nullptr) {
    statement_->Print(stream);
  }
}

}  // namespace ast
