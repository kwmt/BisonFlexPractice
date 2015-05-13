/* $B%W%m%0%i%`(B2.2$B!J(B25$B%Z!<%8!K(B*/

/*  $B9=J82r@O$N2aDx$r%H%l!<%9(B */

%%

input  : expr '\n'        { printf("correct expression\n"); }
       ;

expr   : expr '+' term    { printf("expr + term -> expr\n"); }
       | expr '-' term    { printf("expr - term -> expr\n"); }
       | term             { printf("term -> expr\n"); }
       ;

term   : term '*' factor  { printf("term * factor ->term\n"); }
       | term '/' factor  { printf("term / factor ->term\n"); }
       | factor           { printf("factor -> term\n"); }
       ;

factor : 'i'              { printf("i -> factor\n"); }
       | '(' expr ')'     { printf("( expr ) -> factor\n"); }
       ;
%%

yylex()
{
  int c;

  switch (c =getchar()) {
  case EOF:  printf("EOF detected\n"); break;
  case '\n': printf("--> '\\n'\n");    break;
  default:   printf("--> '%c'\n", c);  break;
  }
  return c;
}
