/* プログラム 9.3 : 記号表の管理（SymTable.c，181, 183, 185, 186ページ） */

#include <stdio.h>
#include "SymTable.h"

static int Level = 0, Mallocp = 0, MaxFSZ;
static char msg[50];                            /* エラーメッセージ */

#define SYMT_SIZE 200
static STentry SymTab[SYMT_SIZE];               /* 記号表 */
static STP     SymTptr = &SymTab[1];            /* 記号表へのポインタ */

#define BT_SIZE 20
static struct {                                 /* ブロック表の要素 */
  int  allocp;
  STP  first;
} BLKtbl[BT_SIZE] = { {0, &SymTab[1]} };        /* ブロック表と初期値 */

extern int SymPrintSW;                          /* 記号表の表示フラグ */


void OpenBlock(void)                            /* ブロックの始まり */
{
  BLKtbl[++Level].first = SymTptr;              /* 記号表ポインタ退避 */
  BLKtbl[Level].allocp = Mallocp;               /* 割付けカウンタ退避 */
}

void CloseBlock(void)                           /* ブロックの終り */
{
  if (Mallocp > MaxFSZ) MaxFSZ = Mallocp;       /* フレームサイズ更新 */
  SymTptr = BLKtbl[Level].first;                /* 記号表ポインタ回復 */
  Mallocp = BLKtbl[Level--].allocp;             /* 割付けカウンタ回復 */
}

static STP MakeEntry(char *Name, Dtype T, Class C)  /* 識別子の登録 */
{                                              
  STP p;

  if ((p=SymTptr++) >= &SymTab[SYMT_SIZE]) {    /* エントリの領域確保 */
    fprintf(stderr, "Too many symbols declared");
    exit(-2); }
  p->type = T;      p->class = C;
  p->deflvl= Level; p->name = Name;
  return p;
}

static STP LookUp(char *Name)                   /* 識別子の探索 */
{
  STP p;

  for (p = SymTptr-1, SymTab[0].name = Name; p->name != Name; p--);
  return p > SymTab? p : NULL;
}

STP MakeFuncEntry(char *Fname)                  /* 間数名用の */
{                                               /* エントリの作成 */
  STP p;

  if ((p=LookUp(Fname)) == NULL)                /* 初出の関数名 ? */
    p = MakeEntry(Fname, VOID, NEWSYM);         /* 記号表エントリ作成 */
  else if (p->class != F_PROT)                  /* 宣言済み */
    yyerror("The Function name already declared");
  if (SymPrintSW) {
    printf("\n"); PrintSymEntry(p); }           /* エントリの内容表示 */
  Mallocp = MaxFSZ = 1;                         /* 割付けカウンタ設定 */
  OpenBlock();                                  /* 有効範囲の始まり */
  return p;
}

void Prototype(STP Funcp, Dtype T)              /* プロトタイプ宣言の */
{                                               /* 関数名の処理 */
  Funcp->class = F_PROT;  Funcp->type = T;
  Funcp->loc = -1;                              /* 後埋めチェイン */
  Funcp->Nparam = SymTptr - BLKtbl[Level].first; /* 引数の個数 */
  if (Level > 1)
    yyerror("The prototype declaration ignored");
  CloseBlock();                                 /* 有効範囲の終り */
}

void FuncDef(STP Funcp, Dtype T)                /* 関数定義名 */
{
  int n = SymTptr - BLKtbl[Level].first;        /* 引数の個数 */
   
  if (Funcp->class == NEWSYM) {                 /* 初出の関数名 ? */
    Funcp->type = T;                            /* 型と引数の個数を */
    Funcp->Nparam = n; }                        /* 書き込む */
  else if (Funcp->class == F_PROT) {            /* 宣言済み ? */
    if (Funcp->type != T)                       /* 返却値は同じ型 ? */
      yyerror("Function type unmatched to the prototype");
    if (Funcp->Nparam != n)                     /* 引数の個数も同じ ? */
      yyerror("No. of parameters mismatched");
    Bpatch(Funcp->loc, PC()+3); }               /* 後埋め */
  else {
    yyerror("The function already declared");
    return; }
  Funcp->class = FUNC;                           /* 関数定義名の表示 */
  Funcp->loc = PC()+3;                           /* 入口のアドレス設定 */
}

void EndFdecl(STP Funcp)                        /* 関数定義の終り */
{
  CloseBlock();                                 /* 有効範囲を閉じる */
  if (Funcp->class == FUNC)                     /* 関数定義 ? */
    Bpatch(Funcp->loc, MaxFSZ);                 /* フレームサイズ設定 */
  if (SymPrintSW) PrintSymEntry(Funcp);
}

void VarDecl(char *Name, Dtype T, Class C)      /* 変数の宣言 */
{
  STP p;

  SymTptr->name = Name;                         /* 二重宣言のチェック */
  for (p = BLKtbl[Level].first; p->name != Name; p++) ;
  if (p >= SymTptr) {
    if (T == VOID) {                            /* void型の変数宣言 ? */
      yyerror("Void is used as a type name"); T = INT; }
    p = MakeEntry(Name, T, C);                  /* 記号表エントリ作成 */
    p->loc = Mallocp++;                         /* メモリ割付け */
    if (SymPrintSW) PrintSymEntry(p); }
  else
    yyerror("Duplicated declaration");
}

STP SymRef(char *Name)                          /* 式に現れる識別子 */
{                                               /* の記号表探索 */
  STP p;

  if ((p=LookUp(Name)) == NULL) {               /* 宣言済み ? */
    sprintf(msg, "Ref. to undeclared identifier %s", Name);
    yyerror(msg);
    p = MakeEntry(Name, INT, VAR); }            /* 未宣言変数の仮登録 */
  return p;
}

void UndefCheck(void)                           /* 未定義関数の検査 */
{
  STP p;

  if (Level > 0)
    yyerror("Block is not properly nested");
  for (p = &SymTab[1]; p < SymTptr; p++)        /* ブロックレベル0の */
    if (p->type == F_PROT && p->loc > 0) {      /* 関数名を走査 */
      sprintf(msg, "Undefined function %s is called", p->name);
      yyerror(msg); }
}

void PrintSymEntry(STP Symp)                    /* 記号表エントリの */
{                                               /* 内容を表示 */
  static char
    *SymC[] = {"NewSym", "Func", "Param", "Var", "P_Type"},
    *SymD[] = {"void", "int"};

  printf("%*c%-10s ", (Symp->deflvl)*2+2, ' ', Symp->name);
  printf("%-6s %-4s  ", SymC[Symp->class], SymD[Symp->type]);
  printf(Symp->deflvl? "%+5d\n": "%5d\n", Symp->loc);
}
