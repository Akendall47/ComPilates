#include "ast_context.hpp"

std::unordered_set<std::string> global_typedef_names;
std::unordered_map<std::string, ast::QualifiedType> global_type_aliases;
int global_typedef_pointer_depth = 0;
std::unordered_map<std::string, int> global_enum_names;
int global_enum_next_value = 0;
