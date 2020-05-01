	/*
		Compilers
		Homework 7. This is the last assignment for the Compilers Analysis and
		Design course. This file is the source for the scanner that will be used
		along the parser generated with Bison.
		
		@author	Luis Francisco Flores Romero, A01328937
		@author Gaspard Goupy, A01755974
		@since	8.dec.2019
	*/
	
	/* Imports */
%{
	#include <string.h>
	#include "parser.h"
	
	#define TAB_WIDTH 4
	
	void reset_char_pos();
	void update_char_pos();
	
	extern int debug_enabled;
%}
	
	/* LEX VARIABLES */
	int lineCount = 1;
	int char_pos = 1;
	int debug_enabled = 0;
	char *last_id = NULL;
	
	/* DEFINITIONS */
DIGIT			[0-9]
ID_SYM_NNUM		[a-zA-Z_$]
ID_SYM			{ID_SYM_NNUM}|{DIGIT}
IDENTIFIER		{ID_SYM_NNUM}{ID_SYM}*
	/* The definition below has been disabled as sign is using tilde character */
	/* INT				(-?[1-9]{DIGIT}*)|0 */
INT				([1-9]{DIGIT}*)|0
FLOAT			({INT}\.{DIGIT}+)|({INT}f)

%%

	/* RULES */
	/* Reserved words */
"program" { update_char_pos(); return PROGRAM; }
"begin" { update_char_pos(); return BEGIN_PROG; }
"end" { update_char_pos(); return END; }
"fun" { update_char_pos(); return FUN; }
"var" { update_char_pos(); return VAR; }
"ifelse" { update_char_pos(); return IFELSE; }
"if" { update_char_pos(); return IF; }
"then" { update_char_pos(); return THEN; }
"else" { update_char_pos(); return ELSE; }
"while" { update_char_pos(); return WHILE; }
"for" { update_char_pos(); return FOR; }
"until" { update_char_pos(); return UNTIL; }
"return" { update_char_pos(); return RETURN; }
"read" { update_char_pos(); return READ; }
"print" { update_char_pos(); return PRINT; }
"int" { update_char_pos(); return TYPE_INT; }
"float" { update_char_pos(); return TYPE_FLOAT; }

	/* Symbols */
":=" { update_char_pos(); return ASSIGN; }
"<-" { update_char_pos(); return ASSIGN; }
"=" { update_char_pos(); return EQUALS; }
"<" { update_char_pos(); return LT; }
"<=" { update_char_pos(); return LET; }
">" { update_char_pos(); return GT; }
">=" { update_char_pos(); return GET; }
"," { update_char_pos(); return COMA; }
":" { update_char_pos(); return COLON; }
";" { update_char_pos(); return SEMICOLON; }
"(" { update_char_pos(); return PARENTHESIS_OPEN; }
")" { update_char_pos(); return PARENTHESIS_CLOSE; }
"+" { update_char_pos(); return PLUS; }
"-" { update_char_pos(); return MINUS; }
"*" { update_char_pos(); return STAR; }
"/" { update_char_pos(); return SLASH_FORWARD; }
"~" { update_char_pos(); return TILDE; }

	/* Tokens */
{IDENTIFIER} { update_char_pos(); if(last_id) free(last_id); last_id = strdup(yytext); return ID; }
{FLOAT} { update_char_pos(); yylval.type_float = atof(yytext); return FLOAT; }
{INT} { update_char_pos(); yylval.type_int = atoi(yytext); return INT; }

	/* WILDCARDS */
[\n] { lineCount++; reset_char_pos(); }
[\r] // Ignore non-standard new lines
[ \t] { update_char_pos(); } // Ignore whitespace
. { update_char_pos(); return ERROR; }

%%

void update_char_pos() {
	char_pos += yyleng;
}

void reset_char_pos() {
	char_pos = 1;
}

char *type_to_string(const int type) {
	switch(type) {
		case TYPE_INT:
			return "int";
		case TYPE_FLOAT:
			return "float";
		case TYPE_VOID:
			return "void";
		default:
			return "?";
	}
}

char* token_to_string(const int token_id) {
	switch(token_id) {
		case PROGRAM:
			return "program";
		case BEGIN_PROG:
			return "begin";
		case END:
			return "end";
		case FUN:
			return "function";
		case CALL:
			return "call";
		case ARG:
			return "arg";
		case CONST:
			return "const";
		case VAR:
			return "var";
		case IFELSE:
			return "ifelse";
		case IF:
			return "if";
		case THEN:
			return "then";
		case ELSE:
			return "else";
		case WHILE:
			return "while";
		case FOR:
			return "for";
		case UNTIL:
			return "until";
		case RETURN:
			return "return";
		case TYPE_INT:
			return "int";
		case TYPE_FLOAT:
			return "float";
		case ASSIGN:
			return "assign";
		case EQUALS:
			return "=";
		case LT:
			return "<";
		case LET:
			return "<=";
		case GT:
			return ">";
		case GET:
			return ">=";
		case COLON:
			return ":";
		case SEMICOLON:
			return ";";
		case PARENTHESIS_OPEN:
			return "(";
		case PARENTHESIS_CLOSE:
			return ")"; 
		case PLUS:
			return "+";
		case MINUS:
			return "-";
		case STAR:
			return "*";
		case SLASH_FORWARD:
			return "/";
		case TILDE:
			return "~";
		case ID:
			return "<id>";
		case READ:
			return "read";
		case PRINT:
			return "print";
		case FLOAT:
			return "<float>";
		case INT:
			return "<int>";
		case ERROR:
			return "error";
		default:
			// Should not reach this
			if(debug_enabled)
				printf("Unexpected token ID: %d\n", token_id);
			return "<token>";
	}
}
