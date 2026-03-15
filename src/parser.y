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

//lexer-parser bridge
//eg yylval.string lives in string field as temp spot
%union {
  Node*				        node;
  NodeList*			        node_list;
  int          		        number_int;
  double       		        number_float;
  std::string*		        string;
  TypeSpecifier 	        type_specifier;
  yytokentype  		        token;
  std::vector<std::string>* string_vec;
}

%token IDENTIFIER INT_CONSTANT FLOAT_CONSTANT STRING_LITERAL
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP AND_OP OR_OP
%token MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN XOR_ASSIGN OR_ASSIGN
%token TYPE_NAME TYPEDEF EXTERN STATIC AUTO REGISTER SIZEOF
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token STRUCT UNION ENUM ELLIPSIS
%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN
%token UNKNOWN

%type <node> external_declaration function_definition primary_expression postfix_expression
%type <node> unary_expression cast_expression multiplicative_expression additive_expression shift_expression relational_expression
%type <node> equality_expression and_expression exclusive_or_expression inclusive_or_expression logical_and_expression logical_or_expression
%type <node> conditional_expression assignment_expression expression declarator direct_declarator statement compound_statement jump_statement
%type <node> parameter_declaration declaration
%type <node> expression_statement selection_statement iteration_statement
%type <node> labeled_statement
%type <node> enum_specifier enumerator_list enumerator


%type <node_list> statement_list parameter_list argument_expression_list translation_unit

%type <number_int> INT_CONSTANT pointer
%type <number_float> FLOAT_CONSTANT
%type <string> IDENTIFIER STRING_LITERAL TYPE_NAME
%type <type_specifier> type_specifier declaration_specifiers
%type <string_vec> struct_member_list


%start ROOT
%%

ROOT
	: translation_unit { g_root = $1; }

translation_unit
	: external_declaration { $$ = new NodeList(NodePtr($1)); }
  | translation_unit external_declaration {
      $1->PushBack(NodePtr($2));
      $$ = $1;
  }
	;

external_declaration
	: function_definition { $$ = $1; }
  | declaration { $$ = $1; }
  | STRUCT IDENTIFIER '{' struct_member_list '}' ';' {
      $$ = new StructDefinition(*$2, std::move(*$4));
      delete $2; delete $4;
  }
	;

struct_member_list
	: type_specifier IDENTIFIER ';' {
		$$ = new std::vector<std::string>(); $$->push_back(*$2); delete $2;
	}
	| struct_member_list type_specifier IDENTIFIER ';' {
		$1->push_back(*$3); delete $3; $$ = $1;
	}
	;

function_definition
	: declaration_specifiers declarator compound_statement {
		$$ = new FunctionDefinition($1, NodePtr($2), NodePtr($3));
	}
	| declaration_specifiers pointer declarator compound_statement {
		$$ = new FunctionDefinition($1, NodePtr($3), NodePtr($4));
	}
	;

declaration_specifiers
	: type_specifier { $$ = $1; }
	;

type_specifier
	: INT { $$ = TypeSpecifier::INT; global_typedef_pointer_depth = 0; }
	| FLOAT { $$ = TypeSpecifier::FLOAT; global_typedef_pointer_depth = 0; }
	| DOUBLE { $$ = TypeSpecifier::DOUBLE; global_typedef_pointer_depth = 0; }
	| CHAR { $$ = TypeSpecifier::CHAR; global_typedef_pointer_depth = 0; }
	| VOID { $$ = TypeSpecifier::VOID; global_typedef_pointer_depth = 0; }
	| UNSIGNED INT { $$ = TypeSpecifier::UINT; global_typedef_pointer_depth = 0; }
	| UNSIGNED CHAR { $$ = TypeSpecifier::UCHAR; global_typedef_pointer_depth = 0; }
	| UNSIGNED { $$ = TypeSpecifier::UINT; }
	| TYPE_NAME {
		auto& qt = global_type_aliases[*$1];
        $$ = qt.base;
        global_typedef_pointer_depth = qt.pointer_depth;
        delete $1;
	}
	| enum_specifier { $$ = TypeSpecifier::INT; global_typedef_pointer_depth = 0; }
	;

enum_specifier
	: ENUM '{' enumerator_list '}' { $$ = nullptr; }
	| ENUM IDENTIFIER '{' enumerator_list '}' { $$ = nullptr; delete $2; }
	| ENUM IDENTIFIER { $$ = nullptr; delete $2; }
	;

enumerator_list
	: enumerator { $$ = nullptr; }
	| enumerator_list ',' enumerator { $$ = nullptr; }
	;

enumerator
	: IDENTIFIER {
		global_enum_names[*$1] = global_enum_next_value++;
        $$ = nullptr;
        delete $1;
	}
	| IDENTIFIER '=' INT_CONSTANT {
		global_enum_names[*$1] = $3;
		global_enum_next_value = $3 + 1;
		$$ = nullptr;
		delete $1;
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
		$$ = new DirectDeclarator(NodePtr($1), NodePtr($3));
	}
	;

parameter_list
	: parameter_declaration { $$ = new NodeList(NodePtr($1)); }
	| parameter_list ',' parameter_declaration { $1->PushBack(NodePtr($3)); $$ = $1; }
	;

//reminder eg: "int x" - $1=type, $2=name, return name node

pointer
	: '*'         { $$ = 1; }
	| '*' pointer { $$ = $2 + 1; }
	;

parameter_declaration
	: declaration_specifiers declarator {
		Identifier* id = dynamic_cast<Identifier*>($2);
		$$ = new Declaration($1, id->GetName(), global_typedef_pointer_depth);
		delete $2;
	}
	| declaration_specifiers pointer IDENTIFIER {
		$$ = new Declaration($1, *$3, $2 + global_typedef_pointer_depth);
		delete $3;
	}
	;

declaration
	: declaration_specifiers ';' { $$ = nullptr; }
	| TYPEDEF declaration_specifiers declarator ';' {
		if (Identifier* id = dynamic_cast<Identifier*>($3)) {
		std::string name = id->GetName();
		global_typedef_names.insert(name);
		global_type_aliases[name] = QualifiedType{$2, global_typedef_pointer_depth};
		}

		$$ = nullptr;
		delete $3;

	}
	| TYPEDEF declaration_specifiers '*' IDENTIFIER ';' {
        std::string name = *$4;
        global_typedef_names.insert(name);
        global_type_aliases[name] = QualifiedType{$2, 1 + global_typedef_pointer_depth};
        $$ = nullptr;
        delete $4;
    }
	| declaration_specifiers declarator ';' {
    // Fixing bug where it takes functon parameters as declarations (sees int functoin () and groups brackerts as declaration)
	if (Identifier* id = dynamic_cast<Identifier*>($2)){
      $$ = new Declaration($1, id->GetName(), global_typedef_pointer_depth);
    }else{
      $$ = nullptr; // Don't make declaraton if function parameters
    }
		delete $2;
	}
	| declaration_specifiers declarator '=' assignment_expression ';' {
		// int x = expr;
		Identifier* id = dynamic_cast<Identifier*>($2);
		$$ = new InitDeclaration($1, id->GetName(), global_typedef_pointer_depth, NodePtr($4));
		delete $2;
	}
	| declaration_specifiers pointer IDENTIFIER ';' {
		$$ = new Declaration($1, *$3, $2);
		delete $3;
	}
	| declaration_specifiers pointer IDENTIFIER '=' assignment_expression ';' {
		$$ = new InitDeclaration($1, *$3, $2, NodePtr($5));
		delete $3;
	}
	| declaration_specifiers IDENTIFIER '[' INT_CONSTANT ']' ';' {
		$$ = new ArrayDeclaration($1, *$2, $4);
		delete $2;
	}
	| STRUCT IDENTIFIER IDENTIFIER ';' {
		// struct x z;
		$$ = new StructDeclaration(*$2, *$3);
		delete $2; delete $3;
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
	| SWITCH '(' expression ')' statement {
		$$ = new SwitchStatement(NodePtr($3), NodePtr($5));
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
	| labeled_statement { $$ = $1; }
	;

labeled_statement
	: CASE expression ':' statement {
		$$ = new CaseStatement(NodePtr($2), NodePtr($4));
	}
	| DEFAULT ':' statement {
		$$ = new DefaultStatement(NodePtr($3));
	}
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
	| BREAK ';' {
		$$ = new BreakStatement();
	}
	| CONTINUE ';' {
		$$ = new ContinueStatement();
	}
	;

primary_expression
	: INT_CONSTANT {
		$$ = new IntConstant($1);
	}
	| FLOAT_CONSTANT { $$ = new FloatConstant($1); }
	| IDENTIFIER {
		$$ = new Identifier(std::move(*$1));
		delete $1;
	}
	| STRING_LITERAL {
		$$ = new StringLiteral(std::move(*$1));
		delete $1;
	}
	;


postfix_expression
	: primary_expression { $$ = $1; }
	| postfix_expression '[' expression ']' {
		// x[i] rvalue; extract name from Identifier, build array subscript
		Identifier* id = dynamic_cast<Identifier*>($1);
		$$ = new ArraySubscript(id->GetName(), NodePtr($3));
		delete $1;
	}
	| postfix_expression '(' ')' {
		// function call with no arguments
		Identifier* id = dynamic_cast<Identifier*>($1);
		$$ = new FunctionCall(id->GetName(), nullptr);
		delete $1;
	}
	| postfix_expression '(' argument_expression_list ')' {
		// function call with arguments
		Identifier* id = dynamic_cast<Identifier*>($1);
		$$ = new FunctionCall(id->GetName(), NodePtr($3));
		delete $1;
	}
	| postfix_expression INC_OP { $$ = new PostfixOp("++", NodePtr($1)); }
	| postfix_expression DEC_OP { $$ = new PostfixOp("--", NodePtr($1)); }
	| postfix_expression '.' IDENTIFIER {
		Identifier* id = dynamic_cast<Identifier*>($1);
		$$ = new StructMemberGet(id->GetName(), *$3);
		delete $1; delete $3;
	}
	;

argument_expression_list
	: assignment_expression { $$ = new NodeList(NodePtr($1)); }
	| argument_expression_list ',' assignment_expression {
		$1->PushBack(NodePtr($3));
		$$ = $1;
	}
	;

unary_expression
	: postfix_expression
	| '-' cast_expression { $$ = new UnaryOp("-", NodePtr($2)); }
	| '~' cast_expression { $$ = new UnaryOp("~", NodePtr($2)); }
	| '!' cast_expression { $$ = new UnaryOp("!", NodePtr($2)); }
	| '&' cast_expression {
		// &x ;  Identifier lvalue
		Identifier* id = dynamic_cast<Identifier*>($2);
		$$ = new AddressOf(id->GetName());
		delete $2;
	}
	| '*' cast_expression {
		$$ = new Deref(NodePtr($2));
	}
	| SIZEOF '(' IDENTIFIER ')' {
		$$ = new SizeOf(*$3);
		delete $3;
	}
	| SIZEOF '(' type_specifier ')' {
		$$ = new SizeOfType($3);
	}
	| SIZEOF '(' type_specifier pointer ')' {
		(void)$3; (void)$4;
		$$ = new SizeOfPointer();
	}
	;

cast_expression
	: unary_expression
	;

multiplicative_expression
	: cast_expression
	| multiplicative_expression '*' cast_expression { $$ = new BinaryOp(NodePtr($1), "*", NodePtr($3)); }
	| multiplicative_expression '/' cast_expression { $$ = new BinaryOp(NodePtr($1), "/", NodePtr($3)); }
	| multiplicative_expression '%' cast_expression { $$ = new BinaryOp(NodePtr($1), "%", NodePtr($3)); }
	;

additive_expression
	: multiplicative_expression
	| additive_expression '+' multiplicative_expression { $$ = new BinaryOp(NodePtr($1), "+", NodePtr($3)); }
	| additive_expression '-' multiplicative_expression { $$ = new BinaryOp(NodePtr($1), "-", NodePtr($3)); }
	;

shift_expression
	: additive_expression
	| shift_expression LEFT_OP additive_expression { $$ = new BinaryOp(NodePtr($1), "<<", NodePtr($3)); }
	| shift_expression RIGHT_OP additive_expression { $$ = new BinaryOp(NodePtr($1), ">>", NodePtr($3)); }
	;

relational_expression
	: shift_expression
	| relational_expression '<' shift_expression { $$ = new BinaryOp(NodePtr($1), "<", NodePtr($3)); }
	| relational_expression '>' shift_expression { $$ = new BinaryOp(NodePtr($1), ">", NodePtr($3)); }
	| relational_expression LE_OP shift_expression { $$ = new BinaryOp(NodePtr($1), "<=", NodePtr($3)); }
	| relational_expression GE_OP shift_expression { $$ = new BinaryOp(NodePtr($1), ">=", NodePtr($3)); }
	;

equality_expression
	: relational_expression
	| equality_expression EQ_OP relational_expression { $$ = new BinaryOp(NodePtr($1), "==", NodePtr($3)); }
	| equality_expression NE_OP relational_expression { $$ = new BinaryOp(NodePtr($1), "!=", NodePtr($3)); }
	;

and_expression
	: equality_expression
	| and_expression '&' equality_expression { $$ = new BinaryOp(NodePtr($1), "&", NodePtr($3)); }
	;

exclusive_or_expression
	: and_expression
	| exclusive_or_expression '^' and_expression { $$ = new BinaryOp(NodePtr($1), "^", NodePtr($3)); }
	;

inclusive_or_expression
	: exclusive_or_expression
	| inclusive_or_expression '|' exclusive_or_expression { $$ = new BinaryOp(NodePtr($1), "|", NodePtr($3)); }
	;

logical_and_expression
	: inclusive_or_expression
	| logical_and_expression AND_OP inclusive_or_expression { $$ = new BinaryOp(NodePtr($1), "&&", NodePtr($3)); }
	;

logical_or_expression
	: logical_and_expression
	| logical_or_expression OR_OP logical_and_expression { $$ = new BinaryOp(NodePtr($1), "||", NodePtr($3)); }
	;

conditional_expression
	: logical_or_expression
	;

assignment_expression
	: conditional_expression
	| unary_expression '=' assignment_expression {
		// extract name from lhs Identifier, build Assignment node owning rhs
		if (Deref* d = dynamic_cast<Deref*>($1)) {
			// *p = rhs, store through ptr
			$$ = new DerefAssign(d->TakeOperand(), NodePtr($3));
			delete $1;
		} else {
			Identifier* id = dynamic_cast<Identifier*>($1);
			$$ = new Assignment(id->GetName(), NodePtr($3));
			delete $1;
		}
	}
	| postfix_expression '[' expression ']' '=' assignment_expression {
		Identifier* id = dynamic_cast<Identifier*>($1);
		$$ = new ArrayAssignment(id->GetName(), NodePtr($3), NodePtr($6));
		delete $1;
	}
	| postfix_expression '.' IDENTIFIER '=' assignment_expression {
		// z.y = rhs
		Identifier* id = dynamic_cast<Identifier*>($1);
		$$ = new StructMemberSet(id->GetName(), *$3, NodePtr($5));
		delete $1; delete $3;
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
