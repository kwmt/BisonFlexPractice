/* $B%W%m%0%i%`(B 10.5 : $B5-9fI=$N4IM}!J(BSymTable.c$B!$(B214, 216, 217, 220, */
/*                                                  221, 222$B%Z!<%8!K(B */

#include <stdio.h>
#include "SymTable.h"
#include "../VSME.h"

int ByteW[] = { IBSZ, CBSZ, IBSZ, DBSZ };      /* $B3F7?$N%P%$%H?t(B */
int GAptr = 0,  FAptr = 0;                     /* $B%a%b%j3dIU$1%+%&%s%?(B */

static int Level = 0, MaxFSZ;                  /* $B%V%m%C%/%l%Y%k(B */
static char msg[50];                           /* $B%(%i!<%a%C%;!<%8MQ(B */

#define ALIGN(X, Y) (((X+Y-1) / Y) * Y)        /* $B6-3&D4@0(B */
#define M_ALLOC(Loc, P, Sz, Bs) { Loc = P = ALIGN(P, Bs); P += (Sz); }

#define SYMT_SIZE 200
static STentry SymTab[SYMT_SIZE];              /* $B5-9fI=(B */
static STP     SymTptr = &SymTab[0];           /* $B5-9fI=$X$N%]%$%s%?(B */

#define BT_SIZE 20
static struct {                                /* $B%V%m%C%/I=(B */
  int  allocp;                                    /* $B3dIU$1%+%&%s%?(B */
  STP  first;                                     /* $B5-9fI=%]%$%s%?(B */
} BLKtbl[BT_SIZE] = {{0, &SymTab[0]}};         /* $B%l%Y%k(B0$BMQ$N=i4|CM(B */

#define HT_SIZE 131
static  STP Htbl[HT_SIZE];                     /* $B5-9fI=MQ$N%O%C%7%eI=(B */
#define SHF(X) ((unsigned int)(X) % HT_SIZE)   /* $B5-9fI=MQ%O%C%7%e4X?t(B */

#define AST_SIZE 10
int AStable[AST_SIZE];                         /* $B3F<!85$NMWAG?t(B */

extern int SymPrintSW;                         /* $B5-9fI=I=<(%U%i%0(B */


void OpenBlock(void)                           /* $B%V%m%C%/$N;O$^$j(B */
{
  BLKtbl[++Level].first = SymTptr;             /* $B5-9fI=%]%$%s%?$rB`Hr(B */
  BLKtbl[Level].allocp = FAptr;                /* $B3dIU$1%]%$%s%?$rB`Hr(B */
}

void CloseBlock(void)                          /* $B%V%m%C%/$N=*$j(B */
{
  if (FAptr > MaxFSZ) MaxFSZ = FAptr;          /* $B%U%l!<%`$N:GBg%5%$%:(B */
  while (SymTptr > BLKtbl[Level].first) {      /* $B>WFM%j%9%H$N99?7(B */
    if ((--SymTptr)->dim > 0) free(SymTptr->dlist);
    Htbl[SHF(SymTptr->name)] = SymTptr->h_chain; }
  FAptr = BLKtbl[Level--].allocp;              /* $B3dIU$1%+%&%s%?$N2sI|(B */
}

static STP MakeEntry(char *Name, Dtype T, Class C)
{                                              /* $B5-9fI=%(%s%H%j$N:n@.(B */
  int h = SHF(Name);                           /* $B%O%C%7%eCM$N@_Dj(B */
  STP p;

  if ((p=SymTptr++) >= &SymTab[SYMT_SIZE]) {   /* $B%(%s%H%jMQNN0h$N3MF@(B */
    fprintf(stderr, "Too many symbols declared"); exit(-2); }
  p->type = T; p->class = C;
  p->dim = p->Nparam = 0; p->deflvl = Level;
  p->attrib = (Level == 0) ? SALLOC : 0;       /* $B@EE*NN0h(B/$B%U%l!<%`(B? */
  p->name = Name;
  p->h_chain = Htbl[h];                        /* $B%O%C%7%eI=$X$NA^F~(B */
  return Htbl[h] = p;
}

static STP LookUp(char *Name)                  /* $B<1JL;R$NC5:w(B */
{
  STP p = Htbl[SHF(Name)];                     /* $B%O%C%7%eCM$N7W;;(B */
                                               /* $B>WFM%j%9%H$NAv::(B */
  for (; p != NULL && p->name != Name; p = p->h_chain);
  return p;
}

STP MakeFuncEntry(char *Fname)                 /* $B4X?tL>$N%(%s%H%j:n@.(B */
{
  STP p;

  if ((p = LookUp(Fname)) == NULL)             /* $B=i=P$N4X?tL>(B ? */
    p = MakeEntry(Fname, VOID, NEWSYM);        /* $B5-9fI=%(%s%H%j$N:n@.(B */
  else if (p->class != F_PROT)                 /* $B@k8@:Q$_(B ? */
    yyerror("The function name alreay declared");
  if (SymPrintSW) {
    printf("\n");  PrintSymEntry(p); }
  OpenBlock();                                 /* $B?7$7$$M-8zHO0O$N@_Dj(B */
  FAptr = MaxFSZ = IBSZ;                       /* $B3dIU$1%+%&%s%?=i4|2=(B */
  return p;
}

void Prototype(STP Funcp, Dtype T)             /* $B%W%m%H%?%$%W$N4X?tL>(B */
{
  if (Level > 1)
    yyerror("The prototype declaration ignored");
  Funcp->class = F_PROT;  Funcp->type =  T & ~SALLOC;  Funcp->loc = -1;
  Funcp->Nparam = SymTptr - BLKtbl[Level].first;        /* $B0z?t$N8D?t(B */
  CloseBlock();                                /* $BM-8zHO0O$rJD$8$k(B */
  SymTptr += Funcp->Nparam;                    /* $B2>0z?t%(%s%H%j$N3NJ](B */
}

void FuncDef(STP Funcp, Dtype T)               /* $B4X?tDj5AL>(B */
{
  int n = SymTptr - BLKtbl[Level].first;       /* $B2>0z?t$N8D?t(B */

  T &= ~SALLOC;                                /* static$B$N;XDj$r>C$9(B */
  if (Funcp->class == F_PROT) { STP p, q;      /* $B@k8@:Q$_(B ? */
    if (Funcp->Nparam != n)
      yyerror("No. of parameter unmatched with the prototype");
    if (Funcp->type == T)
      for (p=Funcp+1, q=BLKtbl[Level].first; q < SymTptr; p++, q++) {
        if (p->type != q->type || (p->dim != q->dim) ||
            p->attrib != q->attrib)
          yyerror("parameter specification unmatched");
        p->loc = q->loc; }                     /* Dseg$B%"%I%l%9$r3JG<(B */
    else
      yyerror("Function type unmatched to the prototype");
    Bpatch(Funcp->loc, PC() + 3);              /* CALL$BL?Na$N8eKd$a(B */
    Funcp->attrib |= DUPFID;
  }
  else if (Funcp->class == NEWSYM)             /* $B4X?tL>$,=i=P$N>l9g(B */
    Funcp->type = T;                           /* $BJV5QCM$N7?$N@_Dj(B */
  else {                                       /* $BFs=EDj5A(B/$B@k8@$N>l9g(B */
    yyerror("The function already declared");
    return; }
  Funcp->class = FUNC;  Funcp->Nparam = n;     /* $B4X?tDj5AL>$NI=<((B */
  Funcp->loc = PC() + 3;                       /* $B%(%s%H%j%]%$%s%H@_Dj(B */
}

void EndFdecl(STP Funcp)                       /* $B4X?tDj5A$N=*$j(B */
{
  CloseBlock();                                /* $BM-8zHO0O$rJD$8$k(B */
  if ((Funcp->attrib & DUPFID) == 0)           /* $B2>0z?tJB$S$NJ]B8(B */
    SymTptr += Funcp->Nparam;
  Bpatch(Funcp->loc, ALIGN(MaxFSZ, DBSZ));     /* $B%U%l!<%`%5%$%:8eKd$a(B */
  if (SymPrintSW) PrintSymEntry(Funcp);
}

STP VarDecl(char *Name, int Dim)               /* $BJQ?t$N@k8@(B */
{
  int i, *dtp;
  STP p = LookUp(Name);                        /* $BJQ?tL>$N5-9fI=C5:w(B */

  if (p == NULL || p->deflvl < Level) {        /* $BFs=EDj5A$N%A%'%C%/(B */
    p = MakeEntry(Name, VOID, VAR);            /* $B5-9fI=%(%s%H%j:n@.(B */
    if ((p->dim = Dim) > 0){                   /* $BG[Ns(B ? */
      p->dlist = dtp = (int *)malloc(sizeof(int)*Dim);
      for (i = 0; i < Dim; i++, dtp++)         /* $B%5%$%:I=$N%3%T!<(B */
        if ((*dtp = AStable[i]) == 0)          /* $BMWAG?t$,(B0 ? */
          yyerror("array size is zero"); }
    else                                       /* $B%9%+%iJQ?t$N>l9g(B */
      p->dlist = NULL; }
  else
    yyerror("Duplicated declaration");
  return p;
}

void MemAlloc(STP Symptr, Dtype T, int Param)  /* $BJQ?tL>$N7?$NEPO?$H(B */
{                                              /* $B%a%b%j3dIU$1(B */
  int n, bs, sz, *p;

  if (T & SALLOC) {                            /* $B@EE*%a%b%j3dIU$1(B ? */
    T &= ~SALLOC; Symptr->attrib |= SALLOC; }  /* SALLOC$B%S%C%HIUBX$((B */
  if ((Symptr->type = T) == VOID)              /* void$B7?$NJQ?t@k8@(B ? */
    yyerror("Void is used as a type name");
  if (Param & PARAM && Symptr->dim > 0)        /* $BG[Ns7?$N0z?t(B ? */
    Param |= BYREF;
  if ((Symptr->attrib |= Param) & BYREF)       /* $B;2>HEO$7(B ? */
    bs = sz = IBSZ;
  else {
    bs = sz = ByteW[T];                        /* $B%P%$%H?t$N7W;;(B */
    for (n = Symptr->dim, p = Symptr->dlist; n > 0; n--, p++)
      if ((sz *= (*p)) <= 0)
        yyerror("Array size unspecified in the definition");
  }
  if ((Symptr->attrib) & SALLOC)
    M_ALLOC(Symptr->loc, GAptr, sz, bs)        /* $B@EE*NN0h$N3dIU$1(B */
  else
    M_ALLOC(Symptr->loc, FAptr, sz, bs)        /* $B%U%l!<%`$X$N3dIU$1(B */
  if (SymPrintSW) PrintSymEntry(Symptr);
}

int AllocCons(char * ConsP, int Blen, int Nobj) /* $BDj?t$N%a%b%j3dIU$1(B */
{
  int m, tsz = Blen*Nobj;

  M_ALLOC(m, GAptr, tsz, Blen)                 /* $B@EE*NN0h$N3NJ](B */
  WriteDseg(m, ConsP, tsz);                    /* Dseg$B$X$N=q9~$_(B */
  return m;
}

STP SymRef(char *Name)                         /* $B<0Cf$K8=$l$k<1JL;R(B */
{                                              /* $BMQ$N5-9fI=C5:w(B */
  STP p;

  if ((p = LookUp(Name)) == NULL) {            /* $B@k8@:Q$_(B ? */
    sprintf(msg, "Ref. to undeclared identifier %s", Name);
    yyerror(msg);
    p = MakeEntry(Name, INT, VAR); }           /* $BL$@k8@$NJQ?t$r2>EPO?(B */
  return p;
}

void UndefCheck(void)                          /* $BL$Dj5A4X?t$N%A%'%C%/(B */
{
  STP p;

  for (p = BLKtbl[0].first; p < SymTptr; p++)  /* $B%l%Y%k#0$G@k8@$5$l$?(B */
    if (p->class == F_PROT && p->loc > 0) {    /* $B%W%m%H%?%$%W$N8!::(B */
      sprintf(msg, "There's no definition of %s", p->name);
      yyerror(msg); }
}

void PrintSymEntry(STP Symp)                   /* $B5-9fI=%(%s%H%j$NI=<((B */
{
  static char *SymC[] = {"NewSym", "Func", "ProtF", "Var"},
              *SymD[] = {"void",   "char", "int",   "double"};

  printf("%*c%-10s", Level*4+2, ' ', Symp->name);
  printf(" %-6s %-6s", SymC[Symp->class], SymD[Symp->type]);
  switch (Symp->class) { int i;
		    case VAR:
      printf((Symp->attrib)&SALLOC ? "%5d  " : "%+5d  ", Symp->loc);
      if ((Symp->attrib) & PARAM)
        printf("%s  ", (Symp->attrib) & BYREF? "by-ref": "by-val");
      for (i = 0; i < Symp->dim; i++)
        printf((Symp->dlist)[i] < 0 ? "[*]": "[%d]", Symp->dlist[i]);
      break;
		    case FUNC:
      printf("%5d  Frame = %2d bytes", Symp->loc, MaxFSZ); }
  printf("\n");
}
