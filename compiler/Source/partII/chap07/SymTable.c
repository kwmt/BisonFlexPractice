/* $B%W%m%0%i%`(B 7.2 : $B5-9fI=$N4IM}!J(BSymTable.c$B!$(B126$B%Z!<%8!K(B */

#include <stdio.h>

#define ST_SIZE 100                           /* $B5-9fI=$NBg$-$5(B */
static char *SymTab[ST_SIZE];                 /* $B5-9fI=$NK\BN(B */
static int Last=0;                            /* $B5-9fI=$N%(%s%H%j?t(B */

void SymDecl(char *sptr)                      /* $B5-9fI=$X$NL>A0$NEPO?(B */
{
  int i;

  SymTab[Last+1] = sptr;                      /* $B%;%s%A%M%k$N@_Dj(B */
  for (i = 1; SymTab[i] != sptr; i++);        /* $BL>A0I=%]%$%s%?$NHf3S(B */
  if (i > Last)
    Last++;                                   /* $BL>A0$NEPO?(B */
  else   
    yyerror("Duplicated declaration");        /* $BFs=E$N@k8@(B */
}

int SymRef(char *sptr)                        /* $B5-9fI=$NC5:w(B */
{
  int i;

  SymTab[Last+1] = sptr;                      /* $B%;%s%A%M%k$N@_Dj(B */
  for (i = 1; SymTab[i] != sptr; i++);        /* $BL>A0I=%]%$%s%?$NHf3S(B */
  if (i > Last)
    yyerror("Undeclared identifier used");    /* $BL$@k8@$NL>A0$N;2>H(B */
  return i;                                   /* $B%(%s%H%j$NHV9f$rJV$9(B */
}
