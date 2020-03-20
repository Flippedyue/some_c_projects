%{
#include <stdio.h>
#include <stdlib.h>
#include "codes.h"
void yyerror(const char* msg) {}
int yylex();
%}
%union{
  TreeNode* node;
  int op;
};


%token <node> NUMBER ID STRING
%token <node> INT DOUBLE CHAR BOOLEAN VOID 
%token <op> PLUS MINUS TIMES OVER MOD PPLUS MMINUS AND OR XOR NOT SHIFT_LEFT SHIFT_RIGHT ASSIGN EQU 
%token <op> GTR LSS GEQ LEQ NEQ LOGICAL_AND LOGICAL_OR LOGICAL_NOT UMINUS 
%token <op> SEMICOLON LB RB COMMA LP RP
%token <op> FOR INPUT OUTPUT ELSE MAIN IF WHILE
%token <node> EOL
%type <node> show_stmt expr stmt decl idlist for_stmt com_stmt output_stmt input_stmt while_stmt stmts if_stmt

// 

%right ASSIGN
%left LOGICAL_OR
%left LOGICAL_AND
%left OR
%left XOR
%left AND
%left EQU NEQ
%left GTR LSS GEQ LEQ
%left SHIFT_LEFT SHIFT_RIGHT
%left PLUS MINUS
%left TIMES OVER MOD
%right NOT LOGICAL_NOT
%right UMINUS
%left  PPLUS MMINUS // 后缀自增、自减
%%
// IfK, WhileK, AssignK, ForK, CompK, InputK, PrintK

show_stmt : stmt           { $$ = $1; gen_code($$); }  //Display($$);} // gen_code

stmt    :  MAIN LP RP com_stmt { $$ = $4; }
		|  com_stmt        { $$ = $1; }
		|  output_stmt     { $$ = $1; }
		|  input_stmt      { $$ = $1; }
		|  while_stmt      { $$ = $1; }
		|  if_stmt         { $$ = $1; }
		|  for_stmt        { $$ = $1; }
		|  expr  SEMICOLON { $$ = $1; }
		|  decl  SEMICOLON { $$ = $1; }
		|  SEMICOLON       { $$ = NULL; }
		;

com_stmt : LB stmts RB {$$ = newTreeNode(StmtK, CompK, $2, NULL);  }
		 ;


stmts   :  stmt stmts 
			{ if($$ == NULL) {
					$$ = $2;
				}
				else {
					$1->sibling = $2; $$ = $1; 
				}
			}
        |  stmt { $$ = $1;}
		;


output_stmt  :  OUTPUT LP expr RP SEMICOLON { $$ = newTreeNode(StmtK, PrintK, $3, NULL); }
			 ;

input_stmt :  INPUT LP expr RP SEMICOLON { $$ = newTreeNode(StmtK, InputK, $3, NULL); }
		   ;

while_stmt :  WHILE LP expr RP stmt { $$ = newTreeNode(StmtK, WhileK, $3, $5, NULL); }
		   ;

for_stmt   :  FOR LP expr SEMICOLON expr SEMICOLON expr RP stmt { $$ = newTreeNode(StmtK, ForK, $3, $5, $7, $9);}
		   |  FOR LP SEMICOLON expr SEMICOLON expr RP stmt      { $$ = newTreeNode(StmtK, ForK, NULL, $4, $6, $8); }
		   |  FOR LP expr SEMICOLON SEMICOLON expr RP stmt      { $$ = newTreeNode(StmtK, ForK, $3, NULL, $6, $8); }
		   |  FOR LP expr SEMICOLON expr SEMICOLON RP stmt      { $$ = newTreeNode(StmtK, ForK, $3, $5, NULL, $8); }
		   |  FOR LP SEMICOLON SEMICOLON expr RP stmt           { $$ = newTreeNode(StmtK, ForK, NULL, NULL, $5, $7); }
		   |  FOR LP SEMICOLON expr SEMICOLON RP stmt           { $$ = newTreeNode(StmtK, ForK, NULL, $4, NULL, $7); }
		   |  FOR LP expr SEMICOLON SEMICOLON RP stmt           { $$ = newTreeNode(StmtK, ForK, $3, NULL, NULL, $7); }
		   |  FOR LP SEMICOLON SEMICOLON RP stmt                { $$ = newTreeNode(StmtK, ForK, NULL, NULL, NULL, $6); }
		   ;

if_stmt :   IF LP expr RP stmt               { $$ = newTreeNode(StmtK, IfK, $3, $5, NULL); }
		|   IF LP expr RP stmt ELSE stmt     { $$ = newTreeNode(StmtK, IfK, $3, $5, $7, NULL); }
		;

expr	:	expr PLUS expr	{ $$ = newExpNode($2, $1, $3); }
		|	expr MINUS expr	{ $$ = newExpNode($2, $1, $3); }
		|	expr TIMES expr	{ $$ = newExpNode($2, $1, $3); }
		|	expr OVER expr	{ $$ = newExpNode($2, $1, $3); }
        | 	expr MOD expr	{ $$ = newExpNode($2, $1, $3); }
        |   expr MMINUS     { $$ = newExpNode($2, $1, NULL); }
        |   expr PPLUS      { $$ = newExpNode($2, $1, NULL); }
        |   expr AND expr   { $$ = newExpNode($2, $1, $3); } 
        |   expr OR expr    { $$ = newExpNode($2, $1, $3); }
        |   NOT expr        { $$ = newExpNode($1, $2, NULL); }
        |   expr XOR expr   { $$ = newExpNode($2, $1, $3); }
        |   expr SHIFT_LEFT expr  { $$ = newExpNode($2, $1, $3); }
        |   expr SHIFT_RIGHT expr { $$ = newExpNode($2, $1, $3); }
        |   expr ASSIGN expr   { $$ = newExpNode($2, $1, $3); }
        |   expr EQU expr   { $$ = newExpNode($2, $1, $3); }
        |   expr GTR expr   { $$ = newExpNode($2, $1, $3); }
        |   expr LSS expr   { $$ = newExpNode($2, $1, $3); }
        |   expr GEQ expr   { $$ = newExpNode($2, $1, $3); }
        |   expr LEQ expr   { $$ = newExpNode($2, $1, $3); }
        |   expr NEQ expr   { $$ = newExpNode($2, $1, $3); }
        |   expr LOGICAL_AND expr  { $$ = newExpNode($2, $1, $3); }
        |   expr LOGICAL_OR expr   { $$ = newExpNode($2, $1, $3); }
        |   LOGICAL_NOT expr   { $$ = newExpNode($1, $2, NULL); }
        |   MINUS expr %prec UMINUS   { $$ = newExpNode($1, $2, NULL); }
		|	LP expr RP	    { $$ = $2; }
		|	NUMBER          { $$ = $1; }   // $$=$1 can be ignored
        |   STRING          { $$ = $1; }
		|   ID              { $$ = $1; }
		;

idlist  :   ID COMMA idlist { $1->sibling = $3; $$ = $1; }
		|   ID { $$ = $1; }
		;

decl    :   INT idlist       { $$ = newDeclNode($1, $2); }
		|   CHAR idlist      { $$ = newDeclNode($1, $2); }
		|   DOUBLE idlist    { $$ = newDeclNode($1, $2); }
		|   BOOLEAN idlist   { $$ = newDeclNode($1, $2); }
		|   VOID idlist      { $$ = newDeclNode($1, $2); }
		;

%%

int yylex();

int main() {
	ConstructMap();
    return yyparse();
}
