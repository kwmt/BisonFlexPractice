/* プログラム 11.1 : 式の木のヘッダファイル（ExprTree.h，229ページ） */

enum { AND=100, OR, NOT, INC, DEC, AELM, SUBL, ARGL, CONS, IDS };

typedef struct Node {                       /* 式の木の節点の定義 */
  unsigned char Op, type, etc;              /* 演算子，型，属性 */
  struct Node *left, *right;                /* 左右の部分木のポインタ */
} Node, *NP;                                /* 節点と節点のポインタ型 */

typedef struct {                            /* 定数表エントリ */
  double Dcons;                             /* 値 */
  int    addr;                              /* Dsegアドレス */
} DCdesc;

extern DCdesc DCtbl[];                      /* 定数表 */

#define INTV(P) (int)((P)->left)            /* int型への変換 */
#define SYMP(P) (STP)((P)->left)            /* 記号表へのポインタ */
#define MakeC(X, T, D) { (X) = AllocNode(CONS, (NP)(D), NULL);\
                            (X)->type = T; }
#define INTCHK(X) { if ((X->type) != INT && (X->type) != CHAR)\
                              yyerror("Non-integer type expression"); }
#define VAR_NODE(X, Y) { X = MakeL(Y);  \
           if ((SYMP(X))->class != VAR) yyerror("Non-variable name"); }

NP   AllocNode(int Op, NP left, NP right);  /* 節点用のメモリの獲得 */
NP   MakeL(char *Name);                     /* 葉の作成 */
NP   MakeN(int Op, NP left, NP right);      /* 節点の作成 */
NP   TypeConv(NP tree, Dtype T);            /* 型変換 */
int  Wcons(double value);                   /* 定数表への登録 */
void WriteTree(Node *root);                 /* 式の木の表示 */
void FreeSubT(NP tree);                     /* 式の木のメモリ開放 */
