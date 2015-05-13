/* $B%W%m%0%i%`(B 6.4 : VSM$B%7%_%e%l!<%?!J(BVSM.c$B!$(B109$B%Z!<%8!K(B */

#include <stdio.h>
#include "VSM.h"

int DebugSW=0;                                   /* $B%G%P%C%0%b!<%II=<((B */

static int Pctr=0, SP=0, Freg=0;
static int InsCount=0, MaxSD=0, MinFR=DSEG_SIZE, MaxPC=0, CallC=0;

static INSTR Iseg[ISEG_SIZE];                    /* $BL?Na%;%0%a%s%H(B */
static int   Dseg[DSEG_SIZE];                    /* $B%G!<%?%;%0%a%s%H(B */

#define STACK_SIZE 100
static int Stack[STACK_SIZE];                    /* $B%*%Z%i%s%I%9%?%C%/(B */

char *Scode[] = {                                /* $BI=<(MQ$NA`:n%3!<%I(B */
                 "Nop",    "  =",   "  +",    "  -",    "  *",  "  /",
                 "  %",    "  -'",  "and",    "or",     "not",  "comp",
                 "copy",   "push",  "push-i", "remove", "pop",  " ++",
                 " --",    "setFR", "++FR",   "--FR",   "jump", "<0 ?",
                 "<=0 ?",  "==0 ?", "!=0 ?",  ">=0 ?",  ">0 ?", "call",
                 "return", "halt",  "input",  "output" };

static void PrintIns(int loc)                    /* $BL?Na$N5-9fJT=8$H(B */
{                                                /* $BI=<((B */
  int   op;

  op = Iseg[loc].Op;
  printf("%5d  %-8s ", loc, Scode[op]);
  switch (op) {
    case PUSH:  case PUSHI: case POP: case SETFR: case INCFR:
    case DECFR: case JUMP:  case BLT: case BLE:   case BEQ:
    case BNE:   case BGE:   case BGT: case CALL:
      printf("%6d%4s", Iseg[loc].Addr, Iseg[loc].Reg ? "[fp]" : " "); 
      break;
    default:
      printf("%10c", ' '); }
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
    if (DebugSW) {
      PrintIns(Pctr); printf("\n"); }
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

#define BINOP(OP) {Stack[SP-1] = Stack[SP-1] OP Stack[SP]; SP--;}

int StartVSM(int StartAddr, int TraceSW)         /* VSM$B$NL?Na<B9T(B */
{
  int addr, op;

  Pctr = StartAddr;                              /* Pctr$B$N@_Dj(B */
  SP = Freg = 0;                                 /* $B%l%8%9%?$N@_Dj(B */
  while (1) {
    if (SP >= STACK_SIZE || SP < 0) {            /* SP$B$NHO0O%A%'%C%/(B */
      fprintf(stderr, "Illegal Stack pointer %d\n", SP);
      return -1; }
    op   = Iseg[Pctr].Op;                        /* $BL?Na$N%U%'%C%A(B */
    addr = Iseg[Pctr].Addr;                      /* $B<B8z%"%I%l%9(B */
    if (Iseg[Pctr++].Reg & FP)                   /* FP$B%l%8%9%?=$>~(B ? */
      addr += Freg;                              /* FP$B$r2C;;(B */
    InsCount++;                                  /* $BAm<B9TL?Na?t(B */
    if (SP > MaxSD) MaxSD = SP;                  /* SP$B$N:GBgCM(B */
    if (TraceSW) {                               /* $BL?Na<B9T%H%l!<%9(B */
      PrintIns(Pctr-1);
      printf("%15d %5d %12d\n", addr, SP, Stack[SP]); }
    switch (op) {                                /* $B3FL?Na$N<B9T(B */
      case NOP:                                              continue;
      case ASSGN:  addr = Stack[--SP];
                   Dseg[addr] = Stack[SP] = Stack[SP+1];     continue;
      case ADD:    BINOP(+);                                 continue;
      case SUB:    BINOP(-);                                 continue;
      case MUL:    BINOP(*);                                 continue;
      case DIV:    if (Stack[SP] == 0) {
                     yyerror("Zero divider detected"); return -2; }
                   BINOP(/);                                 continue;
      case MOD:    if (Stack[SP] == 0) {
                     yyerror("Zero divider detected"); return -2; }
                   BINOP(%);                                 continue;
      case CSIGN:  Stack[SP] = -Stack[SP];                   continue;
      case AND:    BINOP(&&);                                continue;
      case OR:     BINOP(||);                                continue;
      case NOT:    Stack[SP] = !Stack[SP];                   continue;
      case COMP:   Stack[SP-1] = Stack[SP-1] > Stack[SP] ? 1 :
                                 Stack[SP-1] < Stack[SP] ? -1 : 0;
                   SP--;                                     continue;
      case COPY:   ++SP; Stack[SP] = Stack[SP-1];            continue;
      case PUSH:   Stack[++SP] = Dseg[addr];                 continue;
      case PUSHI:  Stack[++SP] = addr;                       continue;
      case REMOVE: --SP;                                     continue;
      case POP:    Dseg[addr] = Stack[SP--];                 continue;
      case INC:    Stack[SP] = ++Stack[SP];                  continue;
      case DEC:    Stack[SP] = --Stack[SP];                  continue;
      case SETFR:  Freg = addr;                              continue;
      case INCFR : if ((Freg += addr) >= DSEG_SIZE) {
                     printf("Freg overflow at loc. %d\n", Pctr-1);
                     return -3; }                            continue;
      case DECFR : Freg -= addr;
                   if (Freg < MinFR) MinFR = Freg;           continue;
      case JUMP:   Pctr = addr;           continue;
      case BLT:    if (Stack[SP--] <  0) Pctr = addr;        continue; 
      case BLE:    if (Stack[SP--] <= 0) Pctr = addr;        continue; 
      case BEQ:    if (Stack[SP--] == 0) Pctr = addr;        continue; 
      case BNE:    if (Stack[SP--] != 0) Pctr = addr;        continue; 
      case BGE:    if (Stack[SP--] >= 0) Pctr = addr;        continue; 
      case BGT:    if (Stack[SP--] >  0) Pctr = addr;        continue; 
      case CALL:   Stack[++SP] = Pctr; Pctr = addr; CallC++; continue;
      case RET:    Pctr = Stack[SP--];                       continue;
      case HALT:                                             return 0;
      case INPUT:  scanf("%d", &Dseg[Stack[SP--]]);          continue;
      case OUTPUT: printf("%15d\n", Stack[SP--]);            continue;
      default:
        printf("Illegal Op. code at location %d\n", Pctr);   return -4;
      }
  }
}

void DumpIseg(int first, int last)               /* Iseg$B$NI=<((B */
{
  printf("\nContents of Instruction Segment\n");
  for (; first<=last; first++)
    PrintIns(first),  printf("\n");
  printf("\n");
}

void ExecReport(void)                            /* $B<B9T%G!<%?$NI=<((B */
{
  printf("\nObject Code Size:%10d ins.\n", MaxPC);
  printf("Max Stack Depth: %10d\n", MaxSD);
  printf("Max Frame Size:  %10d bytes\n", DSEG_SIZE - MinFR);
  printf("Function calls:  %10d times\n", CallC);
  printf("Execution Count: %10d ins. \n\n", InsCount);
}
