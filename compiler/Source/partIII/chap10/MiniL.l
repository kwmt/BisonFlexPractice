/* プログラム 10.4 : MiniLの字句の定義（MiniL.l，210, 211ページ） */

%{
#include "SymTable.h"
#include "ExprTree.h"
#include "y.tab.h"
#include "../VSME.h"

char *IDentry(char *Name, int Len), *StrScan(void);
%}

ws         [ \t\n]
sym        [=,&!():;{}[\]]
digit      [0-9]
letter     [a-zA-Z_]
id         {letter}({letter}|{digit})*
real_num   {digit}+"."{digit}*((E|e)("+"|"-")?{digit}+)?
string     \"([^\"\\\n]|\\.)*[\"\n]
other      .
%%

{ws}+           ;
{sym}           { return(yytext[0]); }
"+"             { yylval.Int = ADD; return ADDOP; }
"-"             { yylval.Int = SUB; return ADDOP; }
"*"             { yylval.Int = MUL; return MULOP; }
"/"             { yylval.Int = DIV; return MULOP; }
"%"             { yylval.Int = MOD; return MULOP; }
"++"            { yylval.Int = INC; return PPMM; }
"--"            { yylval.Int = DEC; return PPMM; }
"||"            { return LOR; }
"&&"            { return LAND; }
"<"             { yylval.Int = BLT; return RELOP; }
"<="            { yylval.Int = BLE; return RELOP; }
"=="            { yylval.Int = BEQ; return RELOP; }
"!="            { yylval.Int = BNE; return RELOP; }
">="            { yylval.Int = BGE; return RELOP; }
">"             { yylval.Int = BGT; return RELOP; }
"'"."'"         { yylval.Int = yytext[1]; return CNUM; }
"'\\n'"         { yylval.Int = '\n'; return CNUM; }
"char"          { yylval.Int = CHAR; return TYPE; }
"do"            { return DO; }
"double"        { yylval.Int = DBL;  return TYPE; }
"else"          { return ELSE; }
"for"           { return FOR; }
"if"            { return IF; }
"int"           { yylval.Int = INT;  return TYPE; }
"read"          { return READ; }
"return"        { return RETURN; }
q"static"        { return STATIC; }
"void"          { yylval.Int = VOID; return TYPE; }
"write"         { return WRITE; }
"while"         { return WHILE; }
{digit}+        { sscanf(yytext, "%d", &yylval.Int);
                  return NUM; }
{real_num}      { sscanf(yytext, "%lf", &yylval.Dbl);
                      return RNUM; }
{id}            { yylval.Name = IDentry(yytext, yyleng);
                  return ID; }
{string}        { yylval.Name = StrScan(); return STRING; }
{other}         { fprintf(stderr, "Illegal char '%c' ignored\n",
                          yytext[0]); }
%%

char *StrScan(void)
{
  char *p, *q;

  if (yytext[yyleng-1] != '\"')
    yyerror("Unterminated string literal");
  for (p=yytext+1, q=yytext; p < yytext+(yyleng-1); *(q++) = *(p++))
    if (*p == '\\')
      if (*(++p) == 'n') *p = '\n';
      else if (*p == 't') *p = '\t';
  *q = '\0';
  yyleng = q - yytext;
  return yytext;
}





