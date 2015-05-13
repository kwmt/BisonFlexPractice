/* $B%W%m%0%i%`(B5.1 : $B%a%b%jIU$-4X?tEEBn!J(B67$B%Z!<%8!K(B */

%{
#include <math.h>

#define M_SIZE 16
double  Memory[M_SIZE];                       /* $BEEBnMQ$N%a%b%j$NDj5A(B */
%}

%union{
  int     ival;
  double  rval;
}

%token        MEM EXP LOG SQRT
%token <ival> INTC
%token <rval> REALC

%type  <ival> mcell
%type  <rval> expr

%right '='
%left  '+' '-'
%left  '*' '/'
%right '^'
%right UMINUS

%%
line   :                         /* $B6u5,B'(B */
       | line expr  '\n'         { printf("%f\n", $2); } 
       | line error '\n'         { yyerrok; } 
       ;

expr   : mcell '=' expr          { $$ = Memory[$1] = $3; }
       | expr '+' expr           { $$ = $1 + $3; }
       | expr '-' expr           { $$ = $1 - $3; }
       | expr '*' expr           { $$ = $1 * $3; }
       | expr '/' expr           { $$ = $1 / $3; }
       | expr '^' expr           { $$ = pow($1, $3); }
       | '-' expr %prec UMINUS   { $$ = -$2; }
       | '(' expr ')'            { $$ = $2; }
       | LOG  '(' expr ')'       { $$ = log($3); }
       | EXP  '(' expr ')'       { $$ = exp($3); }
       | SQRT '(' expr ')'       { $$ = sqrt($3); }
       | mcell                   { $$ = Memory[$1]; }
       | INTC                    { $$ = (double)$1; }
       | REALC                   { $$ = $1; }
       ;

mcell  : MEM '[' INTC ']'        { if ($3>=0 && $3<M_SIZE)
                                     $$ = $3;
                                   else {
                                     printf("Memory Fault\n");
                                     $$ = 0; }
                                 }
       ;
%%

main()
{
  yyparse();                     /* $B<0$N9=J82r@O$H7W;;(B */
}

yyerror(char *msg)               /* $B%(%i!<%a%C%;!<%8$NI=<((B */
{
  printf("%s\n", msg);
}
