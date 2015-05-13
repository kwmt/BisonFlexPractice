/* プログラム5.3用のLexプログラム */

%{
#include "y.tab.h"
%}

ws         [ \t]
symbol     [-+\n]
digit      [0-9]
other      .

%%

{ws}+                ;
{sym}                { return(yytext[0]); }
{digit}+             { sscanf(yytext, "%d", &yylval.ival);
                       return(INTC); }
{digit}+"."{digit}*  { sscanf(yytext, "%lf", &yylval.rval);
                       return(REALC); }
{other}              { fprintf(stderr, "Illegal char '%c' ignored\n",
                               yytext[0]); }

%%
