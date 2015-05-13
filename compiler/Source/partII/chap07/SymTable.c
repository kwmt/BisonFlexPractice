/* プログラム 7.2 : 記号表の管理（SymTable.c，126ページ） */

#include <stdio.h>

#define ST_SIZE 100                           /* 記号表の大きさ */
static char *SymTab[ST_SIZE];                 /* 記号表の本体 */
static int Last=0;                            /* 記号表のエントリ数 */

void SymDecl(char *sptr)                      /* 記号表への名前の登録 */
{
  int i;

  SymTab[Last+1] = sptr;                      /* センチネルの設定 */
  for (i = 1; SymTab[i] != sptr; i++);        /* 名前表ポインタの比較 */
  if (i > Last)
    Last++;                                   /* 名前の登録 */
  else   
    yyerror("Duplicated declaration");        /* 二重の宣言 */
}

int SymRef(char *sptr)                        /* 記号表の探索 */
{
  int i;

  SymTab[Last+1] = sptr;                      /* センチネルの設定 */
  for (i = 1; SymTab[i] != sptr; i++);        /* 名前表ポインタの比較 */
  if (i > Last)
    yyerror("Undeclared identifier used");    /* 未宣言の名前の参照 */
  return i;                                   /* エントリの番号を返す */
}
