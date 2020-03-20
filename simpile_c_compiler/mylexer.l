%{
#include <iostream>
#include "TreeNode.h"
#include "myparser.tab.h"
// #define YYSTYPE TreeNode*
%}

%option yylineno
digit [0-9]
NUMBER {digit}+(\.{digit}+)?
TRUE true
FALSE false
COMMENT (\/\*(.|\n|(\"\*\/\"))*\*\/)|(\/\/.*)
INT int
CHAR char
DOUBLE double
VOID void
BOOLEAN bool
PLUS \+ 
MINUS \-
TIMES \*
OVER \/
MOD \%
PPLUS \+\+
MMINUS \-\-
AND &
OR \|
XOR \^
NOT ~
SHIFT_LEFT <<
SHIFT_RIGHT >>
ASSIGN =
IF if
ELSE else
WHILE while
FOR for
MAIN main
INPUT input 
OUTPUT output 
EQU ==
GTR \>
LSS \< 
GEQ \>= 
LEQ \<=
NEQ !=
LOGICAL_AND &&
LOGICAL_OR \|\|
LOGICAL_NOT !
LB \{    
RB \}
LP \(
RP \) 
SEMICOLON ;
COMMA ,
WS [\t\n ]+
ID [A-Za-z_][A-Za-z0-9_]*
STRING (\"[^\"]*\")|(\'[^\']\')
/* can support 12.34 */
/* CASE case  RETURN return BREAK break */


%%
{SEMICOLON} { return SEMICOLON; }
{COMMENT}   { /*do nothing*/ }
{PLUS} {
    yylval.op = optMap[strdup(yytext)];
    return PLUS;
}
{MINUS} {
    yylval.op = optMap[strdup(yytext)];
    return MINUS;
}
{TIMES} {
    yylval.op = optMap[strdup(yytext)];
    return TIMES;
}
{OVER} {
    yylval.op = optMap[strdup(yytext)];
    return OVER;
}
{PPLUS} {
    yylval.op = optMap[strdup(yytext)];
    return PPLUS;
}
{MMINUS} {
    yylval.op = optMap[strdup(yytext)];
    return MMINUS;
}
{AND} {
    yylval.op = optMap[strdup(yytext)];
    return AND;
}
{OR} {
    yylval.op = optMap[strdup(yytext)];
    return OR;
}
{ASSIGN} {
    yylval.op = optMap[strdup(yytext)];
    return ASSIGN;
}
{EQU} {
    yylval.op = optMap[strdup(yytext)];
    return EQU;
}
{GTR} {
    yylval.op = optMap[strdup(yytext)];
    return GTR;
}
{LSS} {
    yylval.op = optMap[strdup(yytext)];
    return LSS;
}
{GEQ} {
    yylval.op = optMap[strdup(yytext)];
    return GEQ;
}
{LEQ} {
    yylval.op = optMap[strdup(yytext)];
    return LEQ;
}
{NEQ} {
    yylval.op = optMap[strdup(yytext)];
    return NEQ;
}
{LOGICAL_AND} {
    yylval.op = optMap[strdup(yytext)];
    return LOGICAL_AND;
}
{LOGICAL_OR} {
    yylval.op = optMap[strdup(yytext)];
    return LOGICAL_OR;
} 
{LOGICAL_NOT} {
    yylval.op = optMap[strdup(yytext)];
    return LOGICAL_NOT;
}
{LB}        { return LB; }
{RB}        { return RB; }
{LP}        { return LP; }
{RP}        { return RP; }
{COMMA}     { return COMMA; }
{WS}        {/* do nothing */}
{INT} { 
    char s = Integer;
    char *str = &s;
    yylval.node = newVarNode(TypeK, str); 
    return INT; 
}
{CHAR} { 
    char s = Char;
    char *str = &s;
    yylval.node = newVarNode(TypeK, str); 
    return CHAR; 
}
{DOUBLE} { 
    char s = Double;
    char *str = &s;
    yylval.node = newVarNode(TypeK, str); 
    return DOUBLE; 
}
{VOID} { 
    char s = Void;
    char *str = &s;
    yylval.node = newVarNode(TypeK, str); 
    return VOID; 
}
{BOOLEAN} { 
    char s = Boolean;
    char *str = &s;
    yylval.node = newVarNode(TypeK, str); 
    return BOOLEAN; 
}
{FOR}       { return FOR; }
{MAIN}      { return MAIN; }
{IF}        { return IF; }
{ELSE}      { return ELSE; }
{WHILE}     { return WHILE; }
{INPUT}     { return INPUT; }
{OUTPUT}    { return OUTPUT; }
{STRING} {
    yylval.node = newVarNode(ConstK, strdup(yytext));
    return STRING;
}

{NUMBER} {
    yylval.node = newVarNode(ConstK, strdup(yytext));
    return NUMBER;
}
{ID} { 
    yylval.node = newVarNode(IdK, strdup(yytext));
    return ID;
}
%%

int yywrap(void) { 
    return 1;
}