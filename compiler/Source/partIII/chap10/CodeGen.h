/* プログラム 11.2 : コード生成のヘッダファイル（CodeGen.h，232ページ） */

enum { FJ, TJ };                            /* 分岐条件 */
enum { PRE, POST };                         /* 前置，後置演算子 */

void  GenFuncEntry(STP FuncP);              /* 関数の出入口のコード */
void  ExprSmnt(NP tree);                    /* 式文のコード生成 */
int   CtrlExpr(NP tree, int jc);            /* 制御文のコード生成 */
void  GenReturn(STP func, NP tree);         /* return文のコード生成 */
void  Epilogue(void);                       /* コンパイル終了時の処理 */
