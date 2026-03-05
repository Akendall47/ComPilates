#pragma once

#include <stdexcept>
#include <string_view>

namespace ast {

enum class TypeSpecifier { INT, FLOAT, DOUBLE };

// QualifiedType: base + ptr depth e.g. int*, {INT, 1}
// ie the type + extra quals like ptr depth etc etc
struct QualifiedType {
  TypeSpecifier base = TypeSpecifier::INT;
  int pointer_depth = 0;

  bool is_pointer() const { return pointer_depth > 0; }
  int pointee_size() const { return 4; }  // only int for now
  int size() const { return 4; }
};

template <typename LogStream>
LogStream& operator<<(LogStream& ls, const TypeSpecifier& type) {
  const auto TypeToString = [&type] {
    switch (type) {
      case TypeSpecifier::INT:
        return "int";
      case TypeSpecifier::FLOAT:
        return "float";
      case TypeSpecifier::DOUBLE:
        return "double";
    }
    throw std::runtime_error("Unexpected type specifier");
  };
  return ls << TypeToString();
}

}  // namespace ast
