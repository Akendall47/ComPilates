#pragma once

#include <ostream>
#include <string>

#include "ast_context.hpp"
#include "ast_node.hpp"

namespace ast {

// bytes into .rodata, loads address into a0.
class StringLiteral : public Node {
  std::string raw_;

 public:
  explicit StringLiteral(std::string raw) : raw_(std::move(raw)) {}

  void EmitRISC(std::ostream& stream, Context& context) const override {
    int label = context.AllocateLabel();

    // LC == Literal const
    stream << "  .section .rodata\n";
    stream << ".LC" << label << ":\n";

    const char* p = raw_.c_str();
    if (*p == 'L') p++;  // skip L prefix for now, can have wide strings
    p++;

    while (*p && *p != '"') {
      int ch;
      if (*p == '\\') {
        p++;
        switch (*p) {
          case 'n':
            ch = '\n';
            break;
          case 't':
            ch = '\t';
            break;
          case 'r':
            ch = '\r';
            break;
          case '0':
            ch = '\0';
            break;
          case '\\':
            ch = '\\';
            break;
          case '"':
            ch = '"';
            break;
          case '\'':
            ch = '\'';
            break;
          default:
            ch = *p;
            break;
        }
      } else {
        ch = (unsigned char)*p;
      }
      stream << "  .byte " << ch << "\n";
      p++;
    }
    stream << "  .byte 0\n";
    stream << "  .text\n";
    stream << "  la      a0, .LC" << label << "\n";
  }

  void Print(std::ostream& stream) const override { stream << raw_; }
};

}  // namespace ast
