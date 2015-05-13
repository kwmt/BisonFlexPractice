/* プログラム 10.2 : 記号表管理のヘッダファイル（SymTable.h，204ページ） */

typedef enum { NEWSYM, FUNC, F_PROT, VAR } Class;
typedef enum { VOID, CHAR, INT, DBL, ARY=0x10, C_ARY, } Dtype;

typedef struct STentry{                     /* 記号表のエントリ構成 */
  unsigned char type,   class,                 /* 型，種類 */
                attrib, deflvl,                /* 諸属性，定義レベル */
                dim,    Nparam;                /* 次元数，引数の個数 */
  char         *name;                          /* 識別子へのポインタ */
  int           loc, *dlist;                   /* アドレス，*サイズ表 */
  struct STentry *h_chain;                     /* 衝突リストのポインタ */
} STentry, *STP;

#define SALLOC 0x80                         /* 静的割付けの表示 */
#define PARAM  0x40                         /* 仮引数の表示 */
#define BYREF  0x20                         /* 参照渡しの表示 */
#define DUPFID 0x10                         /* 関数名の宣言と定義 */

#define ALLOCNUM(C, T) AllocCons((char *)(&C), T, 1)  /* 数値の割付け */

extern int ByteW[], AStable[];              /* バイト数表，各要素数 */

void OpenBlock(void);                       /* ブロックの始まり */
void CloseBlock(void);                      /* ブロックの終り */

STP  MakeFuncEntry(char *Fname);            /* 関数名の仮登録 */
void FuncDef(STP Funcp, Dtype T);           /* 関数定義名の登録 */
void Prototype(STP Funcp, Dtype T);         /* 関数プロトタイプの登録 */
void EndFdecl(STP Funcp);                   /* 関数定義の終り */
STP  VarDecl(char *Name, int Dim);          /* 記号表への変数名の登録 */
void MemAlloc(STP Symp, Dtype T, int Parm); /* メモリ割付け */
STP  SymRef (char *Name);                   /* 記号表の探索 */
int  AllocCons(char *cp, int bl, int Nobj); /* 定数のメモリ割付け */

void UndefCheck(void);                      /* 未定義関数のチェック */
void PrintSymEntry(STP Symp);               /* 記号表エントリの表示 */
