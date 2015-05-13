/* プログラム 6.4 : VSMシミュレータ（VSM.c，109ページ） */

#include <stdio.h>
#include "VSM.h"

int DebugSW=0;                                   /* デバッグモード表示 */

static int Pctr=0, SP=0, Freg=0;
static int InsCount=0, MaxSD=0, MinFR=DSEG_SIZE, MaxPC=0, CallC=0;

static INSTR Iseg[ISEG_SIZE];                    /* 命令セグメント */
static int   Dseg[DSEG_SIZE];                    /* データセグメント */

#define STACK_SIZE 100
static int Stack[STACK_SIZE];                    /* オペランドスタック */

char *Scode[] = {                                /* 表示用の操作コード */
                 "Nop",    "  =",   "  +",    "  -",    "  *",  "  /",
                 "  %",    "  -'",  "and",    "or",     "not",  "comp",
                 "copy",   "push",  "push-i", "remove", "pop",  " ++",
                 " --",    "setFR", "++FR",   "--FR",   "jump", "<0 ?",
                 "<=0 ?",  "==0 ?", "!=0 ?",  ">=0 ?",  ">0 ?", "call",
                 "return", "halt",  "input",  "output" };

static void PrintIns(int loc)                    /* 命令の記号編集と */
{                                                /* 表示 */
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
    if (DebugSW) {
      PrintIns(Pctr); printf("\n"); }
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

#define BINOP(OP) {Stack[SP-1] = Stack[SP-1] OP Stack[SP]; SP--;}

int StartVSM(int StartAddr, int TraceSW)         /* VSMの命令実行 */
{
  int addr, op;

  Pctr = StartAddr;                              /* Pctrの設定 */
  SP = Freg = 0;                                 /* レジスタの設定 */
  while (1) {
    if (SP >= STACK_SIZE || SP < 0) {            /* SPの範囲チェック */
      fprintf(stderr, "Illegal Stack pointer %d\n", SP);
      return -1; }
    op   = Iseg[Pctr].Op;                        /* 命令のフェッチ */
    addr = Iseg[Pctr].Addr;                      /* 実効アドレス */
    if (Iseg[Pctr++].Reg & FP)                   /* FPレジスタ修飾 ? */
      addr += Freg;                              /* FPを加算 */
    InsCount++;                                  /* 総実行命令数 */
    if (SP > MaxSD) MaxSD = SP;                  /* SPの最大値 */
    if (TraceSW) {                               /* 命令実行トレース */
      PrintIns(Pctr-1);
      printf("%15d %5d %12d\n", addr, SP, Stack[SP]); }
    switch (op) {                                /* 各命令の実行 */
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

void DumpIseg(int first, int last)               /* Isegの表示 */
{
  printf("\nContents of Instruction Segment\n");
  for (; first<=last; first++)
    PrintIns(first),  printf("\n");
  printf("\n");
}

void ExecReport(void)                            /* 実行データの表示 */
{
  printf("\nObject Code Size:%10d ins.\n", MaxPC);
  printf("Max Stack Depth: %10d\n", MaxSD);
  printf("Max Frame Size:  %10d bytes\n", DSEG_SIZE - MinFR);
  printf("Function calls:  %10d times\n", CallC);
  printf("Execution Count: %10d ins. \n\n", InsCount);
}
