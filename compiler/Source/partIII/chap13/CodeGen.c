/* $B%W%m%0%i%`(B 14.4 : $B%3!<%I@8@.!J(BCodeGen.c$B!$(B331, 333, 334, 338,
/*                                               341, 343 $B%Z!<%8!K(B */

#include <stdio.h>
#include "../VSME.h"
#include "SymTable.h"
#include "ExprTree.h"
#include "TypeDef.h"

#define OPcode(Opc, texpr) ((texpr)->tcons == CHAR ? Opc-1 : Opc)
#define PUSH_LV(T)         { int lv = EvalLval(T); \
                             if (lv < 0) SetI(PUSHI, FP, -lv); \
                               else if (lv > 0) Cout(PUSHI, lv); }
#define EMIT(OP, AD)       { if (AD < 0) SetI(OP, FP, -AD); \
                               else if (AD > 0) Cout(OP, AD); }
enum { UNSAVE, SAVEV };

extern int StartP;

static void Cgen(NP tree, int Sv), GenCall(STP fp, NP arg, int Sv),
            GenArg(STP param, NP arg);
static int  EvalLval(NP tree), GenCond(NP tree);

void GenFuncEntry(STP func)                     /* $B4X?t$N=P8}$HF~8}(B */
{                                               /* $B$N%3!<%I(B */
  STP pl;

  SetI(PUSH, FP, 0); Cout(INCFR, -1); Pout(RET);    /* $B=P8}$N%3!<%I(B */
  Cout(DECFR, PC()-2); SetI(POP, FP, 0);
  for (pl = func->plist; pl != NULL; pl = pl->next) /* $B0z?t$N0zEO$7(B */
    if (((pl->type)->tcons) >= ARRAY) { int n = (pl->type)->msize;
      SetI(PUSHI, FP, pl->loc);                     /* $B9=B$BN!$G[Ns$N(B */
      if (n <= 0)                                   /* $BCM$N%3%T!<L?Na(B */
        yyerror("Missing size information on parameter");
	Cout(PUSHI, n); Pout(MCOPY); }
    else
      SetI(OPcode(POP, pl->type), FP, pl->loc);     /* $BCMEO$7(B */
}

void Epilogue(void)                             /* $B%3%s%Q%$%k$N8e=hM}(B */
{
  STP sp;
  extern char *IDentry(char *Name, int len);

  StartP = PC();                                /* $B<B9T3+;O%"%I%l%9(B */
  Cout(SETFR, MAX_DSEG);                        /* Freg$B$N=i4|@_Dj(B */
  if (sp = SymRef(IDentry("main", 4))) {        /* "main"$B$NC5:w(B */
    if (sp->Nparam > 0)                         /* $B0z?t$N%A%'%C%/(B */
      yyerror("Illegal parameter specification of main()");
    Cout(CALL, sp->loc); }                      /* main()$B$N8F=P$7(B */
  Pout(HALT);                                   /* $B<B9T=*N;L?Na$N@8@.(B */
  UndefCheck();                                 /* $BL$Dj5A4X?t$N%A%'%C%/(B */
  CloseBlock();                                 /* $BIT40A47?$N%A%'%C%/(B */
}

static void Cgen(NP tree, int Sv)               /* $B<0$N%3!<%I@8@.(B */
{
  int addr, temp;
  NP  lt = tree->left, rt = tree->right;        /* $B:81&$NItJ,LZ(B */
  TDP texpr = tree->type;                       /* $B7k2L$N7?(B */

  switch(tree->Op) {
    case ASSGN:                                 /* $BBeF~<0(B */
      if (texpr->tcons >= ARRAY) {              /* $B9=B$BN!$G[Ns$NBeF~(B */
        Cgen(rt, rt->Op == CALL ? UNSAVE : SAVEV);      /* $B1&JU$NI>2A(B */
        if ((addr = EvalLval(lt)) == 0) {               /* $B:8JU$NI>2A(B */
          if (Sv == SAVEV) {
            M_ALLOC(temp, FAptr, IBSZ, IBSZ);
            Pout(COPY); SetI(POP, FP, temp); }
          Cout(PUSHI, texpr->msize); Pout(MCOPY);
          if (Sv == SAVEV) SetI(PUSH, FP, temp); }
        else {
          EMIT(PUSHI, addr); Cout(PUSHI, texpr->msize); Pout(MCOPY);
          if (Sv == SAVEV) EMIT(PUSHI, addr); }
      }
      else if (Sv == SAVEV) {                   /* $B%9%+%i7?$NBeF~1i;;(B */
        PUSH_LV(lt); Cgen(rt, SAVEV); Pout(OPcode(ASSGN, texpr)); }
      else {                                    /* $B<0J8$N$H$-(B */
        addr = EvalLval(lt);                            /* $B:8JU$NI>2A(B */
        Cgen(rt, SAVEV);                                /* $B1&JU$NI>2A(B */
        if (addr == 0) {
          Pout(OPcode(ASSGN, texpr)); Pout(REMOVE); }
        else
          EMIT(OPcode(POP, texpr), addr); }             /* $BD>@\BeF~(B */
      break;
    case ADD: case SUB: case MUL: case DIV: case MOD:   /* $B;;=Q1i;;(B */
      Cgen(lt, SAVEV); Cgen(rt, SAVEV); Pout(tree->Op); break;
    case CSIGN:                                         /* $BC19`!](B */
      Cgen(lt, SAVEV); Pout(CSIGN); break;
    case COMP: { int jaddr = GenCond(tree);             /* $B4X781i;;(B */
      Cout(PUSHI, 1); Cout(JUMP, PC()+2);
      Bpatch(jaddr, PC()); Cout(PUSHI, 0); break; }
    case CALL:                                          /* $B4X?t8F=P$7(B */
      GenCall(SYMP(lt), rt, Sv); 
      break;
    case REFOBJ:                                        /* $B4V@\1i;;(B */
      Cgen(lt, SAVEV);
      if (texpr->tcons < ARRAY) Pout(OPcode(RVAL, texpr));
      break;
    case ADDROP:                                      /* $B%"%I%l%91i;;(B */
      PUSH_LV(lt); break;
    case AELM: case MEMOP: case INMEM: case IDS:      /* $B$=$NB>$N1i;;(B */
      if (texpr->tcons >= ARRAY)                      /* $BG[Ns!$9=B$BN(B */
        PUSH_LV(tree)
      else { int addr = EvalLval(tree);               /* $B:8JUCM$NI>2A(B */
        if (addr == 0)
          Pout(OPcode(RVAL, texpr));
        else
          EMIT(OPcode(PUSH, texpr), addr); }
      break;
    case CONS:                                        /* $B@0?tDj?t(B */
      Cout(PUSHI, (int)lt);
  }
}

static int EvalLval(NP tree)                    /* $B:8JUCM$r5a$a$k(B */
{                                               /* $B%3!<%I$N@8@.(B */
  int addr;
  NP  lt = tree->left, rt = tree->right;

  switch (tree->Op) {
    case IDS: { STP sp = SYMP(tree);                    /* $BJQ?tL>(B */
      if (sp->class != VAR)
        yyerror("Non-variable name used as an operand");
      return (sp->deflvl > 0) ? -(sp->loc) : sp->loc; }
    case AELM: { TDP tp = tree->type;                   /* $BG[NsMWAG(B */
      addr = (lt->Op == CALL) ? Cgen(lt, SAVEV), 0 : EvalLval(lt);
      if (rt->Op == CONS) { int of = (tp->msize)*(int)(rt->left);
        if (addr != 0)
          return (addr > 0) ? addr+of : addr-of;        /* $BDj?t7W;;(B */
        if (of > 1) Cout(ADDI, of); }                   /* $B<B9T;~7W;;(B */
      else {
        EMIT(PUSHI, addr);
        Cgen(rt, SAVEV);
        if (tp->msize > 1) Cout(MULI, tp->msize);
        Pout(ADD); }
      break; }
    case REFOBJ: case INMEM:                            /* $B4V@\1i;;(B */
      Cgen(lt, SAVEV);                                  /* $B4V@\%a%s%P(B */
      if (rt != NULL) Cout(ADDI, (int)rt);
      break;
    case MEMOP: { int of = (int)rt;                     /* X.$B%a%s%P(B */
      addr = (lt->Op == CALL) ? Cgen(lt, SAVEV), 0 : EvalLval(lt);
      if (addr != 0)
        return (addr > 0)? addr+of : addr-of;           /* $BDj?t7W;;(B */
      if (of > 0) Cout(ADDI, of);                       /* $B<B9T;~7W;;(B */
      break; }
    default:
      yyerror("Illegal left-hand side value");
    }
  return 0;                                     /* $B:8JUCM$O%9%?%C%/>e(B */
}

static void GenCall(STP fp, NP Arg, int Sv)     /* $B4X?t8F=P$7$NK]Lu(B */
{
  int as, temp;
  TDP tp = fp->type;                            /* $BJV5QCM$N7?%j%9%H(B */

  GenArg(fp->plist, Arg);                       /* $B<B0z?t$NI>2A%3!<%I(B */
  Cout(CALL, fp->loc);
  if (fp->class == F_PROT)                      /* $B4X?t%W%m%H%?%$%W(B */
    fp->loc = PC() - 1;                         /* $B8eKd$a%A%'%$%s99?7(B */
  if (tp->tcons >= ARRAY && Sv == SAVEV) {      /* $BG[Ns!$9=B$BN$N4X?t(B */
    as = tp->msize;                             /* $BJV5QCM$N%P%$%H?t(B */
    M_ALLOC(temp, FAptr, as, tp->atoms);        /* $B$=$NNN0h$N3NJ](B */
    SetI(PUSHI, FP, temp); Cout(PUSHI, as);
    Pout(MCOPY); SetI(PUSHI, FP, temp); }       /* $B%3%T!<L?Na$N@8@.(B */
}

static void GenArg(STP param, NP arg)           /* $B<B0z?tJB$S$NK]Lu(B */
{
  if (param != NULL && arg != NULL) {           /* $B0z?t$N8D?tIT0lCW(B ? */
    if (arg->Op == ARGL) {                      /* $B<B0z?tJB$S(B ? */
      GenArg(param->next, arg->left);
      arg = arg->right; }
    CmptblType(param->type, arg->type);         /* $B7?%A%'%C%/(B */
    Cgen(arg, SAVEV); }                         /* $B<B0z?t$N<0$NK]Lu(B */
}

void GenNewDel(int OP, NP tree)                 /* new()$B$H(Bdelete() */
{
  TDP texpr = tree->type;                       /* $B<B0z?t$N7?(B */

  if (texpr->tcons != REF)                      /* $B%]%$%s%?7?(B ? */
    yyerror("Non-reference type object");
  if (DebugSW) WriteTree(tree);
  if (OP == MALLOC) { int addr, sz;             /* new() */
    sz = (texpr->compnt)->msize;                /* $BHo;2>H7?$N%P%$%H?t(B */
    if (sz <= 0) yyerror("Unknown size of object");
    addr = EvalLval(tree);                      /* $B<B0z?t$N:8JUCM(B */
    Cout(PUSHI, sz); Pout(MALLOC);
    if (addr == 0) {                            /* $B3MF@$7$?NN0h$N(B */
      Pout(ASSGN); Pout(REMOVE); }              /* $B%"%I%l%9$N3JG<(B */
    else 
      EMIT(POP, addr); }
  else {                                        /* delete() */
    Cgen(tree, SAVEV); Pout(MFREE); }
  FreeSubT(tree);                               /* $BLZ$N%a%b%j$r2rJ|(B */
}

void GenExpr(int Op, NP tree)                   /* $B<0J8$N%3!<%I@8@.(B */
{
  int t = (tree->type)->tcons;                  /* $B<0J8$N7?9=@.;R(B */

  if (DebugSW) WriteTree(tree);
  OpenBlock();                                  /* $B2>$N%V%m%C%/3+;O(B */
  if (Op == OUTPUT) {                           /* write$BJ8$N$H$-(B */
    Cgen(tree, SAVEV); Pout(OUTPUT); 
    if (t == VOID || t >= ARRAY)
      yyerror("Illegal type of expression in write-statement"); }
  else {                                        /* $B<0J8$N$H$-(B */
    Cgen(tree, UNSAVE);
    if (tree->Op != ASSGN && t != VOID) Pout(REMOVE); }
  CloseBlock();                                 /* $B2>$N%V%m%C%/=*N;(B */
  FreeSubT(tree);                               /* $BLZ$N%a%b%j$r2rJ|(B */
}

int CtrlExpr(NP tree)                           /* $B@)8f<0$NK]Lu(B */
{
  int jaddr;

  if (DebugSW) WriteTree(tree);
  OpenBlock();                                  /* $B2>$N%V%m%C%/3+;O(B */
  jaddr = GenCond(tree);                        /* $BJ,4t%3!<%I$N@8@.(B */
  CloseBlock(); FreeSubT(tree);                 /* $B2>$N%V%m%C%/=*N;(B*/
  return jaddr;                                 /* $BJ,4t%j%9%H$rJV$9(B */
}

void GenReturn(STP func, NP tree)               /* return$BJ8$NK]Lu(B */
{
  TDP texpr = func->type;                       /* $B4X?t$NJV5QCM$N7?(B */

  if (texpr->tcons == VOID) {                   /* void$B7?$N4X?t(B ? */
    if (tree != NULL)                           /* $BJV5QCM$N;XDj$"$j(B ? */
      yyerror("Meaningless return value specification"); }
  else if (tree == NULL)                        /* $BJV5QCM$N;XDj$J$7(B ? */
    yyerror("Missing return value");
  else
    CmptblType(texpr, tree->type);              /* $BJV5QCM$N7?%A%'%C%/(B */
  if (tree) {
    if (DebugSW) WriteTree(tree);
    OpenBlock(); Cgen(tree, SAVEV); CloseBlock(); /* $BJV5QCM$N<0$NK]Lu(B */
    FreeSubT(tree); }
  Cout(JUMP, (func->loc)-3);                    /* $B=P8}$X$N%8%c%s%W(B */
}

static int GenCond(NP tree)                     /* $BJ,4t%3!<%I$N@8@.(B */
{
  static int NegC[] = { BGE, BGT, BNE, BEQ, BLT, BLE };

  if (tree->Op == COMP) { NP lt = tree->left, rt = tree->right;
    Cgen(lt, SAVEV);
    if (rt->Op != CONS || (int)(rt->left) != 0) {  /* $B#0$H$NHf3S(B ? */
      Cgen(rt, SAVEV);  Pout(COMP); }
    Cout(NegC[tree->etc - BLT], -1); }             /* $BJ,4tL?Na$N@8@.(B */
  else { int t = (tree->type)->tcons;              /* $B4X78<00J30$N<0(B */
    if (t == VOID || t > REF)
      yyerror("Illegal type of control expression");
    Cgen(tree, SAVEV);  Cout(BEQ, -1); }           /* $B#0$N$H$-J,4t(B */
  return PC()-1;                                   /* $BJ,4t@h%j%9%H(B */
}
