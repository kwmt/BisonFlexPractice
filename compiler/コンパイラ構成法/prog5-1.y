/* プログラム5.1 : メモリ付き関数電卓（67ページ） */

%{
#include <math.h>

#define M_SIZE 16
double  Memory[M_SIZE];                       /* 電卓用のメモリの定義 */
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
line   :                         /* 空規則 */
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
  yyparse();                     /* 式の構文解析と計算 */
}

yyerror(char *msg)               /* エラーメッセージの表示 */
{
  printf("%s\n", msg);
}
