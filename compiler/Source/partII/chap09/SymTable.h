/* $B%W%m%0%i%`(B 9.1 : $B5-9fI=%X%C%@%U%!%$%k!J(BSymTable.h$B!$(B169$B%Z!<%8!K(B */

typedef enum {NEWSYM, FUNC, PARAM, VAR, F_PROT} Class;
typedef enum {VOID, INT} Dtype;             /* $B7?$rI=$9Ns5sDj?t(B */

typedef struct STentry{                     /* $B5-9fI=%(%s%H%j$N9=@.(B */
  unsigned char type, class;                  /* $B7?!$<oN`(B */
  unsigned char deflvl, Nparam;               /* $BDj5A%l%Y%k!$0z?t$N?t(B */
  char *name;                                 /* $B<1JL;R$X$N%]%$%s%?(B */
  int   loc;                                  /* $B%"%I%l%9(B */
} STentry, *STP;                            /* $B5-9fI=$X$N%]%$%s%?7?(B*/

void OpenBlock(void);                       /* $B%V%m%C%/$N;O$^$j(B */
void CloseBlock(void);                      /* $B%V%m%C%/$N=*$j(B */

STP  MakeFuncEntry(char *Name);             /* $B5-9fI=$X$N<1JL;R$NEPO?(B */
void FuncDef(STP Funcp, Dtype T);           /* $B4X?tDj5AL>$NEPO?(B */
void Prototype(STP Funcp, Dtype T);         /* $B4X?t%W%m%H%?%$%W$NEPO?(B */
void EndFdecl(STP Funcp);                   /* $B4X?tDj5A$N=*$j(B */
void VarDecl(char *Name, Dtype T, Class C); /* $B5-9fI=$X$NJQ?tL>$NEPO?(B */
STP  SymRef (char *Name);                   /* $B5-9fI=$NC5:w(B */
void UndefCheck(void);                      /* $BL$Dj5A4X?t$N%A%'%C%/(B */

void PrintSymEntry(STP Symp);               /* $B5-9fI=%(%s%H%j$NI=<((B */
