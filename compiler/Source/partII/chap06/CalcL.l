/* プログラム 6.3（CalcL.l，103ページ） */

%{
#include "y.tab.h"
#include "../VSM.h"
%}

ws         [ \t\n]
symbol     [();]
digit      [0-9]
other      .

%%

{ws}+
{symbol}   { return yytext[0]; }
"+"        { yylval = ADD; return ADDOP; }
"-"        { yylval = SUB; return ADDOP; }
"*"        { yylval = MUL; return MULOP; }
"/"        { yylval = DIV; return MULOP; }
"%"        { yylval = MOD; return MULOP; }
{digit}+   { sscanf(yytext, "%d", &yylval);
             return NUM; }
"/*"       { if (SkipComment() == EOF) return EOF; }
{other}    { fprintf(stderr, "Illegal char '%c' ignored\n", yytext[0]); }

%%

int SkipComment(void)
{
  int c;

  while(1) {
    while ((c=input()) != '*' && c != EOF) ;
    if (c == '*') {
      while ((c=input()) == '*') ;
      if (c == '/') return 1; }
    if (c == EOF) {
      fprintf(stderr, "Unterminated comment\n");
      return EOF; }
  }
}