/* プログラム4.2（61ページ） */

%{
#include <math.h>
#define  YYSTYPE double          /* 意味値の型定義 */
%}

%token NUM UMINUS

%left  '+' '-'
%left  '*' '/'
%right '^'
%right UMINUS

%%
line   :                         /* 空規則 */
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

  while ((c = getchar()) == ' ');                 /* 空白の読飛ばし */
  if (((c >= '0') && (c <= '9')) || c == '.') {   /* 数字，小数点 ? */
    ungetc(c, stdin);                             /* 入力に戻してから */
    scanf("%lf", &yylval);                        /* データを読む */
    return NUM; }
  else
    return c;
}
