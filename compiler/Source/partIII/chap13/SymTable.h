/* プログラム 13.1 : 記号表管理のヘッダファイル（SymTable.h，296ページ）*/

typedef enum { NEWSYM, FUNC, F_PROT, VAR, DTYPE } Class;

typedef struct STentry{                        /* 記号表のエントリ構成 */
  unsigned char    class, attrib, deflvl,      /* 種類，諸属性，レベル */
                   Nparam;                     /* 引数の個数 */
  char            *name;                       /* 識別子へのポインタ */
  struct STentry  *next, *h_chain, *plist;     /* ３つのリストポインタ */
  struct TypeDesc *type;                       /* 型リストへのポインタ */
  int              loc;                        /* 対応するアドレス */
} STentry, *STP;

#define PARAM  0x40                            /* 仮引数の表示 */

typedef enum { VOID, CHAR, INT, REF, ARRAY, STRUCT, INCMP } Dtype;

typedef struct TypeDesc{                       /* 型リストのセル */
  unsigned char    tcons, atoms;               /* 型構成子，境界調整数 */
  struct TypeDesc *compnt;                     /* 要素の型 */
  int              numelm, msize;              /* 要素数，メモリサイズ */
} TypeDesc, *TDP;

extern int GAptr, FAptr;                       /* メモリ割付けカウンタ */

#define ALIGN(X, Y) (((X+Y-1) / Y) * Y)
#define M_ALLOC(Loc, P, Sz, Bs) { P=ALIGN(P, Bs); Loc=P; P += (Sz); }

void OpenBlock(void);                          /* ブロックの始まり */
void CloseBlock(void);                         /* ブロックの終り */

STP  MakeEntry(char *Name, Class C);           /* 記号表エントリの作成 */
void TypeDef(char *Name, TDP Texpr);           /* 型定義 */
STP  MakeFuncEntry(char *Fname);               /* 関数名の仮登録 */
void FuncDef(STP Funcp, STP tp);               /* 関数本体の始まり */
void Prototype(STP Funcp, TDP Texpr);          /* プロトタイプ名の登録 */
void EndFdecl(STP Funcp);                      /* 関数定義の終り */
STP  NewID(char *Name, TDP Texpr);             /* 変数名の登録 */
STP  TypeName(char *Name);                     /* 型名の探索 */
void MemAlloc(STP Var, TDP Texpr, int Param);  /* メモリ割付け */
STP  SymRef (char *Name);                      /* 記号表の探索 */
int  AllocCons(char *ConsP, int Blen, int Nobj); /* 定数のメモリ割付け */
void UndefCheck(void);                         /* 未定義関数のチェック */

void BeginStruct(void);                        /* 構造体の始まり */
TDP  EndStruct(void);                          /* 構造体の終り */

void PrintSymEntry(STP p);                     /* 記号表エントリの表示 */
