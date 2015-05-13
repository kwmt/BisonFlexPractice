/* プログラム3.2（41ページ） */

/*   数式の計算   */

%token NUM                                     /* トークンの定義 */

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

yylex()                                        /* 字句解析関数 */
{
  int c;

  while ((c = getchar()) == ' ') ;             /* 空白を読み飛ばす */
  if (isdigit(c)) {                            /* 数字の処理 */
    yylval = c - '0';
    while (isdigit(c = getchar()))             /* 数字列をint型の値へ */
      yylval = yylval*10 + (c-'0');
    ungetc(c, stdin);                          /* 入力文字をもとに戻す */
    return NUM; }
  else                                         /* 空白，数字以外の文字 */
    return c;
}
