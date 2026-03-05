#pragma once

#include <ostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "ast_type_specifier.hpp"

namespace ast {

class Context {
 public:
  std::string current_function;
  int stack_frame_size = 16;

  // symbol table: name -> s0-relative offset
  std::unordered_map<std::string, int> locals;
  // type table: name -> VarType (base type + pointer depth)
  std::unordered_map<std::string, VarType> local_types;

  // global arrays declared at file scope
  std::unordered_set<std::string> globals;
  void RegisterGlobal(const std::string& name) { globals.insert(name); }

  // label counter for control flow
  int label_count = 0;
  int AllocateLabel() { return label_count++; }

  // tracks next free byte offset from s0
  int next_local_offset = -8;

  int GetTypeSize(TypeSpecifier type) {
    switch (type) {
      case TypeSpecifier::DOUBLE: return 8;
      default: return 4;  // int, float
    }
  }

  // allocate stack space for a scalar; records type for pointer arithmetic
  int AllocateLocal(const std::string& name, TypeSpecifier type = TypeSpecifier::INT,
                    int array_length = 1, int pointer_depth = 0) {
    int element_size = GetTypeSize(type);
    int total_size = element_size * array_length;
    next_local_offset -= total_size;
    locals[name] = next_local_offset;
    local_types[name] = VarType{type, pointer_depth};
    return next_local_offset;
  }

  void EmitPrologue(std::ostream& stream) const {
    stream << "addi    sp, sp, -" << stack_frame_size << "\n";
    stream << "sw      ra, " << (stack_frame_size - 4) << "(sp)\n";
    stream << "sw      s0, " << (stack_frame_size - 8) << "(sp)\n";
    stream << "addi    s0, sp, " << stack_frame_size << "\n";
  }

  void EmitEpilogue(std::ostream& stream) const {
    stream << "lw      ra, " << (stack_frame_size - 4) << "(sp)\n";
    stream << "lw      s0, " << (stack_frame_size - 8) << "(sp)\n";
    stream << "addi    sp, sp, " << stack_frame_size << "\n";
  }
};

}  // namespace ast
