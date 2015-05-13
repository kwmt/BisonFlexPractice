/* $B%W%m%0%i%`(B 13.5 : $B5-9fI=$N4IM}!J(BSymTable.c$B!$(B310$B!A(B314$B%Z!<%8(B $B!K(B */

#include <stdio.h>
#include <stdlib.h>
#include "../VSME.h"
#include "SymTable.h"
#include "ExprTree.h"
#include "TypeDef.h"

int         GAptr = 1, FAptr = 0;              /* $B%a%b%j3dIU$1%+%&%s%?(B */
static int  Level = 0, MaxFSZ;                 /* $B%V%m%C%/%l%Y%k(B */
static char msg[50];                           /* $B%(%i!<%a%C%;!<%8MQ(B */

#define BT_SIZE 20
static struct {                                /* $B%V%m%C%/I=(B */
  int allocp;                                    /* $B3dIU$1%+%&%s%?(B */
  STP head;                                      /* $B5-9fI=%]%$%s%?(B */
} BLKtbl[BT_SIZE] = {{0, NULL}};

#define HT_SIZE 131
static  STP Htbl[HT_SIZE];
#define SHF(X) ((unsigned int)(X) % HT_SIZE)

extern int SymPrintSW;

void OpenBlock(void)                           /* $B%V%m%C%/$N;O$^$j(B */
{
  BLKtbl[++Level].head = NULL;                 /* $B5-9fI=%j%9%H$r6u$K(B */
  BLKtbl[Level].allocp = FAptr;                /* $B3dIU$1%]%$%s%?$rB`Hr(B */
}

void CloseBlock(void)                          /* $B%V%m%C%/$N=*$j(B */
{
  STP p;

  for (p = BLKtbl[Level].head; p != NULL; p = p->next) {
    Htbl[SHF(p->name)] = p->h_chain;              /* $B>WFM%j%9%H$N99?7(B */
    if (p->class == DTYPE && (p->type)->tcons == INCMP) {
      sprintf(msg, "Incomplete type name %s", p->name);
      yyerror(msg); }
  }
  if (FAptr > MaxFSZ)                          /* $B%U%l!<%`$N:GBg%5%$%:(B */
    MaxFSZ = FAptr;                            /* $B99?7(B */
  FAptr = BLKtbl[Level--].allocp;              /* $B3dIU$1%+%&%s%?$N2sI|(B */
}

STP MakeEntry(char *Name, Class C)             /* $B5-9fI=%(%s%H%j$N:n@.(B */
{
  STP p = (STP)calloc(1, sizeof(STentry));     /* $B%(%s%H%j$NNN0h3NJ](B */
  
  p->class = C; p->deflvl = Level;             /* $B<oN`$HDj5A%l%Y%k(B */
  p->name = Name;                              /* $B<1JL;R$X$N%]%$%s%?(B */
  p->next = BLKtbl[Level].head;                /* $B5-9fI=%j%9%H$XDI2C(B */
  p->h_chain = Htbl[SHF(Name)];                /* $B>WFM%j%9%H$X$NA^F~(B */
  Htbl[SHF(Name)] = BLKtbl[Level].head = p;
  return p;
}

static STP LookUp(char *Name)                  /* $B<1JL;R$NC5:w(B */
{
  STP p = Htbl[SHF(Name)];                     /* $B%O%C%7%eCM$N7W;;(B */

  for (; p != NULL && p->name != Name; p = p->h_chain);
  return p;
}

STP SymRef(char *Name)                         /* $B<0Cf$K8=$l$k<1JL;R(B */
{                                              /* $BMQ$N5-9fI=C5:w(B */
  STP p = LookUp(Name);

  if (p == NULL) {                             /* $B@k8@:Q$_(B ? */
    sprintf(msg, "Ref. to undeclared identifier %s", Name);
    yyerror(msg);
    p = MakeEntry(Name, VAR); p->type = INTP; } /* $BL$@k8@JQ?t$N2>EPO?(B */
  return p;
}

STP TypeName(char *Name)                       /* $B7?L>$N5-9fI=C5:w(B */
{
  STP p = LookUp(Name);

  if (p != NULL) {                             /* $B@k8@:Q$_$NL>A0(B ? */
    if (p->class == DTYPE) return p;           /* $B%G!<%?7?$NL>A0(B ? */
    yyerror("Invalid identifier used as a type name"); }
  p = MakeEntry(Name, DTYPE);                  /* $BIT40A47?L>$NEPO?(B */
  p->type = MakeTdesc(INCMP, NULL);            /* $BIT40A47?$N%;%k(B */
  return p;
}

STP NewID(char *Name, TDP Texpr)               /* $B5-9fI=$X$NJQ?tL>$N(B */
{                                              /* $BEPO?(B */
  STP p = LookUp(Name);

  if (p == NULL || p->deflvl < Level) {        /* $BFs=E@k8@$N%A%'%C%/(B */
    p = MakeEntry(Name, VAR); p->type = Texpr; }   /* $B%(%s%H%j$N:n@.(B */
  else
    yyerror("Duplicated declaration");
  return p;
}

STP MakeFuncEntry(char *Fname)                 /* $B4X?tL>$N%(%s%H%j:n@.(B */
{
  STP fp = LookUp(Fname);

  if (fp == NULL)                              /* $B=i=P$N4X?tL>(B ? */
    fp = MakeEntry(Fname, NEWSYM);             /* $B5-9fI=%(%s%H%j$r:n@.(B */
  if (SymPrintSW) { printf("\n"); PrintSymEntry(fp); }
  if (Level > 0)                               /* $BBg0h%l%Y%k$G$NDj5A(B? */
    yyerror("Local definition of function");
  OpenBlock();                                 /* $B?7$7$$M-8zHO0O$N@_Dj(B */
  FAptr = MaxFSZ = IBSZ;                       /* $B3dIU$1%+%&%s%?=i4|2=(B */
  return fp;
}

void Prototype(STP Funcp, TDP Texpr)           /* $B%W%m%H%?%$%W$N4X?tL>(B */
{
  if (Level > 1)
    yyerror("The prototype declaration ignored");
  if (Texpr->tcons == INCMP)
    yyerror("Incomplete data type is used");
  Funcp->class = F_PROT; Funcp->type = Texpr; Funcp->loc = -1;
  Funcp->plist = BLKtbl[Level].head;           /* $B2>0z?t%j%9%H$N5-21(B */
  CloseBlock();                                /* $BM-8zHO0O$rJD$8$k(B */
  if (SymPrintSW) PrintSymEntry(Funcp);
}

void FuncDef(STP Funcp, STP tp)                /* $B4X?tDj5AL>(B */
{
  if (Funcp->class == F_PROT) { STP p1, p2;    /* $B%W%m%H%?%$%W@k8@(B ? */
    if (Funcp->type == tp->type) {             /* $B4X?t@k8@;R$NHf3S(B */
      for (p1 = Funcp->plist, p2 = BLKtbl[Level].head;
                             p1 && p2; p1 = p1->next, p2 = p2->next)
        if (SameType(p1->type, p2->type) == 0)
          yyerror("Parameter specification unmatched");
      if (p1 != NULL || p2 != NULL)
        yyerror("No. of parameters unmatched to the prototype"); }
    else
      yyerror("Function type unmatched to the prototype");
    Bpatch(Funcp->loc, PC() + 3); }
  else if (Funcp->class == NEWSYM)             /* $B4X?tL>$,=i=P$N>l9g(B */
    Funcp->type = tp->type;                    /* $BJV5QCM$N7?$N@_Dj(B */
  else                                         /* $BFs=EDj5A(B/$B@k8@$N>l9g(B */
    yyerror("The function already declared");
  if ((tp->type)->tcons == INCMP)
    yyerror("Incomplete data type is used");
  Funcp->class = FUNC; Funcp->loc = PC() + 3;  /* $B4X?tDj5AL>$NI=<((B */
  Funcp->plist = BLKtbl[Level].head;           /* $B2>0z?t%j%9%H$N5-21(B */
}

void EndFdecl(STP Funcp)                       /* $B4X?tDj5A$N=*$j(B */
{
  CloseBlock();                                /* $BM-8zHO0O$rJD$8$k(B */
  if ((Funcp->type)->tcons != VOID)            /* void$B7?0J30$N4X?t(B ? */
    Cout(PUSHI, 0);                            /* $B2>$NJV5QCM$rJV$9(B */
  Cout(JUMP, Funcp->loc - 3);                  /* $B4X?t$N=P8}$X%8%c%s%W(B */
  Bpatch(Funcp->loc, ALIGN(MaxFSZ, DBSZ));     /* $B%U%l!<%`%5%$%:8eKd$a(B */
  if (SymPrintSW) PrintSymEntry(Funcp);
} 

void BeginStruct(void)                         /* $B9=B$BN$N;O$^$j(B */
{
  if (SymPrintSW)
    printf("\n%*cStruct defintion\n", 2*Level+2, ' ');
  OpenBlock();                                 /* $B?7$7$$M-8zHO0O$N@_Dj(B */
  FAptr = MaxFSZ = 0;                          /* $B%a%b%j3dIU$1%+%&%s%?(B */
}                                              /* $B$N=i4|@_Dj(B */

TDP EndStruct(void)                            /* $B9=B$BN$N=*$j(B */
{
  TDP tp;

  tp = MakeTdesc(STRUCT, (TDP)(BLKtbl[Level].head));  /* $B7?%;%k$N:n@.(B */
  tp->atoms = IBSZ; tp->msize = ALIGN(FAptr, IBSZ);   /* $B%5%$%:$N@_Dj(B */
  CloseBlock();                                       /* $BM-8zHO0O=*N;(B */
  return tp;
}

void TypeDef(char *Name, TDP Texpr)            /* $B7?Dj5A$N=hM}(B */
{
  STP sp = LookUp(Name);

  if (sp == NULL) {                            /* $B=i=P$N7?L>(B ? */
    sp = MakeEntry(Name, DTYPE); sp->type = Texpr; } /* $B5-9fI=$X$NEPO?(B */
  else if (sp->class == DTYPE && (sp->type)->tcons == INCMP)
    if (sp == (STP)Texpr->compnt || Texpr->tcons == INCMP)
      yyerror("Illegal reference to an incomplete name");
    else                                       /* $BIT40A47?$rDj5A$5$l$?(B */
      *(sp->type) = *Texpr;                    /* $B7?$XJQ99(B */
  else
    yyerror("Invalid identifier used as a type name");
  if (SymPrintSW) PrintSymEntry(sp);
}

void MemAlloc(STP Var, TDP Texpr, int Param)   /* $BJQ?t$N7?$NEPO?$H(B */
{                                              /* $B%a%b%j3dIU$1(B */
  int bs;

  if (Texpr->tcons == INCMP || Texpr->tcons == VOID) {
    yyerror("Incomplete type name or void is used for declraration");
    Texpr = INTP; }
  if (Var->type == NULL)                       /* $BG[Ns7?0J30$NJQ?t(B ? */
    Var->type = Texpr;                         /* $B7?$N@_Dj(B */
  else {                                       /* $BG[Ns7?$NJQ?t$N=hM}(B */
    ArrayDesc(Var->type, Texpr);               /* $BG[Ns7?%j%9%H$N@_Dj(B */
    Texpr = Var->type; }
  if ((bs = Texpr->msize) > 0)                 /* $B#0%P%$%H$NNN0h(B ? */
    if (Level > 0)                                /* $B6I=j%l%Y%k(B ? */
      M_ALLOC(Var->loc, FAptr, bs, Texpr->atoms)  /* $B@EE*NN0h$N3dIU$1(B */
    else 
      M_ALLOC(Var->loc, GAptr, bs, Texpr->atoms)  /* $B%U%l!<%`3dIU$1(B */
  else                                         /* $BNN0h$N%P%$%H?t$,#0(B */
    yyerror("No storage allocated");
  Var->attrib = Param;                         /* $B2>0z?t$NI=<($N3JG<(B */
  if (SymPrintSW) PrintSymEntry(Var);
}

int AllocCons(char *ConsP, int Blen, int Nobj) /* $BDj?t$N%a%b%j3dIU$1(B */
{
  int m, tsz = Blen*Nobj;

  M_ALLOC(m, GAptr, tsz, Blen)                 /* $B@EE*NN0h$N3NJ](B */
  WriteDseg(m, ConsP, tsz);                    /* Dseg$B$X$N=q9~$_(B */
  return m;
}

void UndefCheck(void)                          /* $BL$Dj5A4X?t$N%A%'%C%/(B */
{
  STP p;

  for (p = BLKtbl[0].head; p != NULL; p = p->next)
    if (p->class == F_PROT && p->loc > 0) {
      sprintf(msg, "There's no definition of %s", p->name);
      yyerror(msg); }
}

void PrintSymEntry(STP p)
{
  static char *SymC[] = { "NewSym", "Func", "ProtF", "Var", "Type" };

  printf("%*c%-10s%-6s", Level*2+2, ' ', p->name, SymC[p->class]);
  switch (p->class) {
  case VAR:
      printf(p->deflvl > 0 ? "%+5d  " : "%5d  ", p->loc);
      PrintType(p->type);
      if (p->attrib & PARAM) printf(", param");
      break;
    case FUNC: case F_PROT:
      printf("%4d  Frame = %2d bytes  ", p->loc, MaxFSZ);
      PrintType(p->type); break;
    case DTYPE:
      printf("= "); PrintType(p->type); break; }
  printf("\n");
}
