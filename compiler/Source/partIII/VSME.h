/* $B%W%m%0%i%`(B 10.1 : VSM-E$B$N%X%C%@%U%!%$%k!J(BVSME.h$B!$(B196$B%Z!<%8!K(B */

typedef enum {
   NOP,   CASSGN, ASSGN,  DASSGN, ADD,    DADD,   ADDI,  SUB,    DSUB,
   SUBI,  MUL,    DMUL,   MULI,   DIV,    DDIV,   DIVI,  MOD,    DUMMY,
   MODI,  CSIGN,  DCSIGN, COMP,   DCOMP,  COMPI,  COPY,  CPUSH,  PUSH,
   DPUSH, PUSHI,  CRVAL,  RVAL,   DRVAL,  REMOVE, CPOP,  POP,    DPOP,
   SETFR, INCFR,  DECFR,  INTDBL, DBLINT, JUMP,   BLT,   BLE,    BEQ,
   BNE,   BGE,    BGT,    CALL,   RET,    HALT,   MCOPY, MALLOC, MFREE,
   INPUT, OUTPUT, OUTSTR } OP;

#define FP   0x01                       /* $B%U%l!<%`%l%8%9%?=$>~%S%C%H(B */

#define ISEG_SIZE 2000                  /* $BL?Na%;%0%a%s%H$N:GBgL?Na?t(B */
#define DSEG_SIZE 5000                  /* $B%G!<%?%;%0%a%s%H$N5-21MFNL(B */

#define CBSZ sizeof(char)               /* $BJ8;zI=8=$N%P%$%H?t(B */
#define IBSZ sizeof(int)                /* $B@0?tI=8=$N%P%$%H?t(B */
#define DBSZ sizeof(double)             /* $B<B?tI=8=$N%P%$%H?t(B */

typedef struct {                        /* $BL?Na$N7A<0(B */
  unsigned char Op, Reg;                /* $BA`:n%3!<%IIt!$%l%8%9%?It(B */
  int      Addr;                        /* $B%"%I%l%9It(B */
} INSTR;

#define MAX_DSEG (DSEG_SIZE * DBSZ)     /* Dseg$B$N:G=*%"%I%l%9(B + 1 */

void SetPC(int N);                          /* Pctr$B$N%;%C%H(B */
int  PC(void);                              /* Pctr$B$NFI=P$7(B */
int  StartVSM(int StartAddr, int TraceSW);  /* $B<B9T3+;O(B */

void SetI(OP OPcode, int Flag, int Addr);  /* $BL?Na$N=q9~$_(B */
void Bpatch(int Loc, int Addr);            /* $B%"%I%l%9It$N=q49$((B */
void WriteDseg(int Loc, char *Data, int Bytes);    /* Dseg$B$X$N=q9~$_(B */
void DumpIseg(int first, int last);        /* $BL?Na%;%0%a%s%H$N%@%s%W(B */
void ExecReport(void);                     /* $B<B9T$K$D$$$F$NJs9p(B */

#define Cout(OPcode, Addr) SetI(OPcode, 0, Addr)
#define Pout(OPcode)       SetI(OPcode, 0, 0)

extern int DebugSW;                     /* Iseg$B$X$NL?Na=q9~$_;~$NI=<((B */
