/* プログラム 6.2 （CalcL.y，101ページ） */

%{
#include "../VSM.h"
%}

%token  NUM                            /* 10進定数を表すトークン */

%left   ADDOP                          /* 加減演算子 */
%left   MULOP                          /* 乗除演算子 */
%right  UMINUS                         /* 単項マイナス演算子 */

%%
program   : expr_list                  { Pout(HALT); }
          ;

expr_list : 
          | expr_list expr  ';'        { Pout(OUTPUT); }
          | expr_list error ';'        { yyerrok; }
          ;

expr      : expr ADDOP expr            { Pout($2); }
          | expr MULOP expr            { Pout($2); }
          | '(' expr ')'
          | ADDOP expr %prec UMINUS    { if ($1 == SUB) Pout(CSIGN); }
          | NUM                        { Cout(PUSHI, $1); }
          ;
%%

#define TraceSW 0                      /* トレースフラグをオフに設定 */

main()
{
  SetPC(0);                            /* Pctrを0に設定 */
  yyparse();                           /* 式の並びをVSMコードに翻訳 */
  DumpIseg(0, PC()-1);                 /* 目的コードのダンプ */
  printf("Enter execution phase\n");
  if (StartVSM(0, TraceSW) != 0)       /* 目的コードの実行と */
    printf("Execution aborted\n");     /* 終了状態のチェック */
}

yyerror(char *msg)
{
  printf("%s\n", msg);
}


