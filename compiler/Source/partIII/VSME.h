/* プログラム 10.1 : VSM-Eのヘッダファイル（VSME.h，196ページ） */

typedef enum {
   NOP,   CASSGN, ASSGN,  DASSGN, ADD,    DADD,   ADDI,  SUB,    DSUB,
   SUBI,  MUL,    DMUL,   MULI,   DIV,    DDIV,   DIVI,  MOD,    DUMMY,
   MODI,  CSIGN,  DCSIGN, COMP,   DCOMP,  COMPI,  COPY,  CPUSH,  PUSH,
   DPUSH, PUSHI,  CRVAL,  RVAL,   DRVAL,  REMOVE, CPOP,  POP,    DPOP,
   SETFR, INCFR,  DECFR,  INTDBL, DBLINT, JUMP,   BLT,   BLE,    BEQ,
   BNE,   BGE,    BGT,    CALL,   RET,    HALT,   MCOPY, MALLOC, MFREE,
   INPUT, OUTPUT, OUTSTR } OP;

#define FP   0x01                       /* フレームレジスタ修飾ビット */

#define ISEG_SIZE 2000                  /* 命令セグメントの最大命令数 */
#define DSEG_SIZE 5000                  /* データセグメントの記憶容量 */

#define CBSZ sizeof(char)               /* 文字表現のバイト数 */
#define IBSZ sizeof(int)                /* 整数表現のバイト数 */
#define DBSZ sizeof(double)             /* 実数表現のバイト数 */

typedef struct {                        /* 命令の形式 */
  unsigned char Op, Reg;                /* 操作コード部，レジスタ部 */
  int      Addr;                        /* アドレス部 */
} INSTR;

#define MAX_DSEG (DSEG_SIZE * DBSZ)     /* Dsegの最終アドレス + 1 */

void SetPC(int N);                          /* Pctrのセット */
int  PC(void);                              /* Pctrの読出し */
int  StartVSM(int StartAddr, int TraceSW);  /* 実行開始 */

void SetI(OP OPcode, int Flag, int Addr);  /* 命令の書込み */
void Bpatch(int Loc, int Addr);            /* アドレス部の書換え */
void WriteDseg(int Loc, char *Data, int Bytes);    /* Dsegへの書込み */
void DumpIseg(int first, int last);        /* 命令セグメントのダンプ */
void ExecReport(void);                     /* 実行についての報告 */

#define Cout(OPcode, Addr) SetI(OPcode, 0, Addr)
#define Pout(OPcode)       SetI(OPcode, 0, 0)

extern int DebugSW;                     /* Isegへの命令書込み時の表示 */
