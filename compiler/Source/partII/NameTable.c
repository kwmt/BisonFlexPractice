/* $B%W%m%0%i%`(B 7.4 : $B<1JL;R$N4IM}!J(BNameTable.c$B!$(B131$B%Z!<%8!K(B */

#include <stdio.h>
#include <string.h>

typedef struct ID_entry {                       /* $BL>A0I=%(%s%H%j(B */
  char  *name_ptr;                              /* $BJ8;zNs$X$N%]%$%s%?(B */
  int    len;                                   /* $BD9$5(B */
  struct ID_entry *next;                        /* $B8eB3%]%$%s%?(B */
  } ID_entry;

#define IDT_SIZE 113                            /* $B%O%C%7%eI=$NBg$-$5(B */
static ID_entry *IDtable[IDT_SIZE];             /* $B%O%C%7%eI=$NDj5A(B */

static int hash(char *);

char *IDentry(char *sp, int len)                /* $BL>A0I=$NC5:w$HEPO?(B */
{
  int hval = hash(sp);                          /* $B%O%C%7%eCM$N7W;;(B */
  ID_entry *np;
 
  for (np=IDtable[hval]; np!=NULL; np=np->next)
    if ((np->len)==len && strcmp(np->name_ptr, sp)==0)
      return np->name_ptr;
  np = (ID_entry *)malloc(sizeof(ID_entry));    /* $B%(%s%H%jMQNN0h3MF@(B */
  np->name_ptr = (char *)malloc(len+1);         /* $BJ8;zNsMQNN0h$N3MF@(B */
  np->len = len;                                /* $BD9$5$N@_Dj(B */
  np->next = IDtable[hval];                     /* $B>WFM%j%9%H$N@hF,$K(B */
  IDtable[hval] = np;                           /* $BEPO?(B */
  return strcpy(np->name_ptr, sp);              /* $BJ8;zNs$N%3%T!<(B */
}

static int hash(char *sp)                       /* $B%O%C%7%e4X?t$NDj5A(B */
{
  unsigned h, g;

  for (h = 0; *sp != '\0'; sp++) {              /* $BJ8;zNs$rAv::(B */
    h = (h << 4) + (unsigned)(*sp);
    if (g = h&0xf0000000)                       /* MSB$B%S%C%H$r(BLSB$B$X(B */
      h = (h ^ g>>24) ^ g;
  }
  return(h % IDT_SIZE);                         /* $B%O%C%7%eCM$rJV$9(B */
}
