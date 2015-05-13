/* プログラム2.1（21ページ） */

/*  Yaccで記述した式の定義  */

%%

input  : expr '\n' ;
expr   : expr '+' term | expr '-' term | term ;
term   : term '*' factor | term '/' factor | factor ;
factor : num | '(' expr ')' ;
     num    : digit | num digit ;
     digit  : '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' ;

%%

yylex()
{
  return getchar();
}

