// Adapted from: https://www.lysator.liu.se/c/ANSI-C-grammar-y.html

// TODO: you can either continue adding to this grammar file or
// rename parser_full.y to parser.y once you're happy with
// how this example works.

%code requires {
	#include "ast.hpp"
	using namespace ast;

	extern int yylineno;
	extern char* yytext;
	extern Node* g_root;
	extern FILE* yyin;

	int yylex(void);
	void yyerror(const char*);
	int yylex_destroy(void);
}

%define parse.error detailed
%define parse.lac full

%union {
  Node*				    node;
  NodeList*			  node_list;
  int          		number_int;
  double       		number_float;
  std::string*		string;
  TypeSpecifier 	type_specifier;
  yytokentype  		token;
}

%token IDENTIFIER INT_CONSTANT FLOAT_CONSTANT STRING_LITERAL
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP AND_OP OR_OP
%token MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN XOR_ASSIGN OR_ASSIGN
%token TYPE_NAME TYPEDEF EXTERN STATIC AUTO REGISTER SIZEOF
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token STRUCT UNION ENUM ELLIPSIS
%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN
%token UNKNOWN

%type <node> translation_unit external_declaration function_definition primary_expression postfix_expression
%type <node> unary_expression cast_expression multiplicative_expression additive_expression shift_expression relational_expression
%type <node> equality_expression and_expression exclusive_or_expression inclusive_or_expression logical_and_expression logical_or_expression
%type <node> conditional_expression assignment_expression expression declarator direct_declarator statement compound_statement jump_statement
%type <node> parameter_list parameter_declaration declaration
%type <node> expression_statement selection_statement iteration_statement


%type <node_list> statement_list

%type <number_int> INT_CONSTANT STRING_LITERAL
%type <number_float> FLOAT_CONSTANT
%type <string> IDENTIFIER
%type <type_specifier> type_specifier declaration_specifiers


%start ROOT
%%

ROOT
	: translation_unit { g_root = $1; }

translation_unit
	: external_declaration { $$ = $1; }
	;

external_declaration
	: function_definition { $$ = $1; }
	;

function_definition
	: declaration_specifiers declarator compound_statement {
		$$ = new FunctionDefinition($1, NodePtr($2), NodePtr($3));
	}
	;

declaration_specifiers
	: type_specifier { $$ = $1; }
	;

type_specifier
	: INT {
		$$ = TypeSpecifier::INT;
	}
	;

declarator
	: direct_declarator { $$ = $1; }
	;

direct_declarator
	: IDENTIFIER {
		$$ = new Identifier(std::move(*$1));
		delete $1;
	}
	| direct_declarator '(' ')' {
		$$ = new DirectDeclarator(NodePtr($1));
	}
	| direct_declarator '(' parameter_list ')'{
		$$ = new DirectDeclarator(NodePtr($1));
		delete $3;  // params parsed but discarded for now
	}
	;

parameter_list
	: parameter_declaration { $$ = $1; }
	| parameter_list ',' parameter_declaration { $$ = $1; delete $3; }  // free discarded param node
	;

//reminder eg: "int x" - $1=type, $2=name, return name node
parameter_declaration
	: declaration_specifiers declarator { $$ = $2; }
	;

declaration
	: declaration_specifiers declarator ';' {
		//static cast may be more effieicent, but less safe, especially as we groww
		Identifier* id = dynamic_cast<Identifier*>($2);
		$$ = new Declaration($1, id->GetName());
		delete $2;
	}
	;

expression_statement
	: expression ';' { $$ = $1; }
	;

selection_statement
	: IF '(' expression ')' statement{
		$$ = new IfStatement(NodePtr($3), NodePtr($5), nullptr);
	}
	| IF '(' expression ')' statement ELSE statement {
		$$ = new IfStatement(NodePtr($3), NodePtr($5), NodePtr($7));
	}
	;

iteration_statement
	: WHILE '(' expression ')' statement{
		$$ = new WhileStatement(NodePtr($3), NodePtr($5));
	}
	| FOR '(' expression ';' expression ';' expression ')' statement {
		$$ = new ForStatement(NodePtr($3), NodePtr($5), NodePtr($7), NodePtr($9));
	}
	;

//ie if (1)
statement
	: jump_statement { $$ = $1; }
	| expression_statement { $$ = $1; } //ie x=10; ex_st vs x=10 just an ex
	| compound_statement { $$ = $1; }
	| selection_statement { $$ = $1;}
	| iteration_statement { $$ = $1; }
	;

compound_statement
	: '{' statement_list '}' { $$ = $2; }
	| '{' '}' { $$ = nullptr; }
	;

statement_list
	: statement { $$ = new NodeList(NodePtr($1)); }
	| declaration { $$ = new NodeList(NodePtr($1)); }
	| statement_list statement { $1->PushBack(NodePtr($2)); $$ = $1; }
	| statement_list declaration { $1->PushBack(NodePtr($2)); $$ = $1; }
	;

jump_statement
	: RETURN ';' {
		$$ = new ReturnStatement(nullptr);
	}
	| RETURN expression ';' {
		$$ = new ReturnStatement(NodePtr($2));
	}
	;

primary_expression
	: INT_CONSTANT {
		$$ = new IntConstant($1);
	}
	| IDENTIFIER {
		$$ = new Identifier(std::move(*$1));
		delete $1;
	}
	;

postfix_expression
	: primary_expression
	;

unary_expression
	: postfix_expression
	;

cast_expression
	: unary_expression
	;

multiplicative_expression
	: cast_expression
	;

additive_expression
	: multiplicative_expression
	;

shift_expression
	: additive_expression
	;

relational_expression
	: shift_expression
	;

equality_expression
	: relational_expression
	;

and_expression
	: equality_expression
	;

exclusive_or_expression
	: and_expression
	;

inclusive_or_expression
	: exclusive_or_expression
	;

logical_and_expression
	: inclusive_or_expression
	;

logical_or_expression
	: logical_and_expression
	;

conditional_expression
	: logical_or_expression
	;

assignment_expression
	: conditional_expression
	| unary_expression '=' assignment_expression {
		// extract name from lhs Identifier, build Assignment node owning rhs

		Identifier* id = dynamic_cast<Identifier*>($1);
		$$ = new Assignment(id->GetName(), NodePtr($3));
		delete $1;
	}
	;

expression
	: assignment_expression
	;

%%

void yyerror (const char *s)
{
  std::cerr << "Error: " << s << " at line " << yylineno;
  std::cerr << " near '" << yytext << "'" << std::endl;
  std::exit(1);
}

Node* g_root;

NodePtr ParseAST(std::string file_name)
{
  yyin = fopen(file_name.c_str(), "r");
  if (yyin == nullptr) {
    std::cerr << "Couldn't open input file: " << file_name << std::endl;
    std::exit(1);
  }

  g_root = nullptr;
  yyparse();

  fclose(yyin);
  yylex_destroy();

  return NodePtr(g_root);
}
