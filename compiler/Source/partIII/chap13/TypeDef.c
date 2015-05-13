/* プログラム 13.6 : 型リストの作成（TypeDef.c，316, 317ページ） */

#include <stdio.h>
#include "../VSME.h"
#include "SymTable.h"
#include "TypeDef.h"

char *BTname[] = {"void", "char", "int", "univ*"};
#define NoOfBT sizeof(BTname)/sizeof(BTname[0])
STP SymBTDP[NoOfBT];
TDP BasicTDP[NoOfBT];

static int ByteW[] = { IBSZ, CBSZ, IBSZ, IBSZ }; /* 各型のバイト数 */


TDP MakeTdesc(Dtype Constrct, TDP Texpr)       /* 型セルの生成 */
{
  TDP tp = (TDP)calloc(1, sizeof(TypeDesc));   /* 型セルの領域の確保 */

  tp->tcons = Constrct; tp->compnt = Texpr;    /* 型構成子，対象の型 */
  return tp;
}

void SetUpSymTab(void)                         /* 基本型名の記号表へ */
{                                              /* の登録と，型セルの */
  Dtype i;                                     /* 初期設定 */

  for (i = VOID; i < NoOfBT; i++) { TDP tp;
    SymBTDP[i] = MakeEntry(BTname[i], DTYPE);  /* 記号表への登録 */
    BasicTDP[i] = SymBTDP[i]->type = tp = MakeTdesc(i, NULL);
    tp->msize = tp->atoms = ByteW[i]; }
}

TDP RefType(TDP Texpr)                         /* REF型セルの生成 */
{
  TDP tp = MakeTdesc(REF, Texpr);

  tp->msize = tp->atoms = ByteW[REF];          /* バイト数と境界調整数 */
  return tp;                                   /* の設定 */
}

void ArrayDesc(TDP Atp, TDP ElmnT)             /* 要素型の設定 */
{
  if (Atp->compnt == NULL)                     /* 型リストの最後 ? */
    Atp->compnt = ElmnT;                       /* 要素型リストの設定 */
  else {
    ArrayDesc(Atp->compnt, ElmnT);             /* 下位の要素型への設定 */
    ElmnT = Atp->compnt; }
  Atp->atoms = ElmnT->atoms;                   /* 境界調整数のコピー */
  Atp->msize = Atp->numelm < 0 ? 0 :  ElmnT->msize * Atp->numelm;
}

int SameType(TDP T1, TDP T2)                   /* 型の同値性の判定 */
{
  if (T1->tcons != T2->tcons) return 0;        /* 型構成子の不一致 */
  switch (T1->tcons) {
    case REF:     return SameType(T1->compnt, T2->compnt);
    case ARRAY: { int n1 = T1->numelm, n2 = T2->numelm;
                  if (n1 > 0 && n2 > 0 && n1 != n2 )
                    yyerror("Array size unmatched");
                  return SameType(T1->compnt, T2->compnt); }
    case STRUCT:                               /* 構造体型 */
    case INCMP:   return T1 == T2; }           /* 不完全型 */
  return 1;                                    /* 同じスカラ型 */
}

void CmptblType(TDP OrgT, TDP ActT)            /* 代入の適合性の判定 */
{
  switch (OrgT->tcons) {
    case CHAR:                                 /* 基本型どうし */
    case INT:    if (ActT->tcons == CHAR || ActT->tcons == INT)
                   return;
                 break;
    case REF:    if (ActT == UNIVP) return;    /* NULLとの演算 */
    case STRUCT:
    case ARRAY:  if (SameType(OrgT, ActT)) return; }
  yyerror("Incompatible type of operand");
}

void PrintType(TDP tp)
{
  static char *TypeS[] = { "void", "char", "int", "ref(", "[", "struct{",
                           "incomplete" };
  if (tp == NULL) return;
  printf("%s", TypeS[tp->tcons]);
  switch(tp->tcons) {
  case REF:    PrintType(tp->compnt); printf(")"); break;
  case ARRAY:  PrintType(tp->compnt);
               printf(tp->numelm < 0 ? ", *" : ", %d", tp->numelm);
               printf("]"); break;
  case STRUCT: printf("%d bytes(%d)}", tp->msize, tp->atoms); }
}
