/* プログラム 13.5 : 記号表の管理（SymTable.c，310〜314ページ ） */

#include <stdio.h>
#include <stdlib.h>
#include "../VSME.h"
#include "SymTable.h"
#include "ExprTree.h"
#include "TypeDef.h"

int         GAptr = 1, FAptr = 0;              /* メモリ割付けカウンタ */
static int  Level = 0, MaxFSZ;                 /* ブロックレベル */
static char msg[50];                           /* エラーメッセージ用 */

#define BT_SIZE 20
static struct {                                /* ブロック表 */
  int allocp;                                    /* 割付けカウンタ */
  STP head;                                      /* 記号表ポインタ */
} BLKtbl[BT_SIZE] = {{0, NULL}};

#define HT_SIZE 131
static  STP Htbl[HT_SIZE];
#define SHF(X) ((unsigned int)(X) % HT_SIZE)

extern int SymPrintSW;

void OpenBlock(void)                           /* ブロックの始まり */
{
  BLKtbl[++Level].head = NULL;                 /* 記号表リストを空に */
  BLKtbl[Level].allocp = FAptr;                /* 割付けポインタを退避 */
}

void CloseBlock(void)                          /* ブロックの終り */
{
  STP p;

  for (p = BLKtbl[Level].head; p != NULL; p = p->next) {
    Htbl[SHF(p->name)] = p->h_chain;              /* 衝突リストの更新 */
    if (p->class == DTYPE && (p->type)->tcons == INCMP) {
      sprintf(msg, "Incomplete type name %s", p->name);
      yyerror(msg); }
  }
  if (FAptr > MaxFSZ)                          /* フレームの最大サイズ */
    MaxFSZ = FAptr;                            /* 更新 */
  FAptr = BLKtbl[Level--].allocp;              /* 割付けカウンタの回復 */
}

STP MakeEntry(char *Name, Class C)             /* 記号表エントリの作成 */
{
  STP p = (STP)calloc(1, sizeof(STentry));     /* エントリの領域確保 */
  
  p->class = C; p->deflvl = Level;             /* 種類と定義レベル */
  p->name = Name;                              /* 識別子へのポインタ */
  p->next = BLKtbl[Level].head;                /* 記号表リストへ追加 */
  p->h_chain = Htbl[SHF(Name)];                /* 衝突リストへの挿入 */
  Htbl[SHF(Name)] = BLKtbl[Level].head = p;
  return p;
}

static STP LookUp(char *Name)                  /* 識別子の探索 */
{
  STP p = Htbl[SHF(Name)];                     /* ハッシュ値の計算 */

  for (; p != NULL && p->name != Name; p = p->h_chain);
  return p;
}

STP SymRef(char *Name)                         /* 式中に現れる識別子 */
{                                              /* 用の記号表探索 */
  STP p = LookUp(Name);

  if (p == NULL) {                             /* 宣言済み ? */
    sprintf(msg, "Ref. to undeclared identifier %s", Name);
    yyerror(msg);
    p = MakeEntry(Name, VAR); p->type = INTP; } /* 未宣言変数の仮登録 */
  return p;
}

STP TypeName(char *Name)                       /* 型名の記号表探索 */
{
  STP p = LookUp(Name);

  if (p != NULL) {                             /* 宣言済みの名前 ? */
    if (p->class == DTYPE) return p;           /* データ型の名前 ? */
    yyerror("Invalid identifier used as a type name"); }
  p = MakeEntry(Name, DTYPE);                  /* 不完全型名の登録 */
  p->type = MakeTdesc(INCMP, NULL);            /* 不完全型のセル */
  return p;
}

STP NewID(char *Name, TDP Texpr)               /* 記号表への変数名の */
{                                              /* 登録 */
  STP p = LookUp(Name);

  if (p == NULL || p->deflvl < Level) {        /* 二重宣言のチェック */
    p = MakeEntry(Name, VAR); p->type = Texpr; }   /* エントリの作成 */
  else
    yyerror("Duplicated declaration");
  return p;
}

STP MakeFuncEntry(char *Fname)                 /* 関数名のエントリ作成 */
{
  STP fp = LookUp(Fname);

  if (fp == NULL)                              /* 初出の関数名 ? */
    fp = MakeEntry(Fname, NEWSYM);             /* 記号表エントリを作成 */
  if (SymPrintSW) { printf("\n"); PrintSymEntry(fp); }
  if (Level > 0)                               /* 大域レベルでの定義? */
    yyerror("Local definition of function");
  OpenBlock();                                 /* 新しい有効範囲の設定 */
  FAptr = MaxFSZ = IBSZ;                       /* 割付けカウンタ初期化 */
  return fp;
}

void Prototype(STP Funcp, TDP Texpr)           /* プロトタイプの関数名 */
{
  if (Level > 1)
    yyerror("The prototype declaration ignored");
  if (Texpr->tcons == INCMP)
    yyerror("Incomplete data type is used");
  Funcp->class = F_PROT; Funcp->type = Texpr; Funcp->loc = -1;
  Funcp->plist = BLKtbl[Level].head;           /* 仮引数リストの記憶 */
  CloseBlock();                                /* 有効範囲を閉じる */
  if (SymPrintSW) PrintSymEntry(Funcp);
}

void FuncDef(STP Funcp, STP tp)                /* 関数定義名 */
{
  if (Funcp->class == F_PROT) { STP p1, p2;    /* プロトタイプ宣言 ? */
    if (Funcp->type == tp->type) {             /* 関数宣言子の比較 */
      for (p1 = Funcp->plist, p2 = BLKtbl[Level].head;
                             p1 && p2; p1 = p1->next, p2 = p2->next)
        if (SameType(p1->type, p2->type) == 0)
          yyerror("Parameter specification unmatched");
      if (p1 != NULL || p2 != NULL)
        yyerror("No. of parameters unmatched to the prototype"); }
    else
      yyerror("Function type unmatched to the prototype");
    Bpatch(Funcp->loc, PC() + 3); }
  else if (Funcp->class == NEWSYM)             /* 関数名が初出の場合 */
    Funcp->type = tp->type;                    /* 返却値の型の設定 */
  else                                         /* 二重定義/宣言の場合 */
    yyerror("The function already declared");
  if ((tp->type)->tcons == INCMP)
    yyerror("Incomplete data type is used");
  Funcp->class = FUNC; Funcp->loc = PC() + 3;  /* 関数定義名の表示 */
  Funcp->plist = BLKtbl[Level].head;           /* 仮引数リストの記憶 */
}

void EndFdecl(STP Funcp)                       /* 関数定義の終り */
{
  CloseBlock();                                /* 有効範囲を閉じる */
  if ((Funcp->type)->tcons != VOID)            /* void型以外の関数 ? */
    Cout(PUSHI, 0);                            /* 仮の返却値を返す */
  Cout(JUMP, Funcp->loc - 3);                  /* 関数の出口へジャンプ */
  Bpatch(Funcp->loc, ALIGN(MaxFSZ, DBSZ));     /* フレームサイズ後埋め */
  if (SymPrintSW) PrintSymEntry(Funcp);
} 

void BeginStruct(void)                         /* 構造体の始まり */
{
  if (SymPrintSW)
    printf("\n%*cStruct defintion\n", 2*Level+2, ' ');
  OpenBlock();                                 /* 新しい有効範囲の設定 */
  FAptr = MaxFSZ = 0;                          /* メモリ割付けカウンタ */
}                                              /* の初期設定 */

TDP EndStruct(void)                            /* 構造体の終り */
{
  TDP tp;

  tp = MakeTdesc(STRUCT, (TDP)(BLKtbl[Level].head));  /* 型セルの作成 */
  tp->atoms = IBSZ; tp->msize = ALIGN(FAptr, IBSZ);   /* サイズの設定 */
  CloseBlock();                                       /* 有効範囲終了 */
  return tp;
}

void TypeDef(char *Name, TDP Texpr)            /* 型定義の処理 */
{
  STP sp = LookUp(Name);

  if (sp == NULL) {                            /* 初出の型名 ? */
    sp = MakeEntry(Name, DTYPE); sp->type = Texpr; } /* 記号表への登録 */
  else if (sp->class == DTYPE && (sp->type)->tcons == INCMP)
    if (sp == (STP)Texpr->compnt || Texpr->tcons == INCMP)
      yyerror("Illegal reference to an incomplete name");
    else                                       /* 不完全型を定義された */
      *(sp->type) = *Texpr;                    /* 型へ変更 */
  else
    yyerror("Invalid identifier used as a type name");
  if (SymPrintSW) PrintSymEntry(sp);
}

void MemAlloc(STP Var, TDP Texpr, int Param)   /* 変数の型の登録と */
{                                              /* メモリ割付け */
  int bs;

  if (Texpr->tcons == INCMP || Texpr->tcons == VOID) {
    yyerror("Incomplete type name or void is used for declraration");
    Texpr = INTP; }
  if (Var->type == NULL)                       /* 配列型以外の変数 ? */
    Var->type = Texpr;                         /* 型の設定 */
  else {                                       /* 配列型の変数の処理 */
    ArrayDesc(Var->type, Texpr);               /* 配列型リストの設定 */
    Texpr = Var->type; }
  if ((bs = Texpr->msize) > 0)                 /* ０バイトの領域 ? */
    if (Level > 0)                                /* 局所レベル ? */
      M_ALLOC(Var->loc, FAptr, bs, Texpr->atoms)  /* 静的領域の割付け */
    else 
      M_ALLOC(Var->loc, GAptr, bs, Texpr->atoms)  /* フレーム割付け */
  else                                         /* 領域のバイト数が０ */
    yyerror("No storage allocated");
  Var->attrib = Param;                         /* 仮引数の表示の格納 */
  if (SymPrintSW) PrintSymEntry(Var);
}

int AllocCons(char *ConsP, int Blen, int Nobj) /* 定数のメモリ割付け */
{
  int m, tsz = Blen*Nobj;

  M_ALLOC(m, GAptr, tsz, Blen)                 /* 静的領域の確保 */
  WriteDseg(m, ConsP, tsz);                    /* Dsegへの書込み */
  return m;
}

void UndefCheck(void)                          /* 未定義関数のチェック */
{
  STP p;

  for (p = BLKtbl[0].head; p != NULL; p = p->next)
    if (p->class == F_PROT && p->loc > 0) {
      sprintf(msg, "There's no definition of %s", p->name);
      yyerror(msg); }
}

void PrintSymEntry(STP p)
{
  static char *SymC[] = { "NewSym", "Func", "ProtF", "Var", "Type" };

  printf("%*c%-10s%-6s", Level*2+2, ' ', p->name, SymC[p->class]);
  switch (p->class) {
  case VAR:
      printf(p->deflvl > 0 ? "%+5d  " : "%5d  ", p->loc);
      PrintType(p->type);
      if (p->attrib & PARAM) printf(", param");
      break;
    case FUNC: case F_PROT:
      printf("%4d  Frame = %2d bytes  ", p->loc, MaxFSZ);
      PrintType(p->type); break;
    case DTYPE:
      printf("= "); PrintType(p->type); break; }
  printf("\n");
}
