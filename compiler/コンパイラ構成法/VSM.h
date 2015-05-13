/* $B%W%m%0%i%`(B 6.1 : VSM$B%X%C%@%U%!%$%k!J(BVSM.h$B!$(B93$B%Z!<%8!K(B */

/* $B2>A[%9%?%C%/%^%7%s$N%$%s%?%U%'!<%9(B */

typedef enum { NOP,   ASSGN, ADD,  SUB,   MUL,   DIV,   MOD,   CSIGN,
               AND,   OR,    NOT,  COMP,  COPY,  PUSH,  PUSHI, REMOVE,
               POP,   INC,   DEC,  SETFR, INCFR, DECFR, JUMP,  BLT,
               BLE,   BEQ,   BNE,  BGE,   BGT,   CALL,  RET,   HALT,
               INPUT, OUTPUT } OP;

#define ISEG_SIZE 1000                  /* $BL?Na%;%0%a%s%H$N:GBgL?Na?t(B */
#define DSEG_SIZE 1000                  /* $B%G!<%?%;%0%a%s%H$N:GBg8l?t(B */
#define FRAME_BOTTOM (DSEG_SIZE-1)      /* Dseg$B$N:G=*%"%I%l%9(B */

#define FP    0x01                      /* $B%U%l!<%`%l%8%9%?=$>~%S%C%H(B */

typedef struct {                        /* $BL?Na$N7A<0(B */
  unsigned char Op, Reg;                   /* $BA`:n%3!<%IIt!$%l%8%9%?It(B */
  int      Addr;                           /* $B%"%I%l%9It(B */
} INSTR;

void SetPC(int N);                          /* Pctr$B$N%;%C%H(B */
int  PC(void);                              /* Pctr$B$NFI=P$7(B */
int  StartVSM(int StartAddr, int TraceSW);  /* $B<B9T3+;O(B */

void SetI(OP OPcode, int Flag, int Addr);   /* $BL?Na$N=q9~$_(B */
void Bpatch(int Loc, int Addr);             /* $B%"%I%l%9It$N=q49$((B */
void DumpIseg(int first, int last);         /* $BL?Na%;%0%a%s%H$N%@%s%W(B */
void ExecReport(void);                      /* $B<B9T$K$D$$$F$NJs9p(B */

#define Cout(OPcode, Addr) SetI(OPcode, 0, Addr)
#define Pout(OPcode)       SetI(OPcode, 0, 0)

extern int DebugSW;                     /* Iseg$B$X$NL?Na=q9~$_;~$NI=<((B */
