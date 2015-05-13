/* $B%W%m%0%i%`(B4.2$B!J(B61$B%Z!<%8!K(B */

%{
#include <math.h>
#define  YYSTYPE double          /* $B0UL#CM$N7?Dj5A(B */
%}

%token NUM UMINUS

%left  '+' '-'
%left  '*' '/'
%right '^'
%right UMINUS

%%
line   :                         /* $B6u5,B'(B */
       | line expr  '\n'         { printf("%f\n", $2); }
       | line error '\n'         { yyerrok; }
       ;

expr   : expr '+' expr           { $$ = $1 + $3; }
       | expr '-' expr           { $$ = $1 - $3; }
       | expr '*' expr           { $$ = $1 * $3; }
       | expr '/' expr           { $$ = $1 / $3; }
       | expr '^' expr           { $$ = pow($1, $3); }
       | '-' expr %prec UMINUS   { $$ = -$2; }
       | '(' expr ')'            { $$ = $2; }
       | NUM
       ;
%%

yylex()
{
  int c;

  while ((c = getchar()) == ' ');                 /* $B6uGr$NFIHt$P$7(B */
  if (((c >= '0') && (c <= '9')) || c == '.') {   /* $B?t;z!$>.?tE@(B ? */
    ungetc(c, stdin);                             /* $BF~NO$KLa$7$F$+$i(B */
    scanf("%lf", &yylval);                        /* $B%G!<%?$rFI$`(B */
    return NUM; }
  else
    return c;
}
