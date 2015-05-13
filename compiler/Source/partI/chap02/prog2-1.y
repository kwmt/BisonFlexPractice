/* $B%W%m%0%i%`(B2.1$B!J(B21$B%Z!<%8!K(B */

/*  Yacc$B$G5-=R$7$?<0$NDj5A(B  */

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

