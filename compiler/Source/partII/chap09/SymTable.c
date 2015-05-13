/* $B%W%m%0%i%`(B 9.3 : $B5-9fI=$N4IM}!J(BSymTable.c$B!$(B181, 183, 185, 186$B%Z!<%8!K(B */

#include <stdio.h>
#include "SymTable.h"

static int Level = 0, Mallocp = 0, MaxFSZ;
static char msg[50];                            /* $B%(%i!<%a%C%;!<%8(B */

#define SYMT_SIZE 200
static STentry SymTab[SYMT_SIZE];               /* $B5-9fI=(B */
static STP     SymTptr = &SymTab[1];            /* $B5-9fI=$X$N%]%$%s%?(B */

#define BT_SIZE 20
static struct {                                 /* $B%V%m%C%/I=$NMWAG(B */
  int  allocp;
  STP  first;
} BLKtbl[BT_SIZE] = { {0, &SymTab[1]} };        /* $B%V%m%C%/I=$H=i4|CM(B */

extern int SymPrintSW;                          /* $B5-9fI=$NI=<(%U%i%0(B */


void OpenBlock(void)                            /* $B%V%m%C%/$N;O$^$j(B */
{
  BLKtbl[++Level].first = SymTptr;              /* $B5-9fI=%]%$%s%?B`Hr(B */
  BLKtbl[Level].allocp = Mallocp;               /* $B3dIU$1%+%&%s%?B`Hr(B */
}

void CloseBlock(void)                           /* $B%V%m%C%/$N=*$j(B */
{
  if (Mallocp > MaxFSZ) MaxFSZ = Mallocp;       /* $B%U%l!<%`%5%$%:99?7(B */
  SymTptr = BLKtbl[Level].first;                /* $B5-9fI=%]%$%s%?2sI|(B */
  Mallocp = BLKtbl[Level--].allocp;             /* $B3dIU$1%+%&%s%?2sI|(B */
}

static STP MakeEntry(char *Name, Dtype T, Class C)  /* $B<1JL;R$NEPO?(B */
{                                              
  STP p;

  if ((p=SymTptr++) >= &SymTab[SYMT_SIZE]) {    /* $B%(%s%H%j$NNN0h3NJ](B */
    fprintf(stderr, "Too many symbols declared");
    exit(-2); }
  p->type = T;      p->class = C;
  p->deflvl= Level; p->name = Name;
  return p;
}

static STP LookUp(char *Name)                   /* $B<1JL;R$NC5:w(B */
{
  STP p;

  for (p = SymTptr-1, SymTab[0].name = Name; p->name != Name; p--);
  return p > SymTab? p : NULL;
}

STP MakeFuncEntry(char *Fname)                  /* $B4V?tL>MQ$N(B */
{                                               /* $B%(%s%H%j$N:n@.(B */
  STP p;

  if ((p=LookUp(Fname)) == NULL)                /* $B=i=P$N4X?tL>(B ? */
    p = MakeEntry(Fname, VOID, NEWSYM);         /* $B5-9fI=%(%s%H%j:n@.(B */
  else if (p->class != F_PROT)                  /* $B@k8@:Q$_(B */
    yyerror("The Function name already declared");
  if (SymPrintSW) {
    printf("\n"); PrintSymEntry(p); }           /* $B%(%s%H%j$NFbMFI=<((B */
  Mallocp = MaxFSZ = 1;                         /* $B3dIU$1%+%&%s%?@_Dj(B */
  OpenBlock();                                  /* $BM-8zHO0O$N;O$^$j(B */
  return p;
}

void Prototype(STP Funcp, Dtype T)              /* $B%W%m%H%?%$%W@k8@$N(B */
{                                               /* $B4X?tL>$N=hM}(B */
  Funcp->class = F_PROT;  Funcp->type = T;
  Funcp->loc = -1;                              /* $B8eKd$a%A%'%$%s(B */
  Funcp->Nparam = SymTptr - BLKtbl[Level].first; /* $B0z?t$N8D?t(B */
  if (Level > 1)
    yyerror("The prototype declaration ignored");
  CloseBlock();                                 /* $BM-8zHO0O$N=*$j(B */
}

void FuncDef(STP Funcp, Dtype T)                /* $B4X?tDj5AL>(B */
{
  int n = SymTptr - BLKtbl[Level].first;        /* $B0z?t$N8D?t(B */
   
  if (Funcp->class == NEWSYM) {                 /* $B=i=P$N4X?tL>(B ? */
    Funcp->type = T;                            /* $B7?$H0z?t$N8D?t$r(B */
    Funcp->Nparam = n; }                        /* $B=q$-9~$`(B */
  else if (Funcp->class == F_PROT) {            /* $B@k8@:Q$_(B ? */
    if (Funcp->type != T)                       /* $BJV5QCM$OF1$87?(B ? */
      yyerror("Function type unmatched to the prototype");
    if (Funcp->Nparam != n)                     /* $B0z?t$N8D?t$bF1$8(B ? */
      yyerror("No. of parameters mismatched");
    Bpatch(Funcp->loc, PC()+3); }               /* $B8eKd$a(B */
  else {
    yyerror("The function already declared");
    return; }
  Funcp->class = FUNC;                           /* $B4X?tDj5AL>$NI=<((B */
  Funcp->loc = PC()+3;                           /* $BF~8}$N%"%I%l%9@_Dj(B */
}

void EndFdecl(STP Funcp)                        /* $B4X?tDj5A$N=*$j(B */
{
  CloseBlock();                                 /* $BM-8zHO0O$rJD$8$k(B */
  if (Funcp->class == FUNC)                     /* $B4X?tDj5A(B ? */
    Bpatch(Funcp->loc, MaxFSZ);                 /* $B%U%l!<%`%5%$%:@_Dj(B */
  if (SymPrintSW) PrintSymEntry(Funcp);
}

void VarDecl(char *Name, Dtype T, Class C)      /* $BJQ?t$N@k8@(B */
{
  STP p;

  SymTptr->name = Name;                         /* $BFs=E@k8@$N%A%'%C%/(B */
  for (p = BLKtbl[Level].first; p->name != Name; p++) ;
  if (p >= SymTptr) {
    if (T == VOID) {                            /* void$B7?$NJQ?t@k8@(B ? */
      yyerror("Void is used as a type name"); T = INT; }
    p = MakeEntry(Name, T, C);                  /* $B5-9fI=%(%s%H%j:n@.(B */
    p->loc = Mallocp++;                         /* $B%a%b%j3dIU$1(B */
    if (SymPrintSW) PrintSymEntry(p); }
  else
    yyerror("Duplicated declaration");
}

STP SymRef(char *Name)                          /* $B<0$K8=$l$k<1JL;R(B */
{                                               /* $B$N5-9fI=C5:w(B */
  STP p;

  if ((p=LookUp(Name)) == NULL) {               /* $B@k8@:Q$_(B ? */
    sprintf(msg, "Ref. to undeclared identifier %s", Name);
    yyerror(msg);
    p = MakeEntry(Name, INT, VAR); }            /* $BL$@k8@JQ?t$N2>EPO?(B */
  return p;
}

void UndefCheck(void)                           /* $BL$Dj5A4X?t$N8!::(B */
{
  STP p;

  if (Level > 0)
    yyerror("Block is not properly nested");
  for (p = &SymTab[1]; p < SymTptr; p++)        /* $B%V%m%C%/%l%Y%k(B0$B$N(B */
    if (p->type == F_PROT && p->loc > 0) {      /* $B4X?tL>$rAv::(B */
      sprintf(msg, "Undefined function %s is called", p->name);
      yyerror(msg); }
}

void PrintSymEntry(STP Symp)                    /* $B5-9fI=%(%s%H%j$N(B */
{                                               /* $BFbMF$rI=<((B */
  static char
    *SymC[] = {"NewSym", "Func", "Param", "Var", "P_Type"},
    *SymD[] = {"void", "int"};

  printf("%*c%-10s ", (Symp->deflvl)*2+2, ' ', Symp->name);
  printf("%-6s %-4s  ", SymC[Symp->class], SymD[Symp->type]);
  printf(Symp->deflvl? "%+5d\n": "%5d\n", Symp->loc);
}
