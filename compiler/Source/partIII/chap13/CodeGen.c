/* プログラム 14.4 : コード生成（CodeGen.c，331, 333, 334, 338,
/*                                               341, 343 ページ） */

#include <stdio.h>
#include "../VSME.h"
#include "SymTable.h"
#include "ExprTree.h"
#include "TypeDef.h"

#define OPcode(Opc, texpr) ((texpr)->tcons == CHAR ? Opc-1 : Opc)
#define PUSH_LV(T)         { int lv = EvalLval(T); \
                             if (lv < 0) SetI(PUSHI, FP, -lv); \
                               else if (lv > 0) Cout(PUSHI, lv); }
#define EMIT(OP, AD)       { if (AD < 0) SetI(OP, FP, -AD); \
                               else if (AD > 0) Cout(OP, AD); }
enum { UNSAVE, SAVEV };

extern int StartP;

static void Cgen(NP tree, int Sv), GenCall(STP fp, NP arg, int Sv),
            GenArg(STP param, NP arg);
static int  EvalLval(NP tree), GenCond(NP tree);

void GenFuncEntry(STP func)                     /* 関数の出口と入口 */
{                                               /* のコード */
  STP pl;

  SetI(PUSH, FP, 0); Cout(INCFR, -1); Pout(RET);    /* 出口のコード */
  Cout(DECFR, PC()-2); SetI(POP, FP, 0);
  for (pl = func->plist; pl != NULL; pl = pl->next) /* 引数の引渡し */
    if (((pl->type)->tcons) >= ARRAY) { int n = (pl->type)->msize;
      SetI(PUSHI, FP, pl->loc);                     /* 構造体，配列の */
      if (n <= 0)                                   /* 値のコピー命令 */
        yyerror("Missing size information on parameter");
	Cout(PUSHI, n); Pout(MCOPY); }
    else
      SetI(OPcode(POP, pl->type), FP, pl->loc);     /* 値渡し */
}

void Epilogue(void)                             /* コンパイルの後処理 */
{
  STP sp;
  extern char *IDentry(char *Name, int len);

  StartP = PC();                                /* 実行開始アドレス */
  Cout(SETFR, MAX_DSEG);                        /* Fregの初期設定 */
  if (sp = SymRef(IDentry("main", 4))) {        /* "main"の探索 */
    if (sp->Nparam > 0)                         /* 引数のチェック */
      yyerror("Illegal parameter specification of main()");
    Cout(CALL, sp->loc); }                      /* main()の呼出し */
  Pout(HALT);                                   /* 実行終了命令の生成 */
  UndefCheck();                                 /* 未定義関数のチェック */
  CloseBlock();                                 /* 不完全型のチェック */
}

static void Cgen(NP tree, int Sv)               /* 式のコード生成 */
{
  int addr, temp;
  NP  lt = tree->left, rt = tree->right;        /* 左右の部分木 */
  TDP texpr = tree->type;                       /* 結果の型 */

  switch(tree->Op) {
    case ASSGN:                                 /* 代入式 */
      if (texpr->tcons >= ARRAY) {              /* 構造体，配列の代入 */
        Cgen(rt, rt->Op == CALL ? UNSAVE : SAVEV);      /* 右辺の評価 */
        if ((addr = EvalLval(lt)) == 0) {               /* 左辺の評価 */
          if (Sv == SAVEV) {
            M_ALLOC(temp, FAptr, IBSZ, IBSZ);
            Pout(COPY); SetI(POP, FP, temp); }
          Cout(PUSHI, texpr->msize); Pout(MCOPY);
          if (Sv == SAVEV) SetI(PUSH, FP, temp); }
        else {
          EMIT(PUSHI, addr); Cout(PUSHI, texpr->msize); Pout(MCOPY);
          if (Sv == SAVEV) EMIT(PUSHI, addr); }
      }
      else if (Sv == SAVEV) {                   /* スカラ型の代入演算 */
        PUSH_LV(lt); Cgen(rt, SAVEV); Pout(OPcode(ASSGN, texpr)); }
      else {                                    /* 式文のとき */
        addr = EvalLval(lt);                            /* 左辺の評価 */
        Cgen(rt, SAVEV);                                /* 右辺の評価 */
        if (addr == 0) {
          Pout(OPcode(ASSGN, texpr)); Pout(REMOVE); }
        else
          EMIT(OPcode(POP, texpr), addr); }             /* 直接代入 */
      break;
    case ADD: case SUB: case MUL: case DIV: case MOD:   /* 算術演算 */
      Cgen(lt, SAVEV); Cgen(rt, SAVEV); Pout(tree->Op); break;
    case CSIGN:                                         /* 単項− */
      Cgen(lt, SAVEV); Pout(CSIGN); break;
    case COMP: { int jaddr = GenCond(tree);             /* 関係演算 */
      Cout(PUSHI, 1); Cout(JUMP, PC()+2);
      Bpatch(jaddr, PC()); Cout(PUSHI, 0); break; }
    case CALL:                                          /* 関数呼出し */
      GenCall(SYMP(lt), rt, Sv); 
      break;
    case REFOBJ:                                        /* 間接演算 */
      Cgen(lt, SAVEV);
      if (texpr->tcons < ARRAY) Pout(OPcode(RVAL, texpr));
      break;
    case ADDROP:                                      /* アドレス演算 */
      PUSH_LV(lt); break;
    case AELM: case MEMOP: case INMEM: case IDS:      /* その他の演算 */
      if (texpr->tcons >= ARRAY)                      /* 配列，構造体 */
        PUSH_LV(tree)
      else { int addr = EvalLval(tree);               /* 左辺値の評価 */
        if (addr == 0)
          Pout(OPcode(RVAL, texpr));
        else
          EMIT(OPcode(PUSH, texpr), addr); }
      break;
    case CONS:                                        /* 整数定数 */
      Cout(PUSHI, (int)lt);
  }
}

static int EvalLval(NP tree)                    /* 左辺値を求める */
{                                               /* コードの生成 */
  int addr;
  NP  lt = tree->left, rt = tree->right;

  switch (tree->Op) {
    case IDS: { STP sp = SYMP(tree);                    /* 変数名 */
      if (sp->class != VAR)
        yyerror("Non-variable name used as an operand");
      return (sp->deflvl > 0) ? -(sp->loc) : sp->loc; }
    case AELM: { TDP tp = tree->type;                   /* 配列要素 */
      addr = (lt->Op == CALL) ? Cgen(lt, SAVEV), 0 : EvalLval(lt);
      if (rt->Op == CONS) { int of = (tp->msize)*(int)(rt->left);
        if (addr != 0)
          return (addr > 0) ? addr+of : addr-of;        /* 定数計算 */
        if (of > 1) Cout(ADDI, of); }                   /* 実行時計算 */
      else {
        EMIT(PUSHI, addr);
        Cgen(rt, SAVEV);
        if (tp->msize > 1) Cout(MULI, tp->msize);
        Pout(ADD); }
      break; }
    case REFOBJ: case INMEM:                            /* 間接演算 */
      Cgen(lt, SAVEV);                                  /* 間接メンバ */
      if (rt != NULL) Cout(ADDI, (int)rt);
      break;
    case MEMOP: { int of = (int)rt;                     /* X.メンバ */
      addr = (lt->Op == CALL) ? Cgen(lt, SAVEV), 0 : EvalLval(lt);
      if (addr != 0)
        return (addr > 0)? addr+of : addr-of;           /* 定数計算 */
      if (of > 0) Cout(ADDI, of);                       /* 実行時計算 */
      break; }
    default:
      yyerror("Illegal left-hand side value");
    }
  return 0;                                     /* 左辺値はスタック上 */
}

static void GenCall(STP fp, NP Arg, int Sv)     /* 関数呼出しの翻訳 */
{
  int as, temp;
  TDP tp = fp->type;                            /* 返却値の型リスト */

  GenArg(fp->plist, Arg);                       /* 実引数の評価コード */
  Cout(CALL, fp->loc);
  if (fp->class == F_PROT)                      /* 関数プロトタイプ */
    fp->loc = PC() - 1;                         /* 後埋めチェイン更新 */
  if (tp->tcons >= ARRAY && Sv == SAVEV) {      /* 配列，構造体の関数 */
    as = tp->msize;                             /* 返却値のバイト数 */
    M_ALLOC(temp, FAptr, as, tp->atoms);        /* その領域の確保 */
    SetI(PUSHI, FP, temp); Cout(PUSHI, as);
    Pout(MCOPY); SetI(PUSHI, FP, temp); }       /* コピー命令の生成 */
}

static void GenArg(STP param, NP arg)           /* 実引数並びの翻訳 */
{
  if (param != NULL && arg != NULL) {           /* 引数の個数不一致 ? */
    if (arg->Op == ARGL) {                      /* 実引数並び ? */
      GenArg(param->next, arg->left);
      arg = arg->right; }
    CmptblType(param->type, arg->type);         /* 型チェック */
    Cgen(arg, SAVEV); }                         /* 実引数の式の翻訳 */
}

void GenNewDel(int OP, NP tree)                 /* new()とdelete() */
{
  TDP texpr = tree->type;                       /* 実引数の型 */

  if (texpr->tcons != REF)                      /* ポインタ型 ? */
    yyerror("Non-reference type object");
  if (DebugSW) WriteTree(tree);
  if (OP == MALLOC) { int addr, sz;             /* new() */
    sz = (texpr->compnt)->msize;                /* 被参照型のバイト数 */
    if (sz <= 0) yyerror("Unknown size of object");
    addr = EvalLval(tree);                      /* 実引数の左辺値 */
    Cout(PUSHI, sz); Pout(MALLOC);
    if (addr == 0) {                            /* 獲得した領域の */
      Pout(ASSGN); Pout(REMOVE); }              /* アドレスの格納 */
    else 
      EMIT(POP, addr); }
  else {                                        /* delete() */
    Cgen(tree, SAVEV); Pout(MFREE); }
  FreeSubT(tree);                               /* 木のメモリを解放 */
}

void GenExpr(int Op, NP tree)                   /* 式文のコード生成 */
{
  int t = (tree->type)->tcons;                  /* 式文の型構成子 */

  if (DebugSW) WriteTree(tree);
  OpenBlock();                                  /* 仮のブロック開始 */
  if (Op == OUTPUT) {                           /* write文のとき */
    Cgen(tree, SAVEV); Pout(OUTPUT); 
    if (t == VOID || t >= ARRAY)
      yyerror("Illegal type of expression in write-statement"); }
  else {                                        /* 式文のとき */
    Cgen(tree, UNSAVE);
    if (tree->Op != ASSGN && t != VOID) Pout(REMOVE); }
  CloseBlock();                                 /* 仮のブロック終了 */
  FreeSubT(tree);                               /* 木のメモリを解放 */
}

int CtrlExpr(NP tree)                           /* 制御式の翻訳 */
{
  int jaddr;

  if (DebugSW) WriteTree(tree);
  OpenBlock();                                  /* 仮のブロック開始 */
  jaddr = GenCond(tree);                        /* 分岐コードの生成 */
  CloseBlock(); FreeSubT(tree);                 /* 仮のブロック終了*/
  return jaddr;                                 /* 分岐リストを返す */
}

void GenReturn(STP func, NP tree)               /* return文の翻訳 */
{
  TDP texpr = func->type;                       /* 関数の返却値の型 */

  if (texpr->tcons == VOID) {                   /* void型の関数 ? */
    if (tree != NULL)                           /* 返却値の指定あり ? */
      yyerror("Meaningless return value specification"); }
  else if (tree == NULL)                        /* 返却値の指定なし ? */
    yyerror("Missing return value");
  else
    CmptblType(texpr, tree->type);              /* 返却値の型チェック */
  if (tree) {
    if (DebugSW) WriteTree(tree);
    OpenBlock(); Cgen(tree, SAVEV); CloseBlock(); /* 返却値の式の翻訳 */
    FreeSubT(tree); }
  Cout(JUMP, (func->loc)-3);                    /* 出口へのジャンプ */
}

static int GenCond(NP tree)                     /* 分岐コードの生成 */
{
  static int NegC[] = { BGE, BGT, BNE, BEQ, BLT, BLE };

  if (tree->Op == COMP) { NP lt = tree->left, rt = tree->right;
    Cgen(lt, SAVEV);
    if (rt->Op != CONS || (int)(rt->left) != 0) {  /* ０との比較 ? */
      Cgen(rt, SAVEV);  Pout(COMP); }
    Cout(NegC[tree->etc - BLT], -1); }             /* 分岐命令の生成 */
  else { int t = (tree->type)->tcons;              /* 関係式以外の式 */
    if (t == VOID || t > REF)
      yyerror("Illegal type of control expression");
    Cgen(tree, SAVEV);  Cout(BEQ, -1); }           /* ０のとき分岐 */
  return PC()-1;                                   /* 分岐先リスト */
}
