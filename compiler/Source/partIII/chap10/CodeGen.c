/* プログラム 12.1 : コード生成関数（CodeGen.c，253, 256, 259, 261, */
/*                                    264, 268, 271, 272, 276ページ） */

#include <stdio.h>
#include "../VSME.h"
#include "SymTable.h"
#include "ExprTree.h"
#include "CodeGen.h"

#define OPC(X, T) ((X)+(T)-INT)
#define FPBIT(P)  ((P->attrib) & SALLOC ? 0 : FP)

extern char *IDentry(char *Name, int len);
extern int StartP, DebugSW;

static void Cgen(NP tree), PushLval(NP tree),
            GenArg(STP param, NP arg, int pnumber);
static int  GenCond(NP tree, int cond, int bpc), 
            GenAddrComp(STP Ap, NP subs);


void GenFuncEntry(STP func)                     /* 関数の出口と入口 */
{                                               /* のコード */
  STP pl;

  SetI(PUSH, FP, 0); Cout(INCFR, -1); Pout(RET);    /* 出口のコード */
  Cout(DECFR, PC()-2); SetI(POP, FP, 0);
  for (pl = func + func->Nparam; pl > func; pl--)   /* 引数の引渡し */
    if ((pl->attrib) & BYREF)
      SetI(POP, FP, pl->loc);                       /* 参照渡し */
    else
      SetI(OPC(POP, pl->type), FP, pl->loc);        /* 値渡し */
}

void ExprStmnt(NP tree)                         /* 式文の翻訳 */
{
  if (tree == NULL) return;
  if (DebugSW) WriteTree(tree);                 /* 式の木の３つ組表示 */
  if (tree->Op == ASSGN && (tree->left)->Op == IDS) { STP sp;
    sp = SYMP(tree->left);
    if (sp->attrib & BYREF) {                   /* 参照渡しの変数への */
      Cgen(tree); Pout(REMOVE); }               /* 代入 */
    else {                                      /* スカラ変数への代入 */
      Cgen(tree->right);
      SetI(OPC(POP, sp->type), FPBIT(sp), sp->loc); }
  }
  else {                                        /* 配列要素への代入 */
    Cgen(tree);
    if (tree->type != VOID) Pout(REMOVE); }
  FreeSubT(tree);                               /* 式の木のメモリ解放 */
}

int CtrlExpr(NP tree, int jc)                   /* 制御式の翻訳 */
{
  if (tree == NULL) return -1;
  if (DebugSW) WriteTree(tree);
  INTCHK(tree);                                 /* 整数型の式 ? */
  jc = GenCond(tree, jc, -1);                   /* 分岐コードへの翻訳 */
  FreeSubT(tree);
  return jc;                                    /* 分岐先リストを返す */
}

void GenReturn(STP func, NP tree)               /* return文の翻訳 */
{
  if (func->type == VOID) {                     /* void型の関数 ? */
    if (tree != NULL)
      yyerror("Meaningless return value"); }
  else if (tree == NULL)
    Cout(PUSHI, 0);                             /* 仮の返却値の生成 */
  else
    tree = TypeConv(tree, func->type);          /* 関数の型への変換 */
  if (tree) {
    if (DebugSW) WriteTree(tree);
    Cgen(tree); FreeSubT(tree); }               /* 返却値のコード生成 */
    Cout(JUMP, (func->loc)-3);                  /* 出口へのジャンプ */
}

void Epilogue(void)                             /* コンパイルの後処理 */
{
  STP sp;

  UndefCheck();                                 /* 未定義関数のチェック */
  StartP = PC();                                /* 実行開始アドレス */
  Cout(SETFR, MAX_DSEG);                        /* Fregの初期設定 */
  if (sp = SymRef(IDentry("main", 4))) {
    if (sp->Nparam > 0)
      yyerror("Parameter list specified at main()");
    Cout(CALL, sp->loc); }                      /* main()の呼出し */
  Pout(HALT);                                   /* 実行終了命令の生成 */
}

static void Cgen(NP tree)                       /* 式のコード生成 */
{
  int Nid = tree->Op,  T = tree->type, jc, n;   /* 節点id，結果の型 */
  NP  lt = tree->left, rt = tree->right;        /* 左右の部分木 */
  STP sp;

  switch (Nid) {
    case ASSGN:                                        /* 代入式 */
      PushLval(lt); Cgen(rt); break;
    case ADD:  case SUB:  case MUL:  case DIV:  case MOD:  /* 算術式 */
      Cgen(lt);
      if (T == INT && (rt->Op) == CONS) {
        Cout(Nid+2, INTV(rt)); return; }
      Cgen(rt); break;
    case INTDBL:               T = INT;                /* (double) */
    case CSIGN:  case DBLINT:  Cgen(lt); break;        /* -'，(int) */
    case INC:  case DEC: T = lt->type;                 /* ++, -- */
      if (lt->Op == IDS && !((sp = SYMP(lt))->attrib & BYREF)) {
        SetI(OPC(PUSH, T), FPBIT(sp), sp->loc);
        if (tree->etc == PRE) {                        /* 前置演算子 */
          Cout(Nid == INC ? ADDI : SUBI, 1);
          Pout(COPY); }
        else {                                         /* 後置演算子 */
          Pout(COPY);
          Cout(Nid == INC ? ADDI : SUBI, 1); }
        SetI(OPC(POP, T), FPBIT(sp), sp->loc); }
      else {
        PushLval(lt); Pout(COPY); Pout(OPC(RVAL, T));
        Cout(Nid==INC? ADDI: SUBI, 1); Pout(OPC(ASSGN, T));
        if (tree->etc == POST)
          Cout(Nid == INC? SUBI: ADDI, 1); }
      return;
    case AND:  case OR:  case NOT:  case COMP:       /* 論理関係演算 */
      jc = GenCond(tree, Nid == NOT? TJ : FJ, -1);
      Cout(PUSHI, 1);   Cout(JUMP, PC()+2);
      Bpatch(jc, PC()); Cout(PUSHI, 0); return;
    case AELM:                                       /* 配列要素 */
      PushLval(tree); Nid = RVAL; break;
    case CALL: sp = SYMP(lt);                        /* 関数呼出し */
      GenArg(sp + sp->Nparam, rt, sp->Nparam);
      Cout(CALL, sp->loc);
      if (sp->class == F_PROT)
        sp->loc = PC() - 1;
      return;
    case INPUT:  PushLval(lt); PushLval(rt); break;  /* 入出力 */
    case OUTPUT: PushLval(lt); Cgen(rt);     Pout(OUTPUT); return;
    case OUTSTR: PushLval(lt); Pout(OUTSTR); return;
    case CONS: n = (int)lt;                          /* 定数 */
      if (T == INT)                                  /* 整数定数 */
        Cout(PUSHI, n);
      else if (T == DBL) {                           /* 実数定数 */
        if (DCtbl[n].addr < 0)                       /* メモリ割付け ? */
          DCtbl[n].addr = ALLOCNUM(DCtbl[n].Dcons, DBSZ);
        Cout(DPUSH, DCtbl[n].addr); }
      return;
    case IDS: sp = (STP)lt;                          /* 変数 */
      if (sp->attrib & BYREF) {                      /* 参照渡し ? */
        SetI(PUSH, FP, sp->loc); Pout(OPC(RVAL, T)); }
      else
        SetI(OPC(PUSH, T), FPBIT(sp), sp->loc);
      return;
  }
  Pout(OPC(Nid, T));                            /* 演算命令の出力 */
}

static void PushLval(NP tree)                   /* 左辺値のための */
{                                               /* コード生成 */
  int n, m, offset;
  STP sp;

  switch (tree->Op) {
    case IDS: sp = SYMP(tree);                  /* スカラ変数 */
      if (sp->attrib & BYREF)
        SetI(PUSH, FP, sp->loc);
      else
        SetI(PUSHI, FPBIT(sp), sp->loc);
      return;
    case CONS: Cout(PUSHI, INTV(tree)); return; /* 文字列リテラル */
    case AELM: sp = SYMP(tree->left);           /* 配列要素 */
      offset = GenAddrComp(sp, tree->right);    /* オフセット計算 */
      for (m = ByteW[sp->type], n = tree->etc; n < sp->dim; n++)
        m *= (sp->dlist)[n];
      if (offset >= 0)                          /* オフセットは定数 ? */
        if (sp->attrib & BYREF) {
          SetI(PUSH, FP, sp->loc); Cout(ADDI, offset*m); }
        else
          SetI(PUSHI, FPBIT(sp), sp->loc + offset*m);
      else {                                    /* 実行時にオフセット */
        if (m > 1) Cout(MULI, m);               /* を計算する場合 */
        if (sp->attrib & BYREF) {               /* 基底アドレスの加算 */
          SetI(PUSH, FP, sp->loc); Pout(ADD); }
        else
          SetI(ADDI, FPBIT(sp), sp->loc);
      }
  }
}

static int GenAddrComp(STP Ap, NP subs)         /* オフセット計算用の */
{                                               /* コードの生成 */
  if (subs->Op == SUBL) { int m, n;             /* 多次元配列 ? */
    if ((m = GenAddrComp(Ap, subs->left)) >= 0) {   /* 左の並びの処理 */
      m *= (Ap->dlist)[subs->etc - 1];
      if ((n = GenAddrComp(Ap, subs->right)) >= 0)  /* 右側の添字式 */
        return m + n;                           /* 添字式並びが定数 */
      if (m > 0) Cout(ADDI, m); }
    else {
      Cout(MULI, (Ap->dlist)[subs->etc - 1]);   /* 要素数の乗算命令 */
      Cgen(subs->right); Pout(ADD); }           /* 添字式の値の加算 */
    }
  else if (subs->Op == CONS)                    /* 添字式は定数 ? */
    return INTV(subs);
  else                                          /* そのほかの式の場合 */
    Cgen(subs);
  return -1;
}

static void GenArg(STP param, NP arg, int pn)   /* 実引数並びの翻訳 */
{
  if (pn <= 0 || arg == NULL) return;           /* 引数の個数不一致 ? */
  if (arg->Op == ARGL) {                        /* 実引数並び ? */
    GenArg(param-1, arg->left, pn-1);
    arg = arg->right; }
  if (param->attrib & BYREF) {                  /* 参照渡し ? */
    if (param->type != ((arg->type)&(~ARY)))    /* 型チェック */
      yyerror("Argument type unmatched to parameter");
    PushLval(arg); }                            /* 左辺値の計算 */
  else {                                        /* 値渡しの引数 */
    arg = TypeConv(arg, param->type);           /* 仮引数の型への変換 */
    Cgen(arg); }                                /* 実引数の式の翻訳 */
}

static int GenCond(NP tree, int cond, int bpc)  /* 制御式のコード生成 */
{
  int temp, n, cc;
  NP  lt = tree->left, rt = tree->right;          /* 左右の部分木 */
  static int NegC[] = { BGE, BGT, BNE, BEQ, BLT, BLE };

  switch (tree->Op) {
    case AND:                                     /* 論理AND演算 */
      if (cond == TJ) {                           /* if(lt && rt) */
        temp = GenCond(lt, FJ, -1);
        bpc =  GenCond(rt, TJ, bpc);
        Bpatch(temp, PC()); }                     /* if(!lt)の後埋め */
      else {                                      /* if(!(lt && rt)) */
        bpc = GenCond(lt, FJ, bpc);
        bpc = GenCond(rt, FJ, bpc); }
      return bpc;
    case OR:                                      /* 論理OR演算 */
      if (cond == TJ) {                           /* if(lt || rt) */
        bpc = GenCond(lt, TJ, bpc);
        bpc = GenCond(rt, TJ, bpc); }
      else {                                      /* if(!(lt || rt)) */
        temp = GenCond(lt, TJ, -1);
        bpc =  GenCond(rt, FJ, bpc);
        Bpatch(temp, PC()); }                     /* if(lt)の後埋め */
      return bpc;
    case NOT:                                     /* 論理否定演算 */
      return GenCond(lt, cond == TJ ? FJ : TJ, bpc); 
    case COMP:                                    /* 関係演算 */
      Cgen(lt);
      if (rt->Op == CONS && rt->type == INT) {    /* 整数定数との比較 */
        if (INTV(rt) != 0) Cout(COMPI, INTV(rt)); }
      else {
        Cgen(rt); Pout(lt->type == DBL ? DCOMP : COMP); }
      cc = tree->etc;                                    /* 分岐条件 */
      Cout(cond == TJ ? cc : NegC[cc-BLT], bpc);         /* 分岐命令 */
      return PC()-1;
    case CONS:  n = (int)lt;                             /* 定数 */     
      if ((cond == TJ && n != 0 ) || (cond == FJ && n == 0)) {
        Cout(JUMP, bpc); bpc = PC()-1; }
      return bpc;
    default:                                          /* そのほかの式 */
      Cgen(tree); SetI(JUMP, cond == TJ ? BNE : BEQ, bpc);
      return PC()-1;
    }
}
