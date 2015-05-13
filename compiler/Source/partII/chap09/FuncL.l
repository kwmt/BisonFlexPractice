/* FuncLの字句の定義（FuncL.l） */

%{
#include "SymTable.h"
#include "y.tab.h"
#include "../VSM.h"

char *IDentry(char *, int);
%}

ws         [ \t\n]
sym        [=(),;{}]
digit      [0-9]
letter     [a-zA-Z_]
id         {letter}({letter}|{digit})*
other      .

%%

{ws}+      ;
{sym}      { return yytext[0]; }
"+"        { yylval.Int = ADD; return ADDOP; }
"-"        { yylval.Int = SUB; return ADDOP; }
"*"        { yylval.Int = MUL; return MULOP; }
"/"        { yylval.Int = DIV; return MULOP; }
"%"        { yylval.Int = MOD; return MULOP; }
"<"        { yylval.Int = BLT; return RELOP; }
"<="       { yylval.Int = BLE; return RELOP; }
"=="       { yylval.Int = BEQ; return RELOP; }
"!="       { yylval.Int = BNE; return RELOP; }
">="       { yylval.Int = BGE; return RELOP; }
">"        { yylval.Int = BGT; return RELOP; }
"else"     { return ELSE; }
"if"       { return IF; }
"int"      { yylval.Int = INT; return TYPE; }
"read"     { return READ; }
"return"   { return RETURN; }
"write"    { return WRITE; }
"void"     { yylval.Int = VOID; return TYPE; }
{digit}+   { sscanf(yytext, "%d", &yylval.Int); return NUM; }
{id}       { yylval.Name = IDentry(yytext, yyleng); return ID; }
{other}    { fprintf(stderr, "Illegal char '%c' ignored\n",
                     yytext[0]); }

%%





