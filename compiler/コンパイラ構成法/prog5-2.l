/* $B%W%m%0%i%`(B5.2$B!J(B73$B%Z!<%8!K(B */

/*   $B%a%b%jIU$-4X?tEEBn%W%m%0%i%`$N;z6gDj5A(B   */

%{
#include "y.tab.h"
%}

ws         [ \t]
symbol     [=+\-*/\^()[\]\n]
digit      [0-9]
other      .

%%

{ws}+ 
{symbol}              { return(yytext[0]); }
"M"                   { return(MEM); }
"exp"                 { return(EXP); }
"log"                 { return(LOG); }
"sqrt"                { return(SQRT); }
{digit}+              { sscanf(yytext, "%d", &yylval.ival);
                        return(INTC); }
{digit}+"."{digit}*   { sscanf(yytext, "%lf", &yylval.rval);
                        return(REALC); }
{other}               { fprintf(stderr, "Illegal char '%c' ignored\n",
                                yytext[0]); }

%%
