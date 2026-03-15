#pragma once

#include <ostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ast_type_specifier.hpp"

extern std::unordered_set<std::string> global_typedef_names;  // Need to make global so parser can see
extern std::unordered_map<std::string, ast::QualifiedType> global_type_aliases; // type alias table for type def
extern int global_typedef_pointer_depth;

extern std::unordered_map<std::string, int> global_enum_names;
extern int global_enum_next_value;
namespace ast {
struct StructMember {
  std::string name;
  int offset;  // byte offset from struct base
  int size;    // bytes
};

struct StructDef {
  std::vector<StructMember> members;
  int total_size;  // bytes
};

struct ContinueTarget {
  int label;
  bool update;
};

class Context {
 public:
  // struct type registry: struct name -> definition
  std::unordered_map<std::string, StructDef> struct_types;

  std::string current_function;
  int stack_frame_size = 16;

  // symbol table: name -> s0-relative offset (base of struct for structs)
  std::unordered_map<std::string, int> locals;

  // type table: name -> QualifiedType (base type + pointer depth)
  std::unordered_map<std::string, QualifiedType> local_types;

  // struct instance name -> struct type name
  std::unordered_map<std::string, std::string> local_struct_type;

  // global arrays declared at file scope
  std::unordered_set<std::string> globals;
  void RegisterGlobal(const std::string& name) { globals.insert(name); }

  // label counter for control flow
  int label_count = 0;
  int AllocateLabel() { return label_count++; }

  // vectors to store the next target after break and continue statements
  std::vector<int> break_targets;
  std::vector<ContinueTarget> continue_targets;

  void PushBreakTarget(int label){
    break_targets.push_back(label);
  }

  void PopBreakTarget(){
    if (!break_targets.empty()){
      break_targets.pop_back();
    }
  }

  // Checks if statement includes break
  bool HasBreakTarget() const { return !break_targets.empty(); }
  int GetBreakTarget() const { return break_targets.back(); }

  void PushContinueTarget(int label, bool update = false){
    continue_targets.push_back(ContinueTarget{label, update});
  }
  void PopContinueTarget() {
    if (!continue_targets.empty()) {
      continue_targets.pop_back();
    }
  }
  
  bool HasContinueTarget() const { return !continue_targets.empty(); }
  ContinueTarget GetContinueTarget() const { return continue_targets.back(); }

  // tracks next free byte offset from s0
  int next_local_offset = -8;

  int GetTypeSize(TypeSpecifier type) {
    switch (type) {
      case TypeSpecifier::DOUBLE:
        return 8;
      case TypeSpecifier::CHAR:
      case TypeSpecifier::UCHAR:
        return 1;
      default:
        return 4;  // int, uint, float
    }
  }

  // allocate stack space for a scalar; records type for pointer arithmetic
  int AllocateLocal(const std::string& name,
                    TypeSpecifier type = TypeSpecifier::INT,
                    int array_length = 1, int pointer_depth = 0) {
    int element_size = GetTypeSize(type);
    int total_size = element_size * array_length;

    // round up to 4 byte alignment so each slot is properly aligned on stack
    int alloc_size = (total_size + 3) & ~3;
    next_local_offset -= alloc_size;
    locals[name] = next_local_offset;
    local_types[name] = QualifiedType{type, pointer_depth};
    return next_local_offset;
  }

  // alloc stack space for a struct instance
  int AllocateStruct(const std::string& var_name,
                     const std::string& struct_type_name) {
    const StructDef& def = struct_types.at(struct_type_name);
    next_local_offset -= def.total_size;
    locals[var_name] = next_local_offset;
    local_struct_type[var_name] = struct_type_name;
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
