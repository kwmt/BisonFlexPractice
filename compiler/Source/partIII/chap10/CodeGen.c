/* $B%W%m%0%i%`(B 12.1 : $B%3!<%I@8@.4X?t!J(BCodeGen.c$B!$(B253, 256, 259, 261, */
/*                                    264, 268, 271, 272, 276$B%Z!<%8!K(B */

#include <stdio.h>
#include "../VSME.h"
#include "SymTable.h"
#include "ExprTree.h"
#include "CodeGen.h"

#define OPC(X, T) ((X)+(T)-INT)
#define FPBIT(P)  ((P->attrib) & SALLOC ? 0 : FP)

extern char *IDentry(char *Name, int len);
extern int StartP, DebugSW;

static void Cgen(NP tree), PushLval(NP tree),
            GenArg(STP param, NP arg, int pnumber);
static int  GenCond(NP tree, int cond, int bpc), 
            GenAddrComp(STP Ap, NP subs);


void GenFuncEntry(STP func)                     /* $B4X?t$N=P8}$HF~8}(B */
{                                               /* $B$N%3!<%I(B */
  STP pl;

  SetI(PUSH, FP, 0); Cout(INCFR, -1); Pout(RET);    /* $B=P8}$N%3!<%I(B */
  Cout(DECFR, PC()-2); SetI(POP, FP, 0);
  for (pl = func + func->Nparam; pl > func; pl--)   /* $B0z?t$N0zEO$7(B */
    if ((pl->attrib) & BYREF)
      SetI(POP, FP, pl->loc);                       /* $B;2>HEO$7(B */
    else
      SetI(OPC(POP, pl->type), FP, pl->loc);        /* $BCMEO$7(B */
}

void ExprStmnt(NP tree)                         /* $B<0J8$NK]Lu(B */
{
  if (tree == NULL) return;
  if (DebugSW) WriteTree(tree);                 /* $B<0$NLZ$N#3$DAHI=<((B */
  if (tree->Op == ASSGN && (tree->left)->Op == IDS) { STP sp;
    sp = SYMP(tree->left);
    if (sp->attrib & BYREF) {                   /* $B;2>HEO$7$NJQ?t$X$N(B */
      Cgen(tree); Pout(REMOVE); }               /* $BBeF~(B */
    else {                                      /* $B%9%+%iJQ?t$X$NBeF~(B */
      Cgen(tree->right);
      SetI(OPC(POP, sp->type), FPBIT(sp), sp->loc); }
  }
  else {                                        /* $BG[NsMWAG$X$NBeF~(B */
    Cgen(tree);
    if (tree->type != VOID) Pout(REMOVE); }
  FreeSubT(tree);                               /* $B<0$NLZ$N%a%b%j2rJ|(B */
}

int CtrlExpr(NP tree, int jc)                   /* $B@)8f<0$NK]Lu(B */
{
  if (tree == NULL) return -1;
  if (DebugSW) WriteTree(tree);
  INTCHK(tree);                                 /* $B@0?t7?$N<0(B ? */
  jc = GenCond(tree, jc, -1);                   /* $BJ,4t%3!<%I$X$NK]Lu(B */
  FreeSubT(tree);
  return jc;                                    /* $BJ,4t@h%j%9%H$rJV$9(B */
}

void GenReturn(STP func, NP tree)               /* return$BJ8$NK]Lu(B */
{
  if (func->type == VOID) {                     /* void$B7?$N4X?t(B ? */
    if (tree != NULL)
      yyerror("Meaningless return value"); }
  else if (tree == NULL)
    Cout(PUSHI, 0);                             /* $B2>$NJV5QCM$N@8@.(B */
  else
    tree = TypeConv(tree, func->type);          /* $B4X?t$N7?$X$NJQ49(B */
  if (tree) {
    if (DebugSW) WriteTree(tree);
    Cgen(tree); FreeSubT(tree); }               /* $BJV5QCM$N%3!<%I@8@.(B */
    Cout(JUMP, (func->loc)-3);                  /* $B=P8}$X$N%8%c%s%W(B */
}

void Epilogue(void)                             /* $B%3%s%Q%$%k$N8e=hM}(B */
{
  STP sp;

  UndefCheck();                                 /* $BL$Dj5A4X?t$N%A%'%C%/(B */
  StartP = PC();                                /* $B<B9T3+;O%"%I%l%9(B */
  Cout(SETFR, MAX_DSEG);                        /* Freg$B$N=i4|@_Dj(B */
  if (sp = SymRef(IDentry("main", 4))) {
    if (sp->Nparam > 0)
      yyerror("Parameter list specified at main()");
    Cout(CALL, sp->loc); }                      /* main()$B$N8F=P$7(B */
  Pout(HALT);                                   /* $B<B9T=*N;L?Na$N@8@.(B */
}

static void Cgen(NP tree)                       /* $B<0$N%3!<%I@8@.(B */
{
  int Nid = tree->Op,  T = tree->type, jc, n;   /* $B@aE@(Bid$B!$7k2L$N7?(B */
  NP  lt = tree->left, rt = tree->right;        /* $B:81&$NItJ,LZ(B */
  STP sp;

  switch (Nid) {
    case ASSGN:                                        /* $BBeF~<0(B */
      PushLval(lt); Cgen(rt); break;
    case ADD:  case SUB:  case MUL:  case DIV:  case MOD:  /* $B;;=Q<0(B */
      Cgen(lt);
      if (T == INT && (rt->Op) == CONS) {
        Cout(Nid+2, INTV(rt)); return; }
      Cgen(rt); break;
    case INTDBL:               T = INT;                /* (double) */
    case CSIGN:  case DBLINT:  Cgen(lt); break;        /* -'$B!$(B(int) */
    case INC:  case DEC: T = lt->type;                 /* ++, -- */
      if (lt->Op == IDS && !((sp = SYMP(lt))->attrib & BYREF)) {
        SetI(OPC(PUSH, T), FPBIT(sp), sp->loc);
        if (tree->etc == PRE) {                        /* $BA0CV1i;;;R(B */
          Cout(Nid == INC ? ADDI : SUBI, 1);
          Pout(COPY); }
        else {                                         /* $B8eCV1i;;;R(B */
          Pout(COPY);
          Cout(Nid == INC ? ADDI : SUBI, 1); }
        SetI(OPC(POP, T), FPBIT(sp), sp->loc); }
      else {
        PushLval(lt); Pout(COPY); Pout(OPC(RVAL, T));
        Cout(Nid==INC? ADDI: SUBI, 1); Pout(OPC(ASSGN, T));
        if (tree->etc == POST)
          Cout(Nid == INC? SUBI: ADDI, 1); }
      return;
    case AND:  case OR:  case NOT:  case COMP:       /* $BO@M}4X781i;;(B */
      jc = GenCond(tree, Nid == NOT? TJ : FJ, -1);
      Cout(PUSHI, 1);   Cout(JUMP, PC()+2);
      Bpatch(jc, PC()); Cout(PUSHI, 0); return;
    case AELM:                                       /* $BG[NsMWAG(B */
      PushLval(tree); Nid = RVAL; break;
    case CALL: sp = SYMP(lt);                        /* $B4X?t8F=P$7(B */
      GenArg(sp + sp->Nparam, rt, sp->Nparam);
      Cout(CALL, sp->loc);
      if (sp->class == F_PROT)
        sp->loc = PC() - 1;
      return;
    case INPUT:  PushLval(lt); PushLval(rt); break;  /* $BF~=PNO(B */
    case OUTPUT: PushLval(lt); Cgen(rt);     Pout(OUTPUT); return;
    case OUTSTR: PushLval(lt); Pout(OUTSTR); return;
    case CONS: n = (int)lt;                          /* $BDj?t(B */
      if (T == INT)                                  /* $B@0?tDj?t(B */
        Cout(PUSHI, n);
      else if (T == DBL) {                           /* $B<B?tDj?t(B */
        if (DCtbl[n].addr < 0)                       /* $B%a%b%j3dIU$1(B ? */
          DCtbl[n].addr = ALLOCNUM(DCtbl[n].Dcons, DBSZ);
        Cout(DPUSH, DCtbl[n].addr); }
      return;
    case IDS: sp = (STP)lt;                          /* $BJQ?t(B */
      if (sp->attrib & BYREF) {                      /* $B;2>HEO$7(B ? */
        SetI(PUSH, FP, sp->loc); Pout(OPC(RVAL, T)); }
      else
        SetI(OPC(PUSH, T), FPBIT(sp), sp->loc);
      return;
  }
  Pout(OPC(Nid, T));                            /* $B1i;;L?Na$N=PNO(B */
}

static void PushLval(NP tree)                   /* $B:8JUCM$N$?$a$N(B */
{                                               /* $B%3!<%I@8@.(B */
  int n, m, offset;
  STP sp;

  switch (tree->Op) {
    case IDS: sp = SYMP(tree);                  /* $B%9%+%iJQ?t(B */
      if (sp->attrib & BYREF)
        SetI(PUSH, FP, sp->loc);
      else
        SetI(PUSHI, FPBIT(sp), sp->loc);
      return;
    case CONS: Cout(PUSHI, INTV(tree)); return; /* $BJ8;zNs%j%F%i%k(B */
    case AELM: sp = SYMP(tree->left);           /* $BG[NsMWAG(B */
      offset = GenAddrComp(sp, tree->right);    /* $B%*%U%;%C%H7W;;(B */
      for (m = ByteW[sp->type], n = tree->etc; n < sp->dim; n++)
        m *= (sp->dlist)[n];
      if (offset >= 0)                          /* $B%*%U%;%C%H$ODj?t(B ? */
        if (sp->attrib & BYREF) {
          SetI(PUSH, FP, sp->loc); Cout(ADDI, offset*m); }
        else
          SetI(PUSHI, FPBIT(sp), sp->loc + offset*m);
      else {                                    /* $B<B9T;~$K%*%U%;%C%H(B */
        if (m > 1) Cout(MULI, m);               /* $B$r7W;;$9$k>l9g(B */
        if (sp->attrib & BYREF) {               /* $B4pDl%"%I%l%9$N2C;;(B */
          SetI(PUSH, FP, sp->loc); Pout(ADD); }
        else
          SetI(ADDI, FPBIT(sp), sp->loc);
      }
  }
}

static int GenAddrComp(STP Ap, NP subs)         /* $B%*%U%;%C%H7W;;MQ$N(B */
{                                               /* $B%3!<%I$N@8@.(B */
  if (subs->Op == SUBL) { int m, n;             /* $BB?<!85G[Ns(B ? */
    if ((m = GenAddrComp(Ap, subs->left)) >= 0) {   /* $B:8$NJB$S$N=hM}(B */
      m *= (Ap->dlist)[subs->etc - 1];
      if ((n = GenAddrComp(Ap, subs->right)) >= 0)  /* $B1&B&$NE:;z<0(B */
        return m + n;                           /* $BE:;z<0JB$S$,Dj?t(B */
      if (m > 0) Cout(ADDI, m); }
    else {
      Cout(MULI, (Ap->dlist)[subs->etc - 1]);   /* $BMWAG?t$N>h;;L?Na(B */
      Cgen(subs->right); Pout(ADD); }           /* $BE:;z<0$NCM$N2C;;(B */
    }
  else if (subs->Op == CONS)                    /* $BE:;z<0$ODj?t(B ? */
    return INTV(subs);
  else                                          /* $B$=$N$[$+$N<0$N>l9g(B */
    Cgen(subs);
  return -1;
}

static void GenArg(STP param, NP arg, int pn)   /* $B<B0z?tJB$S$NK]Lu(B */
{
  if (pn <= 0 || arg == NULL) return;           /* $B0z?t$N8D?tIT0lCW(B ? */
  if (arg->Op == ARGL) {                        /* $B<B0z?tJB$S(B ? */
    GenArg(param-1, arg->left, pn-1);
    arg = arg->right; }
  if (param->attrib & BYREF) {                  /* $B;2>HEO$7(B ? */
    if (param->type != ((arg->type)&(~ARY)))    /* $B7?%A%'%C%/(B */
      yyerror("Argument type unmatched to parameter");
    PushLval(arg); }                            /* $B:8JUCM$N7W;;(B */
  else {                                        /* $BCMEO$7$N0z?t(B */
    arg = TypeConv(arg, param->type);           /* $B2>0z?t$N7?$X$NJQ49(B */
    Cgen(arg); }                                /* $B<B0z?t$N<0$NK]Lu(B */
}

static int GenCond(NP tree, int cond, int bpc)  /* $B@)8f<0$N%3!<%I@8@.(B */
{
  int temp, n, cc;
  NP  lt = tree->left, rt = tree->right;          /* $B:81&$NItJ,LZ(B */
  static int NegC[] = { BGE, BGT, BNE, BEQ, BLT, BLE };

  switch (tree->Op) {
    case AND:                                     /* $BO@M}(BAND$B1i;;(B */
      if (cond == TJ) {                           /* if(lt && rt) */
        temp = GenCond(lt, FJ, -1);
        bpc =  GenCond(rt, TJ, bpc);
        Bpatch(temp, PC()); }                     /* if(!lt)$B$N8eKd$a(B */
      else {                                      /* if(!(lt && rt)) */
        bpc = GenCond(lt, FJ, bpc);
        bpc = GenCond(rt, FJ, bpc); }
      return bpc;
    case OR:                                      /* $BO@M}(BOR$B1i;;(B */
      if (cond == TJ) {                           /* if(lt || rt) */
        bpc = GenCond(lt, TJ, bpc);
        bpc = GenCond(rt, TJ, bpc); }
      else {                                      /* if(!(lt || rt)) */
        temp = GenCond(lt, TJ, -1);
        bpc =  GenCond(rt, FJ, bpc);
        Bpatch(temp, PC()); }                     /* if(lt)$B$N8eKd$a(B */
      return bpc;
    case NOT:                                     /* $BO@M}H]Dj1i;;(B */
      return GenCond(lt, cond == TJ ? FJ : TJ, bpc); 
    case COMP:                                    /* $B4X781i;;(B */
      Cgen(lt);
      if (rt->Op == CONS && rt->type == INT) {    /* $B@0?tDj?t$H$NHf3S(B */
        if (INTV(rt) != 0) Cout(COMPI, INTV(rt)); }
      else {
        Cgen(rt); Pout(lt->type == DBL ? DCOMP : COMP); }
      cc = tree->etc;                                    /* $BJ,4t>r7o(B */
      Cout(cond == TJ ? cc : NegC[cc-BLT], bpc);         /* $BJ,4tL?Na(B */
      return PC()-1;
    case CONS:  n = (int)lt;                             /* $BDj?t(B */     
      if ((cond == TJ && n != 0 ) || (cond == FJ && n == 0)) {
        Cout(JUMP, bpc); bpc = PC()-1; }
      return bpc;
    default:                                          /* $B$=$N$[$+$N<0(B */
      Cgen(tree); SetI(JUMP, cond == TJ ? BNE : BEQ, bpc);
      return PC()-1;
    }
}
