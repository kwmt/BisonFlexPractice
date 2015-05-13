/* $B%W%m%0%i%`(B 8.3 : switch$BJ8$H%i%Y%k$N=hM}!J(BLabelDef.c$B!$(B152$B!A(B153$B%Z!<%8!K(B */

#include "../VSM.h"

#define CT_SIZE 100                            /* $B%i%Y%kI=$NBg$-$5(B */
typedef struct {                               /* $B%(%s%H%j$NDj5A(B */
  int label;                                   /* case$B%i%Y%k(B */
  int addr;                                    /* $BBP1~$9$k%"%I%l%9(B */
} CSentry;
static CSentry Cstk[CT_SIZE], *CaseP = Cstk;   /* $B%i%Y%kI=$H$=$N(B */
                                               /* $B%]%$%s%?$NDj5A(B */
#define SWT_SIZE 10
struct {
  CSentry *CTptr;                              /* $B%i%Y%kI=$X$N%]%$%s%?(B */
  int     DefltAddr;                           /* default$B$N%"%I%l%9(B */
} SWtbl[SWT_SIZE];                             /* case$B%i%Y%k%9%?%C%/(B */
static int SWTptr = 0;                         /* $B$=$N%9%?%C%/%]%$%s%?(B */

void BeginSW(void)                             /* switch$BJ8$N3+;O(B */
{
  SWtbl[++SWTptr].CTptr = CaseP;
  SWtbl[SWTptr].DefltAddr = -1;                /* default$B%i%Y%k$r(B */
}                                              /* $BL$Dj5A$N>uBV$K(B  */

void CaseLbl(int Clabel)                       /* $B%i%Y%kDj5A$N=hM}(B */
{
  CSentry *p;

  if (SWTptr > 0) {
    CaseP->label = Clabel;                     /* $BFs=EDj5A(B ? */
    for (p = SWtbl[SWTptr].CTptr; p->label != Clabel; p++);
    if (p >= CaseP) {
      CaseP->addr = PC();                      /* Iseg$B%"%I%l%9$r3JG<(B */
      CaseP++; }
    else
      yyerror("Duplicated case constant");
  }
}

void DfltLbl(void)                             /* default$B%i%Y%k$N=hM}(B */
{
  if (SWTptr > 0 && SWtbl[SWTptr].DefltAddr < 0)  /* $BFs=EDj5A%A%'%C%/(B */
    SWtbl[SWTptr].DefltAddr = PC();           /* Iseg$B$N%"%I%l%9$r3JG<(B */
  else
    yyerror("Illegal default label");
}

void EndSW(void)                              /* switch$BJ8$N=*N;(B */
{
  CSentry *p;

  for (p = SWtbl[SWTptr].CTptr; p < CaseP; p++) {   /* $BJ,4tL?Na$N@8@.(B */
    Pout(COPY);   Cout(PUSHI, p->label);
    Pout(COMP);   Cout(BNE, PC()+3);
    Pout(REMOVE); Cout(JUMP, p->addr); }
  Pout(REMOVE);  
  if (SWtbl[SWTptr].DefltAddr > 0)            /* default$B%i%Y%k$,Dj5A(B? */
    Cout(JUMP, SWtbl[SWTptr].DefltAddr);      /* $B$=$3$X$N%8%c%s%WL?Na(B */
  CaseP = SWtbl[SWTptr--].CTptr;              /* $B%i%Y%kI=$N%]%$%s%?$r(B */
}                                             /* $BLa$9(B */ 
