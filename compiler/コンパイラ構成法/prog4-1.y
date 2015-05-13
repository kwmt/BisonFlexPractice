/* プログラム4.1（55ページ） */

%{
int ipower(int, int);            /* べき乗計算の関数 */
%}

%token NUM UMINUS

%left  '+' '-'
%left  '*' '/'
%right '^'
%right UMINUS

%%
line   :                         /* 空規則 */
       | line expr '\n'          { printf("%d\n", $2); }
       ;

expr   : expr '+' expr           { $$ = $1 + $3; }
       | expr '-' expr           { $$ = $1 - $3; }
       | expr '*' expr           { $$ = $1 * $3; }
       | expr '/' expr           { $$ = $1 / $3; }
       | expr '^' expr           { $$ = ipower($1, $3); }
       | '-' expr %prec UMINUS   { $$ = -$2; }
       | '(' expr ')'            { $$ = $2; }
       | NUM
       ;
%%

#include <ctype.h>

yylex()
{
  int c;

  while ((c = getchar()) == ' ');
  if (isdigit(c)) {
    yylval = c - '0';
    while (isdigit(c = getchar()))
      yylval = yylval*10 + (c-'0');
    ungetc(c, stdin);
    return NUM; }
  else
    return c;
}

int ipower(int m, int n)
{
  int e;

  if (n < 0)
    return 0;
  else
    for (e = 1; n > 0; n--)
      e *= m;
  return e;
}

