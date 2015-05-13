/* プログラム 11.4 : 式の木の作成（ExprTree.c，238, 240, 242, 245, */
/*                                                  248, ページ） */

#include <stdio.h>
#include <stdlib.h>
#include "SymTable.h"
#include "../VSME.h"
#include "ExprTree.h"

#define NUMCHK(X) { if ((X)->type < CHAR || (X)->type > DBL) \
                              yyerror("Illegal type of expression"); }
#define FMTCHK(X) { if ((X)->type != C_ARY) \
                              yyerror("Illegal format specification"); }
#define LHSCHK(X) { if ((X)->Op==IDS || (X)->Op==AELM) NUMCHK(X)\
                      else yyerror("Illegal L.H.S. expression"); }

#define DCT_SIZE 50
DCdesc  DCtbl[DCT_SIZE];                        /* 定数表 */
static  int NoOfDC = 0;                         /* 定数表のエントリ数 */

static Dtype ToSameType(NP *D1, NP *D2);        /* 2つの式を同じ型に */
static NP    ToDouble(NP np);                   /* double型への変換 */


NP AllocNode(int Nid, NP lt, NP rt)             /* 節点の作成 */
{
  NP np = (NP)malloc(sizeof(Node));             /* 領域の獲得 */

  np->Op = Nid; np->left = lt; np->right = rt;  /* 演算子，オペランド */
  return np;
}

void FreeSubT(NP np)                            /* 式の木のメモリ解放 */
{
  if (np != NULL) {
    if (np->Op != CONS && np->Op != IDS) {      /* 整数定数の節点 ? */
     FreeSubT(np->left); FreeSubT(np->right); } /* 左右の部分木を解放 */
     free(np); }                                /* 自分の領域を解放 */
}

NP MakeL(char *Name)                            /* 識別子用の節点作成 */
{
  STP sp = SymRef(Name);                        /* 記号表探索 */
  NP np;

  np = AllocNode(IDS, (NP)sp, NULL);            /* 節点の領域割付け */
  np->type = sp->type;                          /* 型を格納 */
  if (sp->class == VAR && sp->dim > 0)          /* 配列型の変数 ? */
    np->type |= ARY;                            /* 配列表示ビット設定 */
  return np;
}

NP MakeN(int Nid, NP lt, NP rt)                 /* 演算用の節点の作成 */
{
  NP np = AllocNode(Nid, lt, rt);               /* 節点の領域獲得 */
  STP sp;

  np->type = lt->type;                          /* 演算結果の型の設定 */
  switch (Nid) {
    case ASSGN :                                      /* 代入演算子 */
      LHSCHK(lt); np->right = TypeConv(rt, lt->type); break;
    case ADD:  case SUB:  case MUL:  case DIV:        /* 四則演算 */
      np->type = ToSameType(&np->left, &np->right); break;
    case CSIGN:  NUMCHK(lt);                          /* 単項マイナス */
      if (lt->Op == CONS) {                           /* −整数定数 ? */
        if (lt->type == INT)  
          lt->left = (NP)(-INTV(lt));
        else if (lt->type == DBL)                     /* −実数定数 ? */
          lt->left = (NP)Wcons(-DCtbl[INTV(lt)].Dcons);
        free(np);
        return lt; }
      break;
    case INC:  case DEC:  LHSCHK(lt);                 /* ++, -- */
    case NOT:  case MOD:  case AND:  case OR:         /* !, %, &&, || */
      if (rt != NULL) INTCHK(rt);
      INTCHK(lt); np->type = INT; break;
    case COMP:                                        /* 関係演算子 */
      ToSameType(&np->left, &np->right); np->type = INT; break;
    case AELM:  if (rt->Op != SUBL) INTCHK(rt);       /* 配列要素 */
      if (sp = SYMP(lt)) {
        np->etc = (rt->Op==SUBL)? rt->etc : 1;
        if (np->etc == sp->dim)
          np->type &= (~ARY);
        else if (np->etc > sp->dim) 
          yyerror("Too many subscripts or non-array"); }
      break;
    case SUBL:  INTCHK(rt)                            /* 添字式並び*/
      if (lt->Op != SUBL) INTCHK(lt);
    case ARGL:                                        /* 実引数並び */
      np->etc = (lt->Op==Nid)? lt->etc+1: 2; break;
    case CALL:                                        /* 関数呼出し */ 
      if (sp = SYMP(lt)) { int n;
        n = (rt == NULL)? 0 : (rt->Op == ARGL)? rt->etc: 1;
        if (sp->Nparam != n) yyerror("Number of arguments unmatched");
        if (sp->class != FUNC && sp->class != F_PROT)
          yyerror("Trying to call non-funcation"); }
      break;
    case INPUT:                                      /* read(fmt, v) */
      FMTCHK(lt); LHSCHK(rt); np->type = INT; break;
    case OUTPUT:  NUMCHK(rt);                        /* write(fmt, e) */
    case OUTSTR:  FMTCHK(lt); np->type = VOID;       /* write(fmt) */ 
    }
  return np;
}

NP TypeConv(NP ep, Dtype To)                    /* 型変換 */
{
  Dtype T = ep->type;

  if (T < CHAR || T> DBL)                       /* 基本データ型 ? */
    yyerror("Illegal type conversion");
  switch (To) {
    case CHAR:  case INT:                       /* int型への変換 */
      if (T == DBL) {                           /* 変換対象はdouble? */
        if (ep->Op == CONS)                     /* 実数定数をintへ? */
          ep->left = (NP)(int)DCtbl[INTV(ep)].Dcons;
        else
          ep = AllocNode(DBLINT, ep, NULL);     /* 式の値をint型へ ? */
        ep->type = INT; }
      break;
    case DBL:                                   /* double型への変換 */
      return (T==INT || T==CHAR)? ToDouble(ep): ep;
    default: yyerror("Illegal casting");
  }
  return ep;
}

static Dtype ToSameType(NP *D1, NP *D2)         /* 2つの式の型を */
{                                               /* そろえるための変換 */
  Dtype T1, T2;

  if ((T1 = (*D1)->type) == CHAR) T1 = INT;     /* 第1オペランドの型 */
  if ((T2 = (*D2)->type) == CHAR) T2 = INT;     /* 第2オペランドの型 */
  if (T1 == VOID || T2 == VOID || T1 & ARY || T2 & ARY)
    yyerror("Incompatible type of binary operation");
  else if (T1 == INT && T2 == DBL)              /* (int) op (double) */
    *D1 = ToDouble(*D1);
  else if (T1 == DBL && T2 == INT)              /* (double) op (int) */
    *D2 = ToDouble(*D2);
  return T1 > T2 ? T1 : T2;                     /* 演算結果の型 */
}

static NP ToDouble(NP np)                       /* double型への変換 */
{
  if (np->Op == CONS)                           /* 整数定数 ? */
    np->left = (NP)Wcons((double)INTV(np));
  else                                          /* そのほかの場合 */
    np = AllocNode(INTDBL, np, NULL);           /* 強制型変換の挿入 */
  np->type = DBL;                               /* 変換後の型はdouble */
  return np;
}

int Wcons(double C)                             /* 定数表への登録 */
{
  int i;

  DCtbl[NoOfDC].Dcons = C;                      /* センチネルの設定 */
  for (i = 0; DCtbl[i].Dcons != C; i++) ;       /* 定数表の線形探索 */
  if (i >= NoOfDC)
    DCtbl[NoOfDC++].addr = -1;                  /* 初出の実数定数 */
  return i;
}

static int TempC;                               /* 一時変数のカウンタ */

void WriteTree(NP root)                         /* 式の木の３つ組表示 */
{
  extern int yylineno;

  TempC = 0;
  printf("\nLine %d\n", yylineno);              /* 行番号の表示 */
  WriteNode(root);  printf("\n");
}

int WriteNode(NP np)                            /* 節点の３つ組表示 */
{
  int opc, t1, t2;
  char *ops;
  STP sp;
  static char *SymD[] = { "void", "char", "int", "double" },
              *Ccode[]= { "and", "or", "not", "++", "--", 
                          "[ ]", "subs", "arg"};
  extern char *Scode[];                         /* 記号操作コード表 */

  if (np == NULL) return -1;
  switch(opc = np->Op) {                        /* 節点の識別子は ? */
    case IDS:  sp = SYMP(np);                           /* 識別子 */
      printf("%5d  %-10s ", TempC, "id");
      printf("%-10s%-12s\n", sp->name, SymD[sp->type]); break;
    case CONS:  printf("%5d  ", TempC);                 /* 定数 */
      if (np->type == INT)                              /* 整数定数 */
        printf("%-4s %7d\n", "Int", INTV(np));
      else if (np->type == DBL)                         /* 実数定数 */
        printf("%-8s %f\n", "Double", DCtbl[INTV(np)].Dcons);
      else if (np->type == C_ARY)                       /* 文字列 */
        printf("%-8s loc. %6d\n", "String", INTV(np));
      break;
    default:                                            /* 演算子 */
      t1 = WriteNode(np->left); t2 = WriteNode(np->right);
      ops = (opc < AND) ? Scode[opc] : Ccode[opc-AND];  /* 操作コード */
      printf("%5d  %-8s (%2d)", TempC, ops, t1);
      if (t2 >= 0) printf(" %5c (%2d)", ' ', t2);       /* ２項演算? */
      printf("\n"); }
  return TempC++;
}
