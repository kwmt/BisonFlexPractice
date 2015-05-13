/* CntrlLの字句の定義（CntrlL.l） */

%{
#include "y.tab.h"
#include "../VSM.h"

char *IDentry(char *, int);
%}

ws         [ \t\n]
symbol     [=?!(),;:{}]
digit      [0-9]
letter     [a-zA-Z_]
id         {letter}({letter}|{digit})*
other      .

%%

{ws}+      ;
{symbol}   { return yytext[0]; }
"+"        { yylval.Int = ADD; return ADDOP; }
"-"        { yylval.Int = SUB; return ADDOP; }
"*"        { yylval.Int = MUL; return MULOP; }
"/"        { yylval.Int = DIV; return MULOP; }
"%"        { yylval.Int = MOD; return MULOP; }
"++"       { yylval.Int = INC; return PPMM; }
"--"       { yylval.Int = DEC; return PPMM; }
"&&"       { return LAND; }
"||"       { return LOR; }
"<"        { yylval.Int = BLT; return RELOP; }
"<="       { yylval.Int = BLE; return RELOP; }
"=="       { yylval.Int = BEQ; return RELOP; }
"!="       { yylval.Int = BNE; return RELOP; }
">="       { yylval.Int = BGE; return RELOP; }
">"        { yylval.Int = BGT; return RELOP; }
"break"    { return BREAK; }
"case"     { return CASE; }
"continue" { return CONTI; }
"default"  { return DEFAULT; }
"do"       { return DO; }
"else"     { return ELSE; }
"for"      { return FOR; }
"if"       { return IF; }
"int"      { return TYPE; }
"read"     { return READ; }
"switch"   { return SWITCH; }
"while"    { return WHILE; }
"write"    { return WRITE; }
{digit}+   { sscanf(yytext, "%d", &yylval.Int); return NUM; }
{id}       { yylval.Name = IDentry(yytext, yyleng); return ID; }
{other}    { fprintf(stderr, "Illegal char '%c' ignored\n",
                     yytext[0]); }

%%
