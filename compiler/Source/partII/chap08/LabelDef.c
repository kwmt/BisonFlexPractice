/* プログラム 8.3 : switch文とラベルの処理（LabelDef.c，152〜153ページ） */

#include "../VSM.h"

#define CT_SIZE 100                            /* ラベル表の大きさ */
typedef struct {                               /* エントリの定義 */
  int label;                                   /* caseラベル */
  int addr;                                    /* 対応するアドレス */
} CSentry;
static CSentry Cstk[CT_SIZE], *CaseP = Cstk;   /* ラベル表とその */
                                               /* ポインタの定義 */
#define SWT_SIZE 10
struct {
  CSentry *CTptr;                              /* ラベル表へのポインタ */
  int     DefltAddr;                           /* defaultのアドレス */
} SWtbl[SWT_SIZE];                             /* caseラベルスタック */
static int SWTptr = 0;                         /* そのスタックポインタ */

void BeginSW(void)                             /* switch文の開始 */
{
  SWtbl[++SWTptr].CTptr = CaseP;
  SWtbl[SWTptr].DefltAddr = -1;                /* defaultラベルを */
}                                              /* 未定義の状態に  */

void CaseLbl(int Clabel)                       /* ラベル定義の処理 */
{
  CSentry *p;

  if (SWTptr > 0) {
    CaseP->label = Clabel;                     /* 二重定義 ? */
    for (p = SWtbl[SWTptr].CTptr; p->label != Clabel; p++);
    if (p >= CaseP) {
      CaseP->addr = PC();                      /* Isegアドレスを格納 */
      CaseP++; }
    else
      yyerror("Duplicated case constant");
  }
}

void DfltLbl(void)                             /* defaultラベルの処理 */
{
  if (SWTptr > 0 && SWtbl[SWTptr].DefltAddr < 0)  /* 二重定義チェック */
    SWtbl[SWTptr].DefltAddr = PC();           /* Isegのアドレスを格納 */
  else
    yyerror("Illegal default label");
}

void EndSW(void)                              /* switch文の終了 */
{
  CSentry *p;

  for (p = SWtbl[SWTptr].CTptr; p < CaseP; p++) {   /* 分岐命令の生成 */
    Pout(COPY);   Cout(PUSHI, p->label);
    Pout(COMP);   Cout(BNE, PC()+3);
    Pout(REMOVE); Cout(JUMP, p->addr); }
  Pout(REMOVE);  
  if (SWtbl[SWTptr].DefltAddr > 0)            /* defaultラベルが定義? */
    Cout(JUMP, SWtbl[SWTptr].DefltAddr);      /* そこへのジャンプ命令 */
  CaseP = SWtbl[SWTptr--].CTptr;              /* ラベル表のポインタを */
}                                             /* 戻す */ 
