/* プログラム 12.2 : VSME-Eシミュレータ（VSME.c，281〜285ページ） */

#include <stdio.h>
#include "VSME.h"

extern int GAptr;                                /* 静的割付けカウンタ */

int    DebugSW = 0;                              /* デバッグスイッチ */
static int Pctr=0, SP=0, Freg=0;
static int InsCount=0, MaxSD=0, MinFP=MAX_DSEG, MaxPC=0, CallC=0;

static INSTR Iseg[ISEG_SIZE];                    /* 命令セグメント */

static double Dmemory[DSEG_SIZE];                /* データセグメント */
static char * const Dseg = (char *)Dmemory;      /* バイト構成のメモリ */

#define S_SIZE 100
union OpdS{                                      /* オペランドスタック */
  int I;                                              /* 整数型の要素 */
  double D;                                           /* 実数型の要素 */
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

static void PrintIns(int loc)                    /* 命令の記号編集と */
{                                                /* 表示 */
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

void SetPC(int Addr)                             /* プログラムカウンタ */
{                                                /* のセット */
  Pctr = Addr;
}

int PC(void)                                     /* プログラムカウンタ */
{                                                /* の値の読出し */
  return Pctr;
}

void SetI(OP OpCode, int F, int Addr)            /* 命令書込み */
{
    Iseg[Pctr].Op = OpCode;                      /* 操作コード */
    Iseg[Pctr].Reg = F;                          /* レジスタフラグ */
    Iseg[Pctr].Addr = Addr;                      /* アドレス */
    if (DebugSW) { PrintIns(Pctr); printf("\n"); }
    if (++Pctr > MaxPC) MaxPC = Pctr;            /* 命令数のカウント */
}

void Bpatch(int Loc, int Target)                 /* アドレス部の後埋め */
{
  while (Loc >= 0) { int p;
    if ((p=Iseg[Loc].Addr) == Loc) {
      printf("Trying to rewrite self address part at loc. %d\n", p);
      return; }
    Iseg[Loc].Addr = Target;
    Loc = p; }
}

void WriteDseg(int Loc, char *Data, int Bytes)   /* Dsegへの書込み */
{
  for (; Bytes>0; Bytes--)
    Dseg[Loc++] = *(Data++);
}

#define BINOP(T, OP, R) { S[SP-1].R = S[SP-1].T OP S[SP].T; }
#define INTP(X) ((int *)(Dseg + (X)))
#define DBLP(X) ((double *)(Dseg + (X)))
#define ZERODIV(X) if (S[SP].X == 0) \
                   { fprintf(stderr, "zero divide\n"); return -1; } 

int StartVSM(int StartAddr, int TraceSW)         /* VSMの命令実行 */
{
  char *from, *to;
  int addr, op, c, *ip;
  double *dp;

  Pctr = StartAddr;                              /* Pctrの設定 */
  while (1) {                                    /* 命令実行サイクル */
    if (Pctr < 0 || Pctr >= ISEG_SIZE) {         /* Pctr範囲チェック */
      fprintf(stderr, "Illegal instr. address : %d\n", Pctr);
      return -1; }
    if (SP >= S_SIZE || SP < 0) {                /* SPの範囲チェック */
      fprintf(stderr, "Illegal Stack pointer %d\n", SP);
      return -1; }
    op   = Iseg[Pctr].Op;                        /* 命令のフェッチ */
    addr = Iseg[Pctr].Addr;
    if (Iseg[Pctr++].Reg != 0) addr += Freg;     /* レジスタ修飾 */
    InsCount++;
    if (SP > MaxSD) MaxSD = SP;                  /* SPの最大値 */
    if (TraceSW) {                               /* 命令実行トレース */
      PrintIns(Pctr-1);
      printf("%15d %5d %12d %12d\n", addr, SP, S[SP].I, S[SP-1].I); }
    switch (op) {                                /* 各命令の実行 */
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

void DumpIseg(int first, int last)               /* Isegの表示 */
{
  printf("\nContents of Instruction Segment\n");
  for (; first <= last; first++)
    PrintIns(first),  printf("\n");
  printf("\n");
}

void ExecReport(void)                            /* 実行データの表示 */
{
  printf("\nObject Code Size:%7d instructions\n", MaxPC);
  printf("Global Data Area:%7d bytes\n", GAptr);
  printf("Max Stack Depth: %7d\n", MaxSD);
  printf("Max Frame Size:  %7d bytes\n", MAX_DSEG - MinFP);
  printf("Function Calls:  %7d times\n", CallC);
  printf("Execution Count: %7d ins. \n\n", InsCount);
}
