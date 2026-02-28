#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "ast_array.hpp"
#include "ast_assignment.hpp"
#include "ast_constant.hpp"
#include "ast_context.hpp"
#include "ast_declaration.hpp"
#include "ast_direct_declarator.hpp"
#include "ast_function_definition.hpp"
#include "ast_identifier.hpp"
#include "ast_jump_statement.hpp"
#include "ast_function_call.hpp"
#include "ast_node.hpp"
#include "ast_operations.hpp"
#include "ast_statements.hpp"
#include "ast_type_specifier.hpp"

ast::NodePtr ParseAST(std::string file_name);
