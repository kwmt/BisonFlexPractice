/* プログラム 9.1 : 記号表ヘッダファイル（SymTable.h，169ページ） */

typedef enum {NEWSYM, FUNC, PARAM, VAR, F_PROT} Class;
typedef enum {VOID, INT} Dtype;             /* 型を表す列挙定数 */

typedef struct STentry{                     /* 記号表エントリの構成 */
  unsigned char type, class;                  /* 型，種類 */
  unsigned char deflvl, Nparam;               /* 定義レベル，引数の数 */
  char *name;                                 /* 識別子へのポインタ */
  int   loc;                                  /* アドレス */
} STentry, *STP;                            /* 記号表へのポインタ型*/

void OpenBlock(void);                       /* ブロックの始まり */
void CloseBlock(void);                      /* ブロックの終り */

STP  MakeFuncEntry(char *Name);             /* 記号表への識別子の登録 */
void FuncDef(STP Funcp, Dtype T);           /* 関数定義名の登録 */
void Prototype(STP Funcp, Dtype T);         /* 関数プロトタイプの登録 */
void EndFdecl(STP Funcp);                   /* 関数定義の終り */
void VarDecl(char *Name, Dtype T, Class C); /* 記号表への変数名の登録 */
STP  SymRef (char *Name);                   /* 記号表の探索 */
void UndefCheck(void);                      /* 未定義関数のチェック */

void PrintSymEntry(STP Symp);               /* 記号表エントリの表示 */
