/* $B%W%m%0%i%`(B 13.6 : $B7?%j%9%H$N:n@.!J(BTypeDef.c$B!$(B316, 317$B%Z!<%8!K(B */

#include <stdio.h>
#include "../VSME.h"
#include "SymTable.h"
#include "TypeDef.h"

char *BTname[] = {"void", "char", "int", "univ*"};
#define NoOfBT sizeof(BTname)/sizeof(BTname[0])
STP SymBTDP[NoOfBT];
TDP BasicTDP[NoOfBT];

static int ByteW[] = { IBSZ, CBSZ, IBSZ, IBSZ }; /* $B3F7?$N%P%$%H?t(B */


TDP MakeTdesc(Dtype Constrct, TDP Texpr)       /* $B7?%;%k$N@8@.(B */
{
  TDP tp = (TDP)calloc(1, sizeof(TypeDesc));   /* $B7?%;%k$NNN0h$N3NJ](B */

  tp->tcons = Constrct; tp->compnt = Texpr;    /* $B7?9=@.;R!$BP>]$N7?(B */
  return tp;
}

void SetUpSymTab(void)                         /* $B4pK\7?L>$N5-9fI=$X(B */
{                                              /* $B$NEPO?$H!$7?%;%k$N(B */
  Dtype i;                                     /* $B=i4|@_Dj(B */

  for (i = VOID; i < NoOfBT; i++) { TDP tp;
    SymBTDP[i] = MakeEntry(BTname[i], DTYPE);  /* $B5-9fI=$X$NEPO?(B */
    BasicTDP[i] = SymBTDP[i]->type = tp = MakeTdesc(i, NULL);
    tp->msize = tp->atoms = ByteW[i]; }
}

TDP RefType(TDP Texpr)                         /* REF$B7?%;%k$N@8@.(B */
{
  TDP tp = MakeTdesc(REF, Texpr);

  tp->msize = tp->atoms = ByteW[REF];          /* $B%P%$%H?t$H6-3&D4@0?t(B */
  return tp;                                   /* $B$N@_Dj(B */
}

void ArrayDesc(TDP Atp, TDP ElmnT)             /* $BMWAG7?$N@_Dj(B */
{
  if (Atp->compnt == NULL)                     /* $B7?%j%9%H$N:G8e(B ? */
    Atp->compnt = ElmnT;                       /* $BMWAG7?%j%9%H$N@_Dj(B */
  else {
    ArrayDesc(Atp->compnt, ElmnT);             /* $B2<0L$NMWAG7?$X$N@_Dj(B */
    ElmnT = Atp->compnt; }
  Atp->atoms = ElmnT->atoms;                   /* $B6-3&D4@0?t$N%3%T!<(B */
  Atp->msize = Atp->numelm < 0 ? 0 :  ElmnT->msize * Atp->numelm;
}

int SameType(TDP T1, TDP T2)                   /* $B7?$NF1CM@-$NH=Dj(B */
{
  if (T1->tcons != T2->tcons) return 0;        /* $B7?9=@.;R$NIT0lCW(B */
  switch (T1->tcons) {
    case REF:     return SameType(T1->compnt, T2->compnt);
    case ARRAY: { int n1 = T1->numelm, n2 = T2->numelm;
                  if (n1 > 0 && n2 > 0 && n1 != n2 )
                    yyerror("Array size unmatched");
                  return SameType(T1->compnt, T2->compnt); }
    case STRUCT:                               /* $B9=B$BN7?(B */
    case INCMP:   return T1 == T2; }           /* $BIT40A47?(B */
  return 1;                                    /* $BF1$8%9%+%i7?(B */
}

void CmptblType(TDP OrgT, TDP ActT)            /* $BBeF~$NE,9g@-$NH=Dj(B */
{
  switch (OrgT->tcons) {
    case CHAR:                                 /* $B4pK\7?$I$&$7(B */
    case INT:    if (ActT->tcons == CHAR || ActT->tcons == INT)
                   return;
                 break;
    case REF:    if (ActT == UNIVP) return;    /* NULL$B$H$N1i;;(B */
    case STRUCT:
    case ARRAY:  if (SameType(OrgT, ActT)) return; }
  yyerror("Incompatible type of operand");
}

void PrintType(TDP tp)
{
  static char *TypeS[] = { "void", "char", "int", "ref(", "[", "struct{",
                           "incomplete" };
  if (tp == NULL) return;
  printf("%s", TypeS[tp->tcons]);
  switch(tp->tcons) {
  case REF:    PrintType(tp->compnt); printf(")"); break;
  case ARRAY:  PrintType(tp->compnt);
               printf(tp->numelm < 0 ? ", *" : ", %d", tp->numelm);
               printf("]"); break;
  case STRUCT: printf("%d bytes(%d)}", tp->msize, tp->atoms); }
}
