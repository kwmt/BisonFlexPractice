/* プログラム 6.1 : VSMヘッダファイル（VSM.h，93ページ） */

/* 仮想スタックマシンのインタフェース */

typedef enum { NOP,   ASSGN, ADD,  SUB,   MUL,   DIV,   MOD,   CSIGN,
               AND,   OR,    NOT,  COMP,  COPY,  PUSH,  PUSHI, REMOVE,
               POP,   INC,   DEC,  SETFR, INCFR, DECFR, JUMP,  BLT,
               BLE,   BEQ,   BNE,  BGE,   BGT,   CALL,  RET,   HALT,
               INPUT, OUTPUT } OP;

#define ISEG_SIZE 1000                  /* 命令セグメントの最大命令数 */
#define DSEG_SIZE 1000                  /* データセグメントの最大語数 */
#define FRAME_BOTTOM (DSEG_SIZE-1)      /* Dsegの最終アドレス */

#define FP    0x01                      /* フレームレジスタ修飾ビット */

typedef struct {                        /* 命令の形式 */
  unsigned char Op, Reg;                   /* 操作コード部，レジスタ部 */
  int      Addr;                           /* アドレス部 */
} INSTR;

void SetPC(int N);                          /* Pctrのセット */
int  PC(void);                              /* Pctrの読出し */
int  StartVSM(int StartAddr, int TraceSW);  /* 実行開始 */

void SetI(OP OPcode, int Flag, int Addr);   /* 命令の書込み */
void Bpatch(int Loc, int Addr);             /* アドレス部の書換え */
void DumpIseg(int first, int last);         /* 命令セグメントのダンプ */
void ExecReport(void);                      /* 実行についての報告 */

#define Cout(OPcode, Addr) SetI(OPcode, 0, Addr)
#define Pout(OPcode)       SetI(OPcode, 0, 0)

extern int DebugSW;                     /* Isegへの命令書込み時の表示 */
