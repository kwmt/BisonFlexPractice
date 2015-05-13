/* プログラム 8.2 : 分岐先の管理（JumpChain.c，151ページ） */

#include "../VSM.h"

#define CTRLT_SIZE 20                          /* 分岐スタックサイズ */
struct {
  int StType;                                  /* 文のコード*/
  int Bchain, Cchain;                          /* 分岐先リストヘッダ */
} Ctable[CTRLT_SIZE];                          /* 分岐スタック */

static int CSptr = 0;                          /* 分岐スタックポインタ */

void NestIn(int St)                            /* 繰返し文，switch文 */
{                                              /* の始まり */
  Ctable[++CSptr].StType = St;
  Ctable[CSptr].Bchain = Ctable[CSptr].Cchain = -1;
}

void GenBrk(int JC)                            /* break文の処理 */
{
  if (CSptr > 0) {
    Cout(JC, Ctable[CSptr].Bchain);            /* 分岐命令の生成 */
    Ctable[CSptr].Bchain = PC()-1; }           /* breakチェインの更新 */
  else
    yyerror("Illegal use of break statement");
}

void GenConti(void)                            /* continue文の処理 */
{
  int i;

  for (i=CSptr; i>0 && Ctable[i].StType==0; i--) ;   /* switch文 ? */
  if (i <= 0)                                  /* スタックが空 ? */
    yyerror("Illegal use of the continue");     
  else {
    Cout(JUMP, Ctable[i].Cchain);              /* 分岐命令の生成 */
    Ctable[i].Cchain = PC() - 1; }             /* 後埋めチェイン更新 */
}

void NestOut(int ContP)                        /* 繰返し文，switch文 */
{                                              /* の終り */
    Bpatch(Ctable[CSptr].Cchain, ContP);       /* continue文の後埋め */
    Bpatch(Ctable[CSptr].Bchain, PC());        /* break文の後埋め */
  CSptr--;
}

