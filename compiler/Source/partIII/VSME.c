/* $B%W%m%0%i%`(B 12.2 : VSME-E$B%7%_%e%l!<%?!J(BVSME.c$B!$(B281$B!A(B285$B%Z!<%8!K(B */

#include <stdio.h>
#include "VSME.h"

extern int GAptr;                                /* $B@EE*3dIU$1%+%&%s%?(B */

int    DebugSW = 0;                              /* $B%G%P%C%0%9%$%C%A(B */
static int Pctr=0, SP=0, Freg=0;
static int InsCount=0, MaxSD=0, MinFP=MAX_DSEG, MaxPC=0, CallC=0;

static INSTR Iseg[ISEG_SIZE];                    /* $BL?Na%;%0%a%s%H(B */

static double Dmemory[DSEG_SIZE];                /* $B%G!<%?%;%0%a%s%H(B */
static char * const Dseg = (char *)Dmemory;      /* $B%P%$%H9=@.$N%a%b%j(B */

#define S_SIZE 100
union OpdS{                                      /* $B%*%Z%i%s%I%9%?%C%/(B */
  int I;                                              /* $B@0?t7?$NMWAG(B */
  double D;                                           /* $B<B?t7?$NMWAG(B */
} S[S_SIZE];

char *Scode[] = { "[Nop]", " =(C)",  " =",    " =(D)", " +",   " +(D)", 
      " +i",    " -",    " -(D)",  " -i",   " *",    " *(D)",  " *i",
      " /",     " /(D)", " /i",    " %",    "???",   " %i",    " -'",
      " -'(D)", "comp",  "Dcomp",  "comp_i","copy",  "Cpush",  "push",
      "Dpush",  "push-i","Crval",  "rval",  "Drval", "remove", "Cpop",
      "pop",    "Dpop",  "setFR",  "++FR",  "--FR",  "Int-Dbl","Dbl-Int",
      "jump",   "<0 ?",  "<=0 ?",  "==0 ?", "!=0 ?", ">=0 ?",  ">0 ?",
      "call",   "return","halt",   "Mcopy", "Malloc","Mfree",  "input",
      "output", "outstr" };

static void PrintIns(int loc)                    /* $BL?Na$N5-9fJT=8$H(B */
{                                                /* $BI=<((B */
  int op;

  op = Iseg[loc].Op;
  printf("%5d  %-8s ", loc, Scode[op]);
  switch (op) {
    case ADDI:  case SUBI:  case MULI:  case DIVI:  case MODI:
    case COMPI: case CPUSH: case PUSH:  case DPUSH: case PUSHI:
    case CPOP:  case POP:   case DPOP:  case JUMP:  case BLT:
    case BLE:   case BEQ:   case BNE:   case BGE:   case BGT:
    case CALL:  case SETFR: case INCFR: case DECFR:
      printf("%7d%4s", Iseg[loc].Addr, Iseg[loc].Reg ? "[fp]" : " ");
      break;
    default: printf("%11c", ' '); }
}

void SetPC(int Addr)                             /* $B%W%m%0%i%`%+%&%s%?(B */
{                                                /* $B$N%;%C%H(B */
  Pctr = Addr;
}

int PC(void)                                     /* $B%W%m%0%i%`%+%&%s%?(B */
{                                                /* $B$NCM$NFI=P$7(B */
  return Pctr;
}

void SetI(OP OpCode, int F, int Addr)            /* $BL?Na=q9~$_(B */
{
    Iseg[Pctr].Op = OpCode;                      /* $BA`:n%3!<%I(B */
    Iseg[Pctr].Reg = F;                          /* $B%l%8%9%?%U%i%0(B */
    Iseg[Pctr].Addr = Addr;                      /* $B%"%I%l%9(B */
    if (DebugSW) { PrintIns(Pctr); printf("\n"); }
    if (++Pctr > MaxPC) MaxPC = Pctr;            /* $BL?Na?t$N%+%&%s%H(B */
}

void Bpatch(int Loc, int Target)                 /* $B%"%I%l%9It$N8eKd$a(B */
{
  while (Loc >= 0) { int p;
    if ((p=Iseg[Loc].Addr) == Loc) {
      printf("Trying to rewrite self address part at loc. %d\n", p);
      return; }
    Iseg[Loc].Addr = Target;
    Loc = p; }
}

void WriteDseg(int Loc, char *Data, int Bytes)   /* Dseg$B$X$N=q9~$_(B */
{
  for (; Bytes>0; Bytes--)
    Dseg[Loc++] = *(Data++);
}

#define BINOP(T, OP, R) { S[SP-1].R = S[SP-1].T OP S[SP].T; }
#define INTP(X) ((int *)(Dseg + (X)))
#define DBLP(X) ((double *)(Dseg + (X)))
#define ZERODIV(X) if (S[SP].X == 0) \
                   { fprintf(stderr, "zero divide\n"); return -1; } 

int StartVSM(int StartAddr, int TraceSW)         /* VSM$B$NL?Na<B9T(B */
{
  char *from, *to;
  int addr, op, c, *ip;
  double *dp;

  Pctr = StartAddr;                              /* Pctr$B$N@_Dj(B */
  while (1) {                                    /* $BL?Na<B9T%5%$%/%k(B */
    if (Pctr < 0 || Pctr >= ISEG_SIZE) {         /* Pctr$BHO0O%A%'%C%/(B */
      fprintf(stderr, "Illegal instr. address : %d\n", Pctr);
      return -1; }
    if (SP >= S_SIZE || SP < 0) {                /* SP$B$NHO0O%A%'%C%/(B */
      fprintf(stderr, "Illegal Stack pointer %d\n", SP);
      return -1; }
    op   = Iseg[Pctr].Op;                        /* $BL?Na$N%U%'%C%A(B */
    addr = Iseg[Pctr].Addr;
    if (Iseg[Pctr++].Reg != 0) addr += Freg;     /* $B%l%8%9%?=$>~(B */
    InsCount++;
    if (SP > MaxSD) MaxSD = SP;                  /* SP$B$N:GBgCM(B */
    if (TraceSW) {                               /* $BL?Na<B9T%H%l!<%9(B */
      PrintIns(Pctr-1);
      printf("%15d %5d %12d %12d\n", addr, SP, S[SP].I, S[SP-1].I); }
    switch (op) {                                /* $B3FL?Na$N<B9T(B */
      case NOP:                                        continue;
      case CASSGN: addr = S[SP-1].I;
                   Dseg[addr] = S[SP-1].I = S[SP].I;   break;
      case ASSGN:  ip = INTP(S[SP-1].I);
                   *ip = S[SP-1].I = S[SP].I;          break;
      case DASSGN: dp = DBLP(S[SP-1].I);
                   *dp = S[SP-1].D = S[SP].D;          break;
      case ADD:    BINOP(I, +, I);                     break;
      case DADD:   BINOP(D, +, D);                     break;
      case ADDI:   S[SP].I += addr;                    continue;
      case SUB:    BINOP(I, -, I);                     break;
      case DSUB:   BINOP(D, -, D);                     break;
      case SUBI:   S[SP].I -= addr;                    continue;
      case MUL:    BINOP(I, *, I);                     break;
      case DMUL:   BINOP(D, *, D);                     break;
      case MULI:   S[SP].I *= addr;                    continue;
      case DIV:    ZERODIV(I); BINOP(I, /, I);         break;
      case DDIV:   ZERODIV(D); BINOP(D, /, D);         break;
      case DIVI:   S[SP].I /= addr;                    continue;
      case MOD:    ZERODIV(I); BINOP(I, %, I);         break;
      case MODI:   S[SP].I %= addr;                    continue;
      case CSIGN:  S[SP].I = -S[SP].I;                 continue;
      case DCSIGN: S[SP].D = -S[SP].D;                 continue;
      case COMP:   S[SP-1].I = S[SP-1].I<S[SP].I? -1:
                               S[SP-1].I>S[SP].I;      break;
      case DCOMP:  S[SP-1].I = S[SP-1].D<S[SP].D? -1:
                               S[SP-1].D>S[SP].D;      break;
      case COMPI:  S[SP].I = S[SP].I < addr ? -1 :
                             S[SP].I > addr;           continue;
      case COPY:   ++SP; S[SP] = S[SP-1];              continue;
      case CPUSH:  S[++SP].I =  Dseg[addr];            continue;
      case PUSH:   S[++SP].I = *INTP(addr);            continue;
      case DPUSH:  S[++SP].D = *DBLP(addr);            continue;
      case PUSHI:  S[++SP].I = addr;                   continue;
      case CRVAL:  S[SP].I =  Dseg[S[SP].I];           continue;
      case RVAL:   S[SP].I = *INTP(S[SP].I);           continue;
      case DRVAL:  S[SP].D = *DBLP(S[SP].I);           continue;
      case REMOVE:                                     break;
      case CPOP:   Dseg[addr]  = S[SP].I;              break;
      case POP:    *INTP(addr) = S[SP].I;              break;
      case DPOP:   *DBLP(addr) = S[SP].D;              break;
      case SETFR : Freg = addr;                        continue;
      case INCFR:  if ((Freg += addr) > MAX_DSEG) {
                     fprintf(stderr, "Run-time stack  overflow\n");
                     return -2; }
                   continue;
      case DECFR : if ((Freg -= addr) < MinFP) {
                     if (Freg <= GAptr) {
                       fprintf(stderr, "Run-time stack underflow\n");
                       return -2; }
                     MinFP = Freg; }                   continue;
      case INTDBL: S[SP].D = S[SP].I;                  continue;
      case DBLINT: S[SP].I = S[SP].D;                  continue;
      case JUMP:                       Pctr = addr;    continue;
      case BLT:    if (S[SP--].I <  0) Pctr = addr;    continue;
      case BLE:    if (S[SP--].I <= 0) Pctr = addr;    continue;
      case BEQ:    if (S[SP--].I == 0) Pctr = addr;    continue;
      case BNE:    if (S[SP--].I != 0) Pctr = addr;    continue;
      case BGE:    if (S[SP--].I >= 0) Pctr = addr;    continue;
      case BGT:    if (S[SP--].I >  0) Pctr = addr;    continue;
      case CALL:   S[++SP].I = Pctr;   Pctr = addr;
                   CallC++;                            continue;
      case RET:    Pctr = S[SP--].I;                   continue;
      case HALT:                                       return 0;
      case MCOPY: SP -= 3; from = Dseg + S[SP+1].I;
                  to = Dseg + S[SP+2].I; c = S[SP+3].I;
                for (; c>0; c--) *(to++) = *(from++);  continue;
      case MALLOC: S[SP].I = 
                       (char *)malloc(S[SP].I) - Dseg; continue;
      case MFREE:  free(Dseg + S[SP--].I);             continue;
      case INPUT:  S[SP-1].I = scanf(Dseg+S[SP-1].I,
                                        Dseg+S[SP].I); break;
      case OUTPUT: printf(Dseg+S[SP-1].I, S[SP].D);
                   SP -= 2;                            continue;
      case OUTSTR: printf(Dseg+S[SP--].I);             continue;
      default:
        printf("Illegal Op. code at loc. %d\n", Pctr);
        return -1;
      }
  SP--;
  }
}

void DumpIseg(int first, int last)               /* Iseg$B$NI=<((B */
{
  printf("\nContents of Instruction Segment\n");
  for (; first <= last; first++)
    PrintIns(first),  printf("\n");
  printf("\n");
}

void ExecReport(void)                            /* $B<B9T%G!<%?$NI=<((B */
{
  printf("\nObject Code Size:%7d instructions\n", MaxPC);
  printf("Global Data Area:%7d bytes\n", GAptr);
  printf("Max Stack Depth: %7d\n", MaxSD);
  printf("Max Frame Size:  %7d bytes\n", MAX_DSEG - MinFP);
  printf("Function Calls:  %7d times\n", CallC);
  printf("Execution Count: %7d ins. \n\n", InsCount);
}
