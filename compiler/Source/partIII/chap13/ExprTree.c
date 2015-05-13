/* プログラム 14.3 : 式の木の作成（ExprTree.c，324, 325, 328ページ） */

#include <stdlib.h>
#include "../VSME.h"
#include "SymTable.h"
#include "ExprTree.h"
#include "TypeDef.h"


NP AllocNode(int op, NP lt, NP rt)              /* 節点の作成 */
{
  NP np = (NP)malloc(sizeof(Node));             /* 領域の獲得 */

  np->Op = op; np->left = lt; np->right = rt;   /* 演算子，オペランド */
  return np;
}

void FreeSubT(NP np)                            /* 式の木のメモリ解放 */
{
  if (np != NULL) {
    switch (np->Op) {
      case CONS:  case IDS:  break;             /* 定数，識別子 ? */
      case CSIGN: case REFOBJ: case ADDROP:
      case MEMOP: case INMEM:      FreeSubT(np->left); break;
      default: FreeSubT(np->left); FreeSubT(np->right); }
    free(np); }                                 /* 自分の領域を解放 */
}

NP MakeL(char *Name)                            /* 識別子用の節点作成 */
{
  STP sp = SymRef(Name);                        /* 記号表探索 */
  NP np;

  np = AllocNode(IDS, (NP)sp, NULL);            /* 節点の領域割付け */
  np->type = sp->type;                          /* 型リストの設定 */
  return np;
}

NP MakeN(int Nid, NP lt, NP rt)                 /* 演算用の節点の作成 */
{
  int t1 = (lt->type)->tcons,                   /* 第１オペランドの型 */
      t2 = rt ? (rt->type)->tcons : INT;        /* 第２オペランドの型 */
  NP np = AllocNode(Nid, lt, rt);               /* 節点の領域獲得 */

  np->type = lt->type;                          /* 演算結果の型の設定 */
  switch (Nid) {
    case ASSGN:                                       /* 代入演算子 */
      switch(lt->Op) {                                /* 左辺の検査 */
        case REFOBJ: case AELM: case MEMOP: case INMEM: case IDS:
          CmptblType(lt->type, rt->type); break;
        default: yyerror("Illegal left-hand side expression"); }
      break;
    case ADD: case SUB: case MUL: case DIV: case MOD: case CSIGN:
      if (t1 == VOID || t1 > INT || t2 == VOID || t2 >INT)
        yyerror("Imcompatible type operation");
      np->type = INTP; break;
    case COMP:                                        /* 関係演算子 */
      if (t1 == VOID || t1 >= ARRAY || t2 == VOID || t2 >= ARRAY)
        yyerror("Imcompatible type operand in relational expression");
      else if (lt->type != UNIVP || t2 != REF)        /* ポインタ ? */
        CmptblType(lt->type, rt->type);               /* 型チェック */
      np->type = INTP; break;
    case REFOBJ:                                      /* 間接演算子 */
      if (t1 == REF && lt->type != UNIVP)
        np->type = (np->type)->compnt;                /* 被参照型 */
      else
        yyerror("Reference to non-pointer type object or null value");
      break;
    case ADDROP:                                      /* ＆演算子 */
      np->type = RefType(lt->type);
      switch (lt->Op) {                               /* 左辺値 ? */
        case IDS: case AELM: case MEMOP: case INMEM: break;
        default: yyerror("Illegal operand of &-operator"); }
      break;
    case AELM:                                        /* 配列要素 */
      if (t2 == VOID || t2 > INT)
        yyerror("Non-integer expression used as subscript");
      if (t1 == ARRAY)                                /* 配列型 ? */
        np->type = (lt->type)->compnt;
      else
        yyerror("Subscript specified to non-array");
      break;
    case ARGL:                                        /* 実引数並び */
      np->etc = (lt->Op == ARGL)? lt->etc + 1: 2;
      break;
    case CALL: { STP sp; int n;                       /* 関数呼出し */
      sp = SYMP(lt);
      n = (rt == NULL) ? 0 : (rt->Op == ARGL) ? rt->etc : 1;
      if (sp->Nparam != n) {                          /* 引数の個数 */
        yyerror("Number of arguments unmatched");
        np->right = NULL; }
      if (sp->class != FUNC && sp->class != F_PROT)
        yyerror("Trying to call non-funcation"); }
      break;
  }
  return np;
}

NP RecSelect(int Op, NP tree, char *FldName)    /* （間接）メンバ演算 */
{
  STP p;
  TDP texpr;

  if (tree->Op == IDS && (SYMP(tree))->class != VAR) {
    yyerror("Reference to non-structured variable");
    return tree; }
  texpr = tree->type;
  if (Op == INMEM) {                            /* メンバの間接参照 ? */
    if (texpr->tcons != REF) {
      yyerror("Reference to non-pointer type object");
      return tree; }
    texpr = texpr->compnt; }
  if (texpr->tcons != STRUCT) {                 /* 構造体型 ? */
    yyerror("Reference to non-struct type object");
    return tree; }
  for (p = (STP)texpr->compnt; p != NULL; p = p->next)
    if (p->name == FldName && p->class == VAR)  /* メンバ名の比較 */
      break;
  if (p == NULL) {                              /* メンバの探索結果 */
    yyerror("Reference to undefined field");
    return tree; }
  if (Op == MEMOP && tree->Op == MEMOP)         /* (Y.g).f ? */
    tree->right = (NP)((int)(tree->right)+(p->loc));
  else
    tree = AllocNode(Op, tree, (NP)(p->loc));   /* 節点の作成 */
  tree->type = p->type;
  return tree;
}

static int TempC;

int WriteNode(NP np)
{
  extern char *Scode[];
  int opc, t1, t2;
  char *ops;
  static char *Ccode[] = { "[ ]", "Dref", " &", " .", " ->", "arg" };

  if (np == NULL) return -1;
  switch (opc = np->Op) {
  case IDS: { STP p = SYMP(np);
      printf("%5d  %-8s %-16s%", TempC, "id", p->name); }
      break;
  case CONS:
      printf("%5d  ", TempC);
      if (np->type == INTP)
        printf("%-8s %2d %3c", "Int", (int)(np->left), ' ');
      else if (np->type == UNIVP)
        printf("%-8s %1d %14c", "null", 0, ' ');
      break;
  case MEMOP: case INMEM:
      t1 = WriteNode(np->left);
      printf("%5d  %-8s (%2d)", TempC, Ccode[opc-AELM], t1);
      printf("%8d %3c", (int)(np->right), ' ');
      break;
  default:
      t1 = WriteNode(np->left); t2 = WriteNode(np->right);
      ops = opc < AELM ? Scode[opc] : Ccode[opc-AELM];
      printf("%5d  %-8s (%2d)", TempC, ops, t1);
      if (t2 >= 0)
        printf(" %4c(%2d)   ", ' ', t2);
      else
        printf("%12c", ' ');
  }
  PrintType(np->type); printf(" \n");
  return TempC++;
}

void WriteTree(NP tree)
{
  extern int yylineno;

  TempC = 0;
  printf("\nLine %d\n", yylineno);
  WriteNode(tree);
  printf("\n");
}
