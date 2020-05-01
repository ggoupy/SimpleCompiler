/*
	Compilers
	Homework 7. This is the last assignment for the Compilers Analysis and
	Design course. This file is the source for the Bison parser that will take
	user's program. AST is generated using nodes. This implementation supports
	functions (including recursive and embedded calls).
	
	@author	Luis Francisco Flores Romero, A01328937
	@author Gaspard Goupy, A01755974
	@since	8.dec.2019
*/

/* C declarations */
%{
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <assert.h>
	#include "table/table.c"
	#include "ast/ast.h"
	#include "ast/ast.c"
	
	int yylex();
	int yyerror(const char *);
	char *type_to_string(const int);
	int exit_error(char const * msg);
	
	extern int lineCount;
	extern int char_pos;
	extern int debug_enabled;
	extern char *last_id;
	
	extern char *yytext;
	extern int yyleng;
	extern FILE *yyin;
%}

/* Bison declarations */
%token
	PROGRAM
	BEGIN_PROG
	END
	FUN
	CALL
	ARG
	VAR
	CONST
	IFELSE
	IF
	THEN
	ELSE
	WHILE
	FOR
	UNTIL
	RETURN
	TYPE_VOID
	TYPE_INT
	TYPE_FLOAT
	ASSIGN
	EQUALS
	LT
	LET
	GT
	GET
	COMA
	COLON
	SEMICOLON
	PARENTHESIS_OPEN
	PARENTHESIS_CLOSE
	PLUS
	MINUS
	STAR
	SLASH_FORWARD
	TILDE
	ID
	READ
	PRINT
	FLOAT
	INT
	ERROR
	
%type<type_int> type relop INT
%type<type_float> FLOAT
%type<type_ptr> id expr expression term factor stmt stmt_lst opt_stmts arg arg_lst opt_args fun_id call_id

%union {
	int type_int;
	float type_float;
	void *type_ptr;
}

%start prog

%%

/* Grammar rules */
prog :	opt_decls opt_fun_decls { scope = NULL; } BEGIN_PROG opt_stmts END {
			tree = $5;
			
			if(debug_enabled)
				printf("Parsing succeeded!\n");
			
			return 0;
		}
;

opt_fun_decls : fun_decls
              | /* empty */
;

fun_decls : fun_dec SEMICOLON fun_decls
          | fun_dec
;

fun_dec	: FUN fun_id PARENTHESIS_OPEN opt_params PARENTHESIS_CLOSE COLON type {
			fun_symbol_t *fun = lookup_function($2);
			fun->type = $7; // update function type
			fun->symbols_template[0].type = $7; // update return type in symbols
		} opt_decls BEGIN_PROG opt_stmts END {
			fun_symbol_t *fun = lookup_function($2);
			
			if(debug_enabled)
				printf("Saving tree for %s: %p\n", $2, $11);
			
			fun->tree = $11; // update function tree
		}
;

opt_params : param_lst
           | /* empty */
;

param_lst : param COMA param_lst
          | param
;

param : ID COLON type {
	assert(scope != NULL); // we must be inside a function declaration
	
	check_declaration_scoped(last_id);
	symbol_t item = new_symbol(last_id, $3);
	add_scoped_symbol(item);
	
	scope->param_count++; // update param count
};

opt_decls : decls
          | /* empty */
;

decls : dec SEMICOLON decls
      | dec
;

dec : VAR ID COLON type {
	check_declaration_scoped(last_id);
	symbol_t item = new_symbol(last_id, $<type_int>4);
	add_scoped_symbol(item);
};

type : TYPE_INT { $$ = TYPE_INT; }
     | TYPE_FLOAT { $$ = TYPE_FLOAT; }
;

/* Statements create semicolon nodes */
stmt :
	id ASSIGN expr {
		check_use_scoped((char *)(((node_t *)$1)->a));
		$$ = create_node(SEMICOLON, create_node(ASSIGN, $1, $3, NULL, 0), NULL, NULL, 0);
	}
	| IF PARENTHESIS_OPEN expression PARENTHESIS_CLOSE stmt {
		$$ = create_node(SEMICOLON, create_node(IF, $3, $5, NULL, 0), NULL, NULL, 0);
	}
	| IFELSE PARENTHESIS_OPEN expression PARENTHESIS_CLOSE stmt stmt {
		$$ = create_node(SEMICOLON, create_node(IFELSE, $3, $5, $6, 0), NULL, NULL, 0);
	}
	| WHILE PARENTHESIS_OPEN expression PARENTHESIS_CLOSE stmt {
		$$ = create_node(SEMICOLON, create_node(WHILE, $3, $5, NULL, 0), NULL, NULL, 0);
	}
	| READ id {
		check_use_scoped(last_id);
		$$ = create_node(SEMICOLON, create_node(READ, $2, NULL, NULL, 0), NULL, NULL, 0);
	}
	| PRINT expr { $$ = create_node(SEMICOLON, create_node(PRINT, $2, NULL, NULL, 0), NULL, NULL, 0); }
	| BEGIN_PROG opt_stmts END { $$ = $2; }
	| RETURN expr { $$ = create_node(SEMICOLON, create_node(RETURN, $2, NULL, NULL, 0), NULL, NULL, 0); }
;

opt_stmts : stmt_lst { $$ = $1; }
          | /* empty */ { $$ = NULL; }
;

stmt_lst : stmt SEMICOLON stmt_lst { append_node($1, $3); $$ = $1; }
         | stmt { $$ = $1; }
;

expression : expr { $$ = $1; }
           | expr relop expr { $$ = create_node($2, $1, $3, NULL, 0); }
;

expr : expr PLUS term { $$ = create_node(PLUS, $1, $3, NULL, 0); }
     | expr MINUS term { $$ = create_node(MINUS, $1, $3, NULL, 0); }
     | sign term { $$ = create_node(TILDE, $2, NULL, NULL, 0); }
     | term { $$ = $1; }
;

term : term STAR factor { $$ = create_node(STAR, $1, $3, NULL, 0); }
     | term SLASH_FORWARD factor { $$ = create_node(SLASH_FORWARD, $1, $3, NULL, 0); }
     | factor { $$ = $1; }
;

factor	: PARENTHESIS_OPEN expr PARENTHESIS_CLOSE {
			$$ = $2;
		}
		| id {
			check_use_scoped(last_id);
			$$ = $1;
		}
		| INT {
			value_t *val = (value_t *)malloc(sizeof(value_t));
			val->int_val = $1;
			$$ = create_node(CONST, (node_t *)val, NULL, NULL, TYPE_INT);
		}
		| FLOAT {
			value_t *val = (value_t *)malloc(sizeof(value_t));
			val->float_val = $1;
			$$ = create_node(CONST, (node_t *)val, NULL, NULL, TYPE_FLOAT);
		}
		| call_id PARENTHESIS_OPEN opt_args PARENTHESIS_CLOSE {
			fun_symbol_t *function_symbol = lookup_function((char *)$1);
			assert(function_symbol != NULL);
			
			// Match arguments count
			size_t req_args = function_symbol->param_count;
			size_t act_args = 0;
			node_t *curr_arg = $3;
			while(curr_arg != NULL) {
				act_args++;
				curr_arg = curr_arg->b;
			}
			
			if(req_args != act_args) {
				char *err_msg;
				if(asprintf(&err_msg, "wrong number of arguments in function call. Expected %zu, got %zu", req_args, act_args) < 0) {
					printf("Failed while writing error msg\n");
					exit(0);
				}
				
				exit(yyerror(err_msg));
			}
			
			$$ = create_node(CALL, (node_t *)function_symbol, $3, NULL, 0);
		}
;

opt_args : arg_lst { $$ = $1; }
         | /* empty */ { $$ = NULL; }
;

arg_lst : arg COMA arg_lst { append_node($1, $3); $$ = $1; }
        | arg { $$ = $1; }
;

arg : expr {
	$$ = create_node(ARG, $1, NULL, NULL, 0);
};

relop : LT { $$ = LT; }
      | GT { $$ = GT; }
      | EQUALS { $$ = EQUALS; }
      | LET { $$ = LET; }
      | GET { $$ = GET; }
;

sign : TILDE
;

id : ID { $$ = create_node(VAR, (node_t *)last_id, NULL, NULL, 0); }
;

fun_id	: ID {
			// Create function and add to table
			fun_symbol_t item = new_function_symbol(last_id);
			add_function(item);
			scope = &functions[functions_next - 1]; // update scope
			
			$$ = (void *)strdup(last_id);
		};

call_id : ID { $$ = (void *)strdup(last_id); }
;

%%

/* C code */
int yyerror(const char *error) {
	fprintf(stderr, "Error at line %d, col %d: %s. ", lineCount, char_pos - yyleng, error);
	fprintf(stderr, "While reading \"%s\"", yytext);
	fprintf(stderr, ".\n");
	
	return ERROR;
}

void yywarning(const char *message) {
	fprintf(stderr, "Warning: %s. At line %d, col %d.\n", message, lineCount, char_pos - yyleng);
}

/* Print the message in parameter and exit */
int exit_error(char const * msg) {
	if(strlen(msg) > 0) fprintf(stderr, "%s\n",msg);
	fprintf(stderr, "### Exit Error ###\n");
	exit(1);
}

int main(int argc, char* argv[]) {
	
	if (argc <= 1) exit_error("Invalid number of arguments.\nUsage: <cmd> <file> [--debug]");
	
	if (argc > 1) {
		// Try to open file
		yyin = fopen(argv[1], "r"); // Open file and assign input stream

		if(yyin == NULL) {
			//printf("Failed to open file \"%s\"\n", argv[1]);
			exit_error(strcat(argv[1], ": Failed to open the file"));
		}
	}
	
	// Check for debug flag
	if(argc > 2) {
		if(strcmp(argv[2], "--debug") == 0) {
			printf("***DEBUG MODE ENABLED***\n");
			debug_enabled = 1;
		}
		else {
			exit_error("Unexpected parameter(s) on command call.\nUsage: <cmd> <filename> [--debug]");
		}
	}
	
	yyparse();
	execute(tree); // AST Function running the program

	return 0; 
}
