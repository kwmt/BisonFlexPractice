/* Yaccで記述した四季の定義 */
%{
#include<stdio.h>
%}
%%

input: expr '\n'      { printf("correct expression\n");}
     ;
expr : expr '+' term  { printf("expr + term -> expr\n"); }
     | expr '-' term  { printf("expr - term -> expr\n"); }
     | term           { printf("term -> expr\n"); }
     ;
term: term '*' factor { printf("term * factor -> term\n"); }
    | term '/' factor { printf("term / factor -> term\n"); }
    | factor          { printf("factor -> term\n"); }
    ;
factor : 'i'          { printf("i -> factor\n"); }
       | '(' expr ')' { printf("( expr ) -> factor\n"); }
;

%%

yylex()
{
  int c;
  switch( c = getchar() ){
    case EOF:  printf("EOF detected\n"); break;
    case '\n': printf("--> '\\n'\n");    break;
    default:   printf("--> '%c'\n",c);   break;
  }
  return c;
}
