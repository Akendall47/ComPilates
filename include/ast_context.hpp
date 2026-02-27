#pragma once

#include <ostream>
#include <string>
#include <unordered_map>

namespace ast {
// An object of class Context is passed between ast nodes during compilation.
// This can be used to pass around information about what's currently being
// compiled (e.g. function scope and variable names).
class Context {
 public:
  std::string current_function;
  int stack_frame_size = 16;

  // basic symbol table for now; maps variable name to offset from s0
  std::unordered_map<std::string, int> locals;

  // label counter for control flow; returns unique int eg .Lend_0, .Lfor_1
  int label_count = 0;
  int AllocateLabel() { return label_count++; }

  // allocate next stack slot, returns its offset from s0
  int AllocateLocal(const std::string& name) {
    int offset = -(int)((locals.size() + 1) * 4 + 8);
    locals[name] = offset;
    return offset;
  }

  // ostream base class, so method works with any of its derivatives
  /*confirmed with GodBolt*/

  // grow stack, save return address, save old frame ptr, set fp to top
  // 16 byte aligned remember; init to 16: ra; 4bytes, s0: 4 bytes, -> align to
  // 16

  void EmitPrologue(std::ostream& stream) const {
    stream << "addi    sp, sp, -" << stack_frame_size << "\n";  // new bottom
    stream << "sw      ra, " << (stack_frame_size - 4) << "(sp)\n";
    stream << "sw      s0, " << (stack_frame_size - 8) << "(sp)\n";
    stream << "addi    s0, sp, " << stack_frame_size << "\n";  // new s0 to top
  }

  // restore return addr, restore old fp, shrink stack, can then jump
  void EmitEpilogue(std::ostream& stream) const {
    stream << "lw      ra, " << (stack_frame_size - 4) << "(sp)\n";
    stream << "lw      s0, " << (stack_frame_size - 8) << "(sp)\n";
    stream << "addi    sp, sp, " << stack_frame_size << "\n";
  }
};

}  // namespace ast
