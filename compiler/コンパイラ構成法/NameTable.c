/* プログラム 7.4 : 識別子の管理（NameTable.c，131ページ） */

#include <stdio.h>
#include <string.h>

typedef struct ID_entry {                       /* 名前表エントリ */
  char  *name_ptr;                              /* 文字列へのポインタ */
  int    len;                                   /* 長さ */
  struct ID_entry *next;                        /* 後続ポインタ */
  } ID_entry;

#define IDT_SIZE 113                            /* ハッシュ表の大きさ */
static ID_entry *IDtable[IDT_SIZE];             /* ハッシュ表の定義 */

static int hash(char *);

char *IDentry(char *sp, int len)                /* 名前表の探索と登録 */
{
  int hval = hash(sp);                          /* ハッシュ値の計算 */
  ID_entry *np;
 
  for (np=IDtable[hval]; np!=NULL; np=np->next)
    if ((np->len)==len && strcmp(np->name_ptr, sp)==0)
      return np->name_ptr;
  np = (ID_entry *)malloc(sizeof(ID_entry));    /* エントリ用領域獲得 */
  np->name_ptr = (char *)malloc(len+1);         /* 文字列用領域の獲得 */
  np->len = len;                                /* 長さの設定 */
  np->next = IDtable[hval];                     /* 衝突リストの先頭に */
  IDtable[hval] = np;                           /* 登録 */
  return strcpy(np->name_ptr, sp);              /* 文字列のコピー */
}

static int hash(char *sp)                       /* ハッシュ関数の定義 */
{
  unsigned h, g;

  for (h = 0; *sp != '\0'; sp++) {              /* 文字列を走査 */
    h = (h << 4) + (unsigned)(*sp);
    if (g = h&0xf0000000)                       /* MSBビットをLSBへ */
      h = (h ^ g>>24) ^ g;
  }
  return(h % IDT_SIZE);                         /* ハッシュ値を返す */
}
