/* $B%W%m%0%i%`(B 14.3 : $B<0$NLZ$N:n@.!J(BExprTree.c$B!$(B324, 325, 328$B%Z!<%8!K(B */

#include <stdlib.h>
#include "../VSME.h"
#include "SymTable.h"
#include "ExprTree.h"
#include "TypeDef.h"


NP AllocNode(int op, NP lt, NP rt)              /* $B@aE@$N:n@.(B */
{
  NP np = (NP)malloc(sizeof(Node));             /* $BNN0h$N3MF@(B */

  np->Op = op; np->left = lt; np->right = rt;   /* $B1i;;;R!$%*%Z%i%s%I(B */
  return np;
}

void FreeSubT(NP np)                            /* $B<0$NLZ$N%a%b%j2rJ|(B */
{
  if (np != NULL) {
    switch (np->Op) {
      case CONS:  case IDS:  break;             /* $BDj?t!$<1JL;R(B ? */
      case CSIGN: case REFOBJ: case ADDROP:
      case MEMOP: case INMEM:      FreeSubT(np->left); break;
      default: FreeSubT(np->left); FreeSubT(np->right); }
    free(np); }                                 /* $B<+J,$NNN0h$r2rJ|(B */
}

NP MakeL(char *Name)                            /* $B<1JL;RMQ$N@aE@:n@.(B */
{
  STP sp = SymRef(Name);                        /* $B5-9fI=C5:w(B */
  NP np;

  np = AllocNode(IDS, (NP)sp, NULL);            /* $B@aE@$NNN0h3dIU$1(B */
  np->type = sp->type;                          /* $B7?%j%9%H$N@_Dj(B */
  return np;
}

NP MakeN(int Nid, NP lt, NP rt)                 /* $B1i;;MQ$N@aE@$N:n@.(B */
{
  int t1 = (lt->type)->tcons,                   /* $BBh#1%*%Z%i%s%I$N7?(B */
      t2 = rt ? (rt->type)->tcons : INT;        /* $BBh#2%*%Z%i%s%I$N7?(B */
  NP np = AllocNode(Nid, lt, rt);               /* $B@aE@$NNN0h3MF@(B */

  np->type = lt->type;                          /* $B1i;;7k2L$N7?$N@_Dj(B */
  switch (Nid) {
    case ASSGN:                                       /* $BBeF~1i;;;R(B */
      switch(lt->Op) {                                /* $B:8JU$N8!::(B */
        case REFOBJ: case AELM: case MEMOP: case INMEM: case IDS:
          CmptblType(lt->type, rt->type); break;
        default: yyerror("Illegal left-hand side expression"); }
      break;
    case ADD: case SUB: case MUL: case DIV: case MOD: case CSIGN:
      if (t1 == VOID || t1 > INT || t2 == VOID || t2 >INT)
        yyerror("Imcompatible type operation");
      np->type = INTP; break;
    case COMP:                                        /* $B4X781i;;;R(B */
      if (t1 == VOID || t1 >= ARRAY || t2 == VOID || t2 >= ARRAY)
        yyerror("Imcompatible type operand in relational expression");
      else if (lt->type != UNIVP || t2 != REF)        /* $B%]%$%s%?(B ? */
        CmptblType(lt->type, rt->type);               /* $B7?%A%'%C%/(B */
      np->type = INTP; break;
    case REFOBJ:                                      /* $B4V@\1i;;;R(B */
      if (t1 == REF && lt->type != UNIVP)
        np->type = (np->type)->compnt;                /* $BHo;2>H7?(B */
      else
        yyerror("Reference to non-pointer type object or null value");
      break;
    case ADDROP:                                      /* $B!u1i;;;R(B */
      np->type = RefType(lt->type);
      switch (lt->Op) {                               /* $B:8JUCM(B ? */
        case IDS: case AELM: case MEMOP: case INMEM: break;
        default: yyerror("Illegal operand of &-operator"); }
      break;
    case AELM:                                        /* $BG[NsMWAG(B */
      if (t2 == VOID || t2 > INT)
        yyerror("Non-integer expression used as subscript");
      if (t1 == ARRAY)                                /* $BG[Ns7?(B ? */
        np->type = (lt->type)->compnt;
      else
        yyerror("Subscript specified to non-array");
      break;
    case ARGL:                                        /* $B<B0z?tJB$S(B */
      np->etc = (lt->Op == ARGL)? lt->etc + 1: 2;
      break;
    case CALL: { STP sp; int n;                       /* $B4X?t8F=P$7(B */
      sp = SYMP(lt);
      n = (rt == NULL) ? 0 : (rt->Op == ARGL) ? rt->etc : 1;
      if (sp->Nparam != n) {                          /* $B0z?t$N8D?t(B */
        yyerror("Number of arguments unmatched");
        np->right = NULL; }
      if (sp->class != FUNC && sp->class != F_PROT)
        yyerror("Trying to call non-funcation"); }
      break;
  }
  return np;
}

NP RecSelect(int Op, NP tree, char *FldName)    /* $B!J4V@\!K%a%s%P1i;;(B */
{
  STP p;
  TDP texpr;

  if (tree->Op == IDS && (SYMP(tree))->class != VAR) {
    yyerror("Reference to non-structured variable");
    return tree; }
  texpr = tree->type;
  if (Op == INMEM) {                            /* $B%a%s%P$N4V@\;2>H(B ? */
    if (texpr->tcons != REF) {
      yyerror("Reference to non-pointer type object");
      return tree; }
    texpr = texpr->compnt; }
  if (texpr->tcons != STRUCT) {                 /* $B9=B$BN7?(B ? */
    yyerror("Reference to non-struct type object");
    return tree; }
  for (p = (STP)texpr->compnt; p != NULL; p = p->next)
    if (p->name == FldName && p->class == VAR)  /* $B%a%s%PL>$NHf3S(B */
      break;
  if (p == NULL) {                              /* $B%a%s%P$NC5:w7k2L(B */
    yyerror("Reference to undefined field");
    return tree; }
  if (Op == MEMOP && tree->Op == MEMOP)         /* (Y.g).f ? */
    tree->right = (NP)((int)(tree->right)+(p->loc));
  else
    tree = AllocNode(Op, tree, (NP)(p->loc));   /* $B@aE@$N:n@.(B */
  tree->type = p->type;
  return tree;
}

static int TempC;

int WriteNode(NP np)
{
  extern char *Scode[];
  int opc, t1, t2;
  char *ops;
  static char *Ccode[] = { "[ ]", "Dref", " &", " .", " ->", "arg" };

  if (np == NULL) return -1;
  switch (opc = np->Op) {
  case IDS: { STP p = SYMP(np);
      printf("%5d  %-8s %-16s%", TempC, "id", p->name); }
      break;
  case CONS:
      printf("%5d  ", TempC);
      if (np->type == INTP)
        printf("%-8s %2d %3c", "Int", (int)(np->left), ' ');
      else if (np->type == UNIVP)
        printf("%-8s %1d %14c", "null", 0, ' ');
      break;
  case MEMOP: case INMEM:
      t1 = WriteNode(np->left);
      printf("%5d  %-8s (%2d)", TempC, Ccode[opc-AELM], t1);
      printf("%8d %3c", (int)(np->right), ' ');
      break;
  default:
      t1 = WriteNode(np->left); t2 = WriteNode(np->right);
      ops = opc < AELM ? Scode[opc] : Ccode[opc-AELM];
      printf("%5d  %-8s (%2d)", TempC, ops, t1);
      if (t2 >= 0)
        printf(" %4c(%2d)   ", ' ', t2);
      else
        printf("%12c", ' ');
  }
  PrintType(np->type); printf(" \n");
  return TempC++;
}

void WriteTree(NP tree)
{
  extern int yylineno;

  TempC = 0;
  printf("\nLine %d\n", yylineno);
  WriteNode(tree);
  printf("\n");
}
