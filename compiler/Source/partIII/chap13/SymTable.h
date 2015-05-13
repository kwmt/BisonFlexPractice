/* $B%W%m%0%i%`(B 13.1 : $B5-9fI=4IM}$N%X%C%@%U%!%$%k!J(BSymTable.h$B!$(B296$B%Z!<%8!K(B*/

typedef enum { NEWSYM, FUNC, F_PROT, VAR, DTYPE } Class;

typedef struct STentry{                        /* $B5-9fI=$N%(%s%H%j9=@.(B */
  unsigned char    class, attrib, deflvl,      /* $B<oN`!$=tB0@-!$%l%Y%k(B */
                   Nparam;                     /* $B0z?t$N8D?t(B */
  char            *name;                       /* $B<1JL;R$X$N%]%$%s%?(B */
  struct STentry  *next, *h_chain, *plist;     /* $B#3$D$N%j%9%H%]%$%s%?(B */
  struct TypeDesc *type;                       /* $B7?%j%9%H$X$N%]%$%s%?(B */
  int              loc;                        /* $BBP1~$9$k%"%I%l%9(B */
} STentry, *STP;

#define PARAM  0x40                            /* $B2>0z?t$NI=<((B */

typedef enum { VOID, CHAR, INT, REF, ARRAY, STRUCT, INCMP } Dtype;

typedef struct TypeDesc{                       /* $B7?%j%9%H$N%;%k(B */
  unsigned char    tcons, atoms;               /* $B7?9=@.;R!$6-3&D4@0?t(B */
  struct TypeDesc *compnt;                     /* $BMWAG$N7?(B */
  int              numelm, msize;              /* $BMWAG?t!$%a%b%j%5%$%:(B */
} TypeDesc, *TDP;

extern int GAptr, FAptr;                       /* $B%a%b%j3dIU$1%+%&%s%?(B */

#define ALIGN(X, Y) (((X+Y-1) / Y) * Y)
#define M_ALLOC(Loc, P, Sz, Bs) { P=ALIGN(P, Bs); Loc=P; P += (Sz); }

void OpenBlock(void);                          /* $B%V%m%C%/$N;O$^$j(B */
void CloseBlock(void);                         /* $B%V%m%C%/$N=*$j(B */

STP  MakeEntry(char *Name, Class C);           /* $B5-9fI=%(%s%H%j$N:n@.(B */
void TypeDef(char *Name, TDP Texpr);           /* $B7?Dj5A(B */
STP  MakeFuncEntry(char *Fname);               /* $B4X?tL>$N2>EPO?(B */
void FuncDef(STP Funcp, STP tp);               /* $B4X?tK\BN$N;O$^$j(B */
void Prototype(STP Funcp, TDP Texpr);          /* $B%W%m%H%?%$%WL>$NEPO?(B */
void EndFdecl(STP Funcp);                      /* $B4X?tDj5A$N=*$j(B */
STP  NewID(char *Name, TDP Texpr);             /* $BJQ?tL>$NEPO?(B */
STP  TypeName(char *Name);                     /* $B7?L>$NC5:w(B */
void MemAlloc(STP Var, TDP Texpr, int Param);  /* $B%a%b%j3dIU$1(B */
STP  SymRef (char *Name);                      /* $B5-9fI=$NC5:w(B */
int  AllocCons(char *ConsP, int Blen, int Nobj); /* $BDj?t$N%a%b%j3dIU$1(B */
void UndefCheck(void);                         /* $BL$Dj5A4X?t$N%A%'%C%/(B */

void BeginStruct(void);                        /* $B9=B$BN$N;O$^$j(B */
TDP  EndStruct(void);                          /* $B9=B$BN$N=*$j(B */

void PrintSymEntry(STP p);                     /* $B5-9fI=%(%s%H%j$NI=<((B */
