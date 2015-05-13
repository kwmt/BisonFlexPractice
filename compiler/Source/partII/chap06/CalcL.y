/* $B%W%m%0%i%`(B 6.2 $B!J(BCalcL.y$B!$(B101$B%Z!<%8!K(B */

%{
#include "../VSM.h"
%}

%token  NUM                            /* 10$B?JDj?t$rI=$9%H!<%/%s(B */

%left   ADDOP                          /* $B2C8:1i;;;R(B */
%left   MULOP                          /* $B>h=|1i;;;R(B */
%right  UMINUS                         /* $BC19`%^%$%J%91i;;;R(B */

%%
program   : expr_list                  { Pout(HALT); }
          ;

expr_list : 
          | expr_list expr  ';'        { Pout(OUTPUT); }
          | expr_list error ';'        { yyerrok; }
          ;

expr      : expr ADDOP expr            { Pout($2); }
          | expr MULOP expr            { Pout($2); }
          | '(' expr ')'
          | ADDOP expr %prec UMINUS    { if ($1 == SUB) Pout(CSIGN); }
          | NUM                        { Cout(PUSHI, $1); }
          ;
%%

#define TraceSW 0                      /* $B%H%l!<%9%U%i%0$r%*%U$K@_Dj(B */

main()
{
  SetPC(0);                            /* Pctr$B$r(B0$B$K@_Dj(B */
  yyparse();                           /* $B<0$NJB$S$r(BVSM$B%3!<%I$KK]Lu(B */
  DumpIseg(0, PC()-1);                 /* $BL\E*%3!<%I$N%@%s%W(B */
  printf("Enter execution phase\n");
  if (StartVSM(0, TraceSW) != 0)       /* $BL\E*%3!<%I$N<B9T$H(B */
    printf("Execution aborted\n");     /* $B=*N;>uBV$N%A%'%C%/(B */
}

yyerror(char *msg)
{
  printf("%s\n", msg);
}


