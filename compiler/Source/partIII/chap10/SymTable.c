/* プログラム 10.5 : 記号表の管理（SymTable.c，214, 216, 217, 220, */
/*                                                  221, 222ページ） */

#include <stdio.h>
#include "SymTable.h"
#include "../VSME.h"

int ByteW[] = { IBSZ, CBSZ, IBSZ, DBSZ };      /* 各型のバイト数 */
int GAptr = 0,  FAptr = 0;                     /* メモリ割付けカウンタ */

static int Level = 0, MaxFSZ;                  /* ブロックレベル */
static char msg[50];                           /* エラーメッセージ用 */

#define ALIGN(X, Y) (((X+Y-1) / Y) * Y)        /* 境界調整 */
#define M_ALLOC(Loc, P, Sz, Bs) { Loc = P = ALIGN(P, Bs); P += (Sz); }

#define SYMT_SIZE 200
static STentry SymTab[SYMT_SIZE];              /* 記号表 */
static STP     SymTptr = &SymTab[0];           /* 記号表へのポインタ */

#define BT_SIZE 20
static struct {                                /* ブロック表 */
  int  allocp;                                    /* 割付けカウンタ */
  STP  first;                                     /* 記号表ポインタ */
} BLKtbl[BT_SIZE] = {{0, &SymTab[0]}};         /* レベル0用の初期値 */

#define HT_SIZE 131
static  STP Htbl[HT_SIZE];                     /* 記号表用のハッシュ表 */
#define SHF(X) ((unsigned int)(X) % HT_SIZE)   /* 記号表用ハッシュ関数 */

#define AST_SIZE 10
int AStable[AST_SIZE];                         /* 各次元の要素数 */

extern int SymPrintSW;                         /* 記号表表示フラグ */


void OpenBlock(void)                           /* ブロックの始まり */
{
  BLKtbl[++Level].first = SymTptr;             /* 記号表ポインタを退避 */
  BLKtbl[Level].allocp = FAptr;                /* 割付けポインタを退避 */
}

void CloseBlock(void)                          /* ブロックの終り */
{
  if (FAptr > MaxFSZ) MaxFSZ = FAptr;          /* フレームの最大サイズ */
  while (SymTptr > BLKtbl[Level].first) {      /* 衝突リストの更新 */
    if ((--SymTptr)->dim > 0) free(SymTptr->dlist);
    Htbl[SHF(SymTptr->name)] = SymTptr->h_chain; }
  FAptr = BLKtbl[Level--].allocp;              /* 割付けカウンタの回復 */
}

static STP MakeEntry(char *Name, Dtype T, Class C)
{                                              /* 記号表エントリの作成 */
  int h = SHF(Name);                           /* ハッシュ値の設定 */
  STP p;

  if ((p=SymTptr++) >= &SymTab[SYMT_SIZE]) {   /* エントリ用領域の獲得 */
    fprintf(stderr, "Too many symbols declared"); exit(-2); }
  p->type = T; p->class = C;
  p->dim = p->Nparam = 0; p->deflvl = Level;
  p->attrib = (Level == 0) ? SALLOC : 0;       /* 静的領域/フレーム? */
  p->name = Name;
  p->h_chain = Htbl[h];                        /* ハッシュ表への挿入 */
  return Htbl[h] = p;
}

static STP LookUp(char *Name)                  /* 識別子の探索 */
{
  STP p = Htbl[SHF(Name)];                     /* ハッシュ値の計算 */
                                               /* 衝突リストの走査 */
  for (; p != NULL && p->name != Name; p = p->h_chain);
  return p;
}

STP MakeFuncEntry(char *Fname)                 /* 関数名のエントリ作成 */
{
  STP p;

  if ((p = LookUp(Fname)) == NULL)             /* 初出の関数名 ? */
    p = MakeEntry(Fname, VOID, NEWSYM);        /* 記号表エントリの作成 */
  else if (p->class != F_PROT)                 /* 宣言済み ? */
    yyerror("The function name alreay declared");
  if (SymPrintSW) {
    printf("\n");  PrintSymEntry(p); }
  OpenBlock();                                 /* 新しい有効範囲の設定 */
  FAptr = MaxFSZ = IBSZ;                       /* 割付けカウンタ初期化 */
  return p;
}

void Prototype(STP Funcp, Dtype T)             /* プロトタイプの関数名 */
{
  if (Level > 1)
    yyerror("The prototype declaration ignored");
  Funcp->class = F_PROT;  Funcp->type =  T & ~SALLOC;  Funcp->loc = -1;
  Funcp->Nparam = SymTptr - BLKtbl[Level].first;        /* 引数の個数 */
  CloseBlock();                                /* 有効範囲を閉じる */
  SymTptr += Funcp->Nparam;                    /* 仮引数エントリの確保 */
}

void FuncDef(STP Funcp, Dtype T)               /* 関数定義名 */
{
  int n = SymTptr - BLKtbl[Level].first;       /* 仮引数の個数 */

  T &= ~SALLOC;                                /* staticの指定を消す */
  if (Funcp->class == F_PROT) { STP p, q;      /* 宣言済み ? */
    if (Funcp->Nparam != n)
      yyerror("No. of parameter unmatched with the prototype");
    if (Funcp->type == T)
      for (p=Funcp+1, q=BLKtbl[Level].first; q < SymTptr; p++, q++) {
        if (p->type != q->type || (p->dim != q->dim) ||
            p->attrib != q->attrib)
          yyerror("parameter specification unmatched");
        p->loc = q->loc; }                     /* Dsegアドレスを格納 */
    else
      yyerror("Function type unmatched to the prototype");
    Bpatch(Funcp->loc, PC() + 3);              /* CALL命令の後埋め */
    Funcp->attrib |= DUPFID;
  }
  else if (Funcp->class == NEWSYM)             /* 関数名が初出の場合 */
    Funcp->type = T;                           /* 返却値の型の設定 */
  else {                                       /* 二重定義/宣言の場合 */
    yyerror("The function already declared");
    return; }
  Funcp->class = FUNC;  Funcp->Nparam = n;     /* 関数定義名の表示 */
  Funcp->loc = PC() + 3;                       /* エントリポイント設定 */
}

void EndFdecl(STP Funcp)                       /* 関数定義の終り */
{
  CloseBlock();                                /* 有効範囲を閉じる */
  if ((Funcp->attrib & DUPFID) == 0)           /* 仮引数並びの保存 */
    SymTptr += Funcp->Nparam;
  Bpatch(Funcp->loc, ALIGN(MaxFSZ, DBSZ));     /* フレームサイズ後埋め */
  if (SymPrintSW) PrintSymEntry(Funcp);
}

STP VarDecl(char *Name, int Dim)               /* 変数の宣言 */
{
  int i, *dtp;
  STP p = LookUp(Name);                        /* 変数名の記号表探索 */

  if (p == NULL || p->deflvl < Level) {        /* 二重定義のチェック */
    p = MakeEntry(Name, VOID, VAR);            /* 記号表エントリ作成 */
    if ((p->dim = Dim) > 0){                   /* 配列 ? */
      p->dlist = dtp = (int *)malloc(sizeof(int)*Dim);
      for (i = 0; i < Dim; i++, dtp++)         /* サイズ表のコピー */
        if ((*dtp = AStable[i]) == 0)          /* 要素数が0 ? */
          yyerror("array size is zero"); }
    else                                       /* スカラ変数の場合 */
      p->dlist = NULL; }
  else
    yyerror("Duplicated declaration");
  return p;
}

void MemAlloc(STP Symptr, Dtype T, int Param)  /* 変数名の型の登録と */
{                                              /* メモリ割付け */
  int n, bs, sz, *p;

  if (T & SALLOC) {                            /* 静的メモリ割付け ? */
    T &= ~SALLOC; Symptr->attrib |= SALLOC; }  /* SALLOCビット付替え */
  if ((Symptr->type = T) == VOID)              /* void型の変数宣言 ? */
    yyerror("Void is used as a type name");
  if (Param & PARAM && Symptr->dim > 0)        /* 配列型の引数 ? */
    Param |= BYREF;
  if ((Symptr->attrib |= Param) & BYREF)       /* 参照渡し ? */
    bs = sz = IBSZ;
  else {
    bs = sz = ByteW[T];                        /* バイト数の計算 */
    for (n = Symptr->dim, p = Symptr->dlist; n > 0; n--, p++)
      if ((sz *= (*p)) <= 0)
        yyerror("Array size unspecified in the definition");
  }
  if ((Symptr->attrib) & SALLOC)
    M_ALLOC(Symptr->loc, GAptr, sz, bs)        /* 静的領域の割付け */
  else
    M_ALLOC(Symptr->loc, FAptr, sz, bs)        /* フレームへの割付け */
  if (SymPrintSW) PrintSymEntry(Symptr);
}

int AllocCons(char * ConsP, int Blen, int Nobj) /* 定数のメモリ割付け */
{
  int m, tsz = Blen*Nobj;

  M_ALLOC(m, GAptr, tsz, Blen)                 /* 静的領域の確保 */
  WriteDseg(m, ConsP, tsz);                    /* Dsegへの書込み */
  return m;
}

STP SymRef(char *Name)                         /* 式中に現れる識別子 */
{                                              /* 用の記号表探索 */
  STP p;

  if ((p = LookUp(Name)) == NULL) {            /* 宣言済み ? */
    sprintf(msg, "Ref. to undeclared identifier %s", Name);
    yyerror(msg);
    p = MakeEntry(Name, INT, VAR); }           /* 未宣言の変数を仮登録 */
  return p;
}

void UndefCheck(void)                          /* 未定義関数のチェック */
{
  STP p;

  for (p = BLKtbl[0].first; p < SymTptr; p++)  /* レベル０で宣言された */
    if (p->class == F_PROT && p->loc > 0) {    /* プロトタイプの検査 */
      sprintf(msg, "There's no definition of %s", p->name);
      yyerror(msg); }
}

void PrintSymEntry(STP Symp)                   /* 記号表エントリの表示 */
{
  static char *SymC[] = {"NewSym", "Func", "ProtF", "Var"},
              *SymD[] = {"void",   "char", "int",   "double"};

  printf("%*c%-10s", Level*4+2, ' ', Symp->name);
  printf(" %-6s %-6s", SymC[Symp->class], SymD[Symp->type]);
  switch (Symp->class) { int i;
		    case VAR:
      printf((Symp->attrib)&SALLOC ? "%5d  " : "%+5d  ", Symp->loc);
      if ((Symp->attrib) & PARAM)
        printf("%s  ", (Symp->attrib) & BYREF? "by-ref": "by-val");
      for (i = 0; i < Symp->dim; i++)
        printf((Symp->dlist)[i] < 0 ? "[*]": "[%d]", Symp->dlist[i]);
      break;
		    case FUNC:
      printf("%5d  Frame = %2d bytes", Symp->loc, MaxFSZ); }
  printf("\n");
}
