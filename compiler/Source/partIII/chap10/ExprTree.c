/* $B%W%m%0%i%`(B 11.4 : $B<0$NLZ$N:n@.!J(BExprTree.c$B!$(B238, 240, 242, 245, */
/*                                                  248, $B%Z!<%8!K(B */

#include <stdio.h>
#include <stdlib.h>
#include "SymTable.h"
#include "../VSME.h"
#include "ExprTree.h"

#define NUMCHK(X) { if ((X)->type < CHAR || (X)->type > DBL) \
                              yyerror("Illegal type of expression"); }
#define FMTCHK(X) { if ((X)->type != C_ARY) \
                              yyerror("Illegal format specification"); }
#define LHSCHK(X) { if ((X)->Op==IDS || (X)->Op==AELM) NUMCHK(X)\
                      else yyerror("Illegal L.H.S. expression"); }

#define DCT_SIZE 50
DCdesc  DCtbl[DCT_SIZE];                        /* $BDj?tI=(B */
static  int NoOfDC = 0;                         /* $BDj?tI=$N%(%s%H%j?t(B */

static Dtype ToSameType(NP *D1, NP *D2);        /* 2$B$D$N<0$rF1$87?$K(B */
static NP    ToDouble(NP np);                   /* double$B7?$X$NJQ49(B */


NP AllocNode(int Nid, NP lt, NP rt)             /* $B@aE@$N:n@.(B */
{
  NP np = (NP)malloc(sizeof(Node));             /* $BNN0h$N3MF@(B */

  np->Op = Nid; np->left = lt; np->right = rt;  /* $B1i;;;R!$%*%Z%i%s%I(B */
  return np;
}

void FreeSubT(NP np)                            /* $B<0$NLZ$N%a%b%j2rJ|(B */
{
  if (np != NULL) {
    if (np->Op != CONS && np->Op != IDS) {      /* $B@0?tDj?t$N@aE@(B ? */
     FreeSubT(np->left); FreeSubT(np->right); } /* $B:81&$NItJ,LZ$r2rJ|(B */
     free(np); }                                /* $B<+J,$NNN0h$r2rJ|(B */
}

NP MakeL(char *Name)                            /* $B<1JL;RMQ$N@aE@:n@.(B */
{
  STP sp = SymRef(Name);                        /* $B5-9fI=C5:w(B */
  NP np;

  np = AllocNode(IDS, (NP)sp, NULL);            /* $B@aE@$NNN0h3dIU$1(B */
  np->type = sp->type;                          /* $B7?$r3JG<(B */
  if (sp->class == VAR && sp->dim > 0)          /* $BG[Ns7?$NJQ?t(B ? */
    np->type |= ARY;                            /* $BG[NsI=<(%S%C%H@_Dj(B */
  return np;
}

NP MakeN(int Nid, NP lt, NP rt)                 /* $B1i;;MQ$N@aE@$N:n@.(B */
{
  NP np = AllocNode(Nid, lt, rt);               /* $B@aE@$NNN0h3MF@(B */
  STP sp;

  np->type = lt->type;                          /* $B1i;;7k2L$N7?$N@_Dj(B */
  switch (Nid) {
    case ASSGN :                                      /* $BBeF~1i;;;R(B */
      LHSCHK(lt); np->right = TypeConv(rt, lt->type); break;
    case ADD:  case SUB:  case MUL:  case DIV:        /* $B;MB'1i;;(B */
      np->type = ToSameType(&np->left, &np->right); break;
    case CSIGN:  NUMCHK(lt);                          /* $BC19`%^%$%J%9(B */
      if (lt->Op == CONS) {                           /* $B!]@0?tDj?t(B ? */
        if (lt->type == INT)  
          lt->left = (NP)(-INTV(lt));
        else if (lt->type == DBL)                     /* $B!]<B?tDj?t(B ? */
          lt->left = (NP)Wcons(-DCtbl[INTV(lt)].Dcons);
        free(np);
        return lt; }
      break;
    case INC:  case DEC:  LHSCHK(lt);                 /* ++, -- */
    case NOT:  case MOD:  case AND:  case OR:         /* !, %, &&, || */
      if (rt != NULL) INTCHK(rt);
      INTCHK(lt); np->type = INT; break;
    case COMP:                                        /* $B4X781i;;;R(B */
      ToSameType(&np->left, &np->right); np->type = INT; break;
    case AELM:  if (rt->Op != SUBL) INTCHK(rt);       /* $BG[NsMWAG(B */
      if (sp = SYMP(lt)) {
        np->etc = (rt->Op==SUBL)? rt->etc : 1;
        if (np->etc == sp->dim)
          np->type &= (~ARY);
        else if (np->etc > sp->dim) 
          yyerror("Too many subscripts or non-array"); }
      break;
    case SUBL:  INTCHK(rt)                            /* $BE:;z<0JB$S(B*/
      if (lt->Op != SUBL) INTCHK(lt);
    case ARGL:                                        /* $B<B0z?tJB$S(B */
      np->etc = (lt->Op==Nid)? lt->etc+1: 2; break;
    case CALL:                                        /* $B4X?t8F=P$7(B */ 
      if (sp = SYMP(lt)) { int n;
        n = (rt == NULL)? 0 : (rt->Op == ARGL)? rt->etc: 1;
        if (sp->Nparam != n) yyerror("Number of arguments unmatched");
        if (sp->class != FUNC && sp->class != F_PROT)
          yyerror("Trying to call non-funcation"); }
      break;
    case INPUT:                                      /* read(fmt, v) */
      FMTCHK(lt); LHSCHK(rt); np->type = INT; break;
    case OUTPUT:  NUMCHK(rt);                        /* write(fmt, e) */
    case OUTSTR:  FMTCHK(lt); np->type = VOID;       /* write(fmt) */ 
    }
  return np;
}

NP TypeConv(NP ep, Dtype To)                    /* $B7?JQ49(B */
{
  Dtype T = ep->type;

  if (T < CHAR || T> DBL)                       /* $B4pK\%G!<%?7?(B ? */
    yyerror("Illegal type conversion");
  switch (To) {
    case CHAR:  case INT:                       /* int$B7?$X$NJQ49(B */
      if (T == DBL) {                           /* $BJQ49BP>]$O(Bdouble? */
        if (ep->Op == CONS)                     /* $B<B?tDj?t$r(Bint$B$X(B? */
          ep->left = (NP)(int)DCtbl[INTV(ep)].Dcons;
        else
          ep = AllocNode(DBLINT, ep, NULL);     /* $B<0$NCM$r(Bint$B7?$X(B ? */
        ep->type = INT; }
      break;
    case DBL:                                   /* double$B7?$X$NJQ49(B */
      return (T==INT || T==CHAR)? ToDouble(ep): ep;
    default: yyerror("Illegal casting");
  }
  return ep;
}

static Dtype ToSameType(NP *D1, NP *D2)         /* 2$B$D$N<0$N7?$r(B */
{                                               /* $B$=$m$($k$?$a$NJQ49(B */
  Dtype T1, T2;

  if ((T1 = (*D1)->type) == CHAR) T1 = INT;     /* $BBh(B1$B%*%Z%i%s%I$N7?(B */
  if ((T2 = (*D2)->type) == CHAR) T2 = INT;     /* $BBh(B2$B%*%Z%i%s%I$N7?(B */
  if (T1 == VOID || T2 == VOID || T1 & ARY || T2 & ARY)
    yyerror("Incompatible type of binary operation");
  else if (T1 == INT && T2 == DBL)              /* (int) op (double) */
    *D1 = ToDouble(*D1);
  else if (T1 == DBL && T2 == INT)              /* (double) op (int) */
    *D2 = ToDouble(*D2);
  return T1 > T2 ? T1 : T2;                     /* $B1i;;7k2L$N7?(B */
}

static NP ToDouble(NP np)                       /* double$B7?$X$NJQ49(B */
{
  if (np->Op == CONS)                           /* $B@0?tDj?t(B ? */
    np->left = (NP)Wcons((double)INTV(np));
  else                                          /* $B$=$N$[$+$N>l9g(B */
    np = AllocNode(INTDBL, np, NULL);           /* $B6/@)7?JQ49$NA^F~(B */
  np->type = DBL;                               /* $BJQ498e$N7?$O(Bdouble */
  return np;
}

int Wcons(double C)                             /* $BDj?tI=$X$NEPO?(B */
{
  int i;

  DCtbl[NoOfDC].Dcons = C;                      /* $B%;%s%A%M%k$N@_Dj(B */
  for (i = 0; DCtbl[i].Dcons != C; i++) ;       /* $BDj?tI=$N@~7AC5:w(B */
  if (i >= NoOfDC)
    DCtbl[NoOfDC++].addr = -1;                  /* $B=i=P$N<B?tDj?t(B */
  return i;
}

static int TempC;                               /* $B0l;~JQ?t$N%+%&%s%?(B */

void WriteTree(NP root)                         /* $B<0$NLZ$N#3$DAHI=<((B */
{
  extern int yylineno;

  TempC = 0;
  printf("\nLine %d\n", yylineno);              /* $B9THV9f$NI=<((B */
  WriteNode(root);  printf("\n");
}

int WriteNode(NP np)                            /* $B@aE@$N#3$DAHI=<((B */
{
  int opc, t1, t2;
  char *ops;
  STP sp;
  static char *SymD[] = { "void", "char", "int", "double" },
              *Ccode[]= { "and", "or", "not", "++", "--", 
                          "[ ]", "subs", "arg"};
  extern char *Scode[];                         /* $B5-9fA`:n%3!<%II=(B */

  if (np == NULL) return -1;
  switch(opc = np->Op) {                        /* $B@aE@$N<1JL;R$O(B ? */
    case IDS:  sp = SYMP(np);                           /* $B<1JL;R(B */
      printf("%5d  %-10s ", TempC, "id");
      printf("%-10s%-12s\n", sp->name, SymD[sp->type]); break;
    case CONS:  printf("%5d  ", TempC);                 /* $BDj?t(B */
      if (np->type == INT)                              /* $B@0?tDj?t(B */
        printf("%-4s %7d\n", "Int", INTV(np));
      else if (np->type == DBL)                         /* $B<B?tDj?t(B */
        printf("%-8s %f\n", "Double", DCtbl[INTV(np)].Dcons);
      else if (np->type == C_ARY)                       /* $BJ8;zNs(B */
        printf("%-8s loc. %6d\n", "String", INTV(np));
      break;
    default:                                            /* $B1i;;;R(B */
      t1 = WriteNode(np->left); t2 = WriteNode(np->right);
      ops = (opc < AND) ? Scode[opc] : Ccode[opc-AND];  /* $BA`:n%3!<%I(B */
      printf("%5d  %-8s (%2d)", TempC, ops, t1);
      if (t2 >= 0) printf(" %5c (%2d)", ' ', t2);       /* $B#29`1i;;(B? */
      printf("\n"); }
  return TempC++;
}
