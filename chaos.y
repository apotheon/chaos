%{

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "utilities/platform.h"
#include "utilities/language.h"
#include "utilities/helpers.h"
#include "symbol.h"

extern int yylex();
extern int yyparse();
extern FILE* yyin;

void yyerror(const char* s);

bool is_interactive = true;
%}

%union {
	bool bval;
	int ival;
	float fval;
	char *sval;
}

%token<bval> T_TRUE T_FALSE
%token<ival> T_INT
%token<fval> T_FLOAT
%token<sval> T_STRING T_F_STRING T_VAR
%token T_PLUS T_MINUS T_MULTIPLY T_DIVIDE T_LEFT T_RIGHT T_EQUAL
%token T_NEWLINE T_QUIT
%token T_PRINT
%token T_VAR_BOOL
%left T_PLUS T_MINUS
%left T_MULTIPLY T_DIVIDE

%type<ival> expression
%type<fval> mixed_expression
%type<sval> variable

%start parser

%%

parser:
	| parser line					{ is_interactive ? printf("\n%s ", __SHELL_INDICATOR__) : printf("\n"); }
;

line: T_NEWLINE
    | mixed_expression T_NEWLINE		{ printf("%f", $1); }
    | expression T_NEWLINE				{ printf("%i", $1); }
	| variable T_NEWLINE				{ }
    | T_QUIT T_NEWLINE					{ printf("bye!\n"); exit(0); }
	| T_PRINT T_INT T_NEWLINE			{ printf("%i", $2); }
	| T_PRINT T_F_STRING T_NEWLINE		{ printf("%s", $2); }
	| T_PRINT T_STRING T_NEWLINE		{ printf("%s", $2); }
;

mixed_expression: T_FLOAT                 		 		{ $$ = $1; }
	| mixed_expression T_PLUS mixed_expression	 		{ $$ = $1 + $3; }
	| mixed_expression T_MINUS mixed_expression	 		{ $$ = $1 - $3; }
	| mixed_expression T_MULTIPLY mixed_expression		{ $$ = $1 * $3; }
	| mixed_expression T_DIVIDE mixed_expression	 	{ $$ = $1 / $3; }
	| T_LEFT mixed_expression T_RIGHT		 			{ $$ = $2; }
	| expression T_PLUS mixed_expression	 	 		{ $$ = $1 + $3; }
	| expression T_MINUS mixed_expression	 	 		{ $$ = $1 - $3; }
	| expression T_MULTIPLY mixed_expression 	 		{ $$ = $1 * $3; }
	| expression T_DIVIDE mixed_expression	 			{ $$ = $1 / $3; }
	| mixed_expression T_PLUS expression	 	 		{ $$ = $1 + $3; }
	| mixed_expression T_MINUS expression	 	 		{ $$ = $1 - $3; }
	| mixed_expression T_MULTIPLY expression 	 		{ $$ = $1 * $3; }
	| mixed_expression T_DIVIDE expression	 			{ $$ = $1 / $3; }
	| expression T_DIVIDE expression		 			{ $$ = $1 / (float)$3; }
;

expression: T_INT						{ $$ = $1; }
	| expression T_PLUS expression		{ $$ = $1 + $3; }
	| expression T_MINUS expression		{ $$ = $1 - $3; }
	| expression T_MULTIPLY expression	{ $$ = $1 * $3; }
	| T_LEFT expression T_RIGHT			{ $$ = $2; }
;

variable: T_VAR								{ printSymbolValue(getSymbol($1)); }
;

variable: T_VAR_BOOL						{ }
	| variable T_VAR T_EQUAL T_TRUE			{ addSymbol($2, BOOL, $4); printf("%s", $4 ? "true" : "false"); }
	| variable T_VAR T_EQUAL T_FALSE		{ addSymbol($2, BOOL, $4); printf("%s", $4 ? "true" : "false"); }
;

%%

int main(int argc, char** argv) {
	printf("%s Language %s (%s %s)\n", __LANGUAGE_NAME__, __LANGUAGE_VERSION__, __DATE__, __TIME__);
	printf("GCC version: %d.%d.%d on %s\n",__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__, __PLATFORM_NAME__);

	FILE *fp = argc > 1 ? fopen (argv[1], "r") : stdin;

	is_interactive = (fp != stdin) ? false : true;

	yyin = fp;

	do {
		!is_interactive ?: printf("%s ", __SHELL_INDICATOR__);
		yyparse();
	} while(!feof(yyin));

	return 0;
}

void yyerror(const char* s) {
	fprintf(stderr, "Parse error: %s\n", s);
	exit(1);
}
