/* プログラム 13.2 : 型リストのヘッダファイル（TypeDef.h，299ページ） */

extern STP  SymBTDP[];                     /* 基本型の記号表エントリ */
extern TDP  BasicTDP[];                    /* 基本型の型リストの表 */

#define VOIDP BasicTDP[VOID]                    /* voidの型リスト*/
#define CHARP BasicTDP[CHAR]                    /* charの型リスト*/
#define INTP  BasicTDP[INT]                     /* intの型リスト*/
#define UNIVP BasicTDP[REF]                     /* nullの型リスト*/

#define MAKE_ARRAY(P, N) { P = MakeTdesc(ARRAY, NULL); P->numelm = N; }

void SetUpSymTab(void);                         /* 記号表の初期設定 */
TDP  MakeTdesc(Dtype Constrct, TDP Texpr);      /* 型セルの生成 */
void ArrayDesc(TDP Atp, TDP ElmnT);             /* 配列型のセルの設定 */
TDP  RefType(TDP Texpr);                        /* ポインタ型 */
int  SameType(TDP T1, TDP T2);                  /* 同じ型の判定 */
void CmptblType(TDP OrgT, TDP ActT);            /* 代入適合性のテスト */
void PrintType(TDP tp);                         /* 型リストの表示 */
