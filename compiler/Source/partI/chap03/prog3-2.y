/* $B%W%m%0%i%`(B3.2$B!J(B41$B%Z!<%8!K(B */

/*   $B?t<0$N7W;;(B   */

%token NUM                                     /* $B%H!<%/%s$NDj5A(B */

%%
line   : expr '\n'          { printf("%d\n", $1); }
       ;

expr   : expr '+' term      { $$ = $1 + $3; }
       | expr '-' term      { $$ = $1 - $3; }
       | term
       ;

term   : term '*' factor    { $$ = $1 * $3; }
       | term '/' factor    { $$ = $1 / $3; }
       | factor
       ;

factor : '(' expr ')'       { $$ = $2; }
       | NUM
       ;
%%

#include <ctype.h>

yylex()                                        /* $B;z6g2r@O4X?t(B */
{
  int c;

  while ((c = getchar()) == ' ') ;             /* $B6uGr$rFI$_Ht$P$9(B */
  if (isdigit(c)) {                            /* $B?t;z$N=hM}(B */
    yylval = c - '0';
    while (isdigit(c = getchar()))             /* $B?t;zNs$r(Bint$B7?$NCM$X(B */
      yylval = yylval*10 + (c-'0');
    ungetc(c, stdin);                          /* $BF~NOJ8;z$r$b$H$KLa$9(B */
    return NUM; }
  else                                         /* $B6uGr!$?t;z0J30$NJ8;z(B */
    return c;
}
