/* $B%W%m%0%i%`(B 10.2 : $B5-9fI=4IM}$N%X%C%@%U%!%$%k!J(BSymTable.h$B!$(B204$B%Z!<%8!K(B */

typedef enum { NEWSYM, FUNC, F_PROT, VAR } Class;
typedef enum { VOID, CHAR, INT, DBL, ARY=0x10, C_ARY, } Dtype;

typedef struct STentry{                     /* $B5-9fI=$N%(%s%H%j9=@.(B */
  unsigned char type,   class,                 /* $B7?!$<oN`(B */
                attrib, deflvl,                /* $B=tB0@-!$Dj5A%l%Y%k(B */
                dim,    Nparam;                /* $B<!85?t!$0z?t$N8D?t(B */
  char         *name;                          /* $B<1JL;R$X$N%]%$%s%?(B */
  int           loc, *dlist;                   /* $B%"%I%l%9!$(B*$B%5%$%:I=(B */
  struct STentry *h_chain;                     /* $B>WFM%j%9%H$N%]%$%s%?(B */
} STentry, *STP;

#define SALLOC 0x80                         /* $B@EE*3dIU$1$NI=<((B */
#define PARAM  0x40                         /* $B2>0z?t$NI=<((B */
#define BYREF  0x20                         /* $B;2>HEO$7$NI=<((B */
#define DUPFID 0x10                         /* $B4X?tL>$N@k8@$HDj5A(B */

#define ALLOCNUM(C, T) AllocCons((char *)(&C), T, 1)  /* $B?tCM$N3dIU$1(B */

extern int ByteW[], AStable[];              /* $B%P%$%H?tI=!$3FMWAG?t(B */

void OpenBlock(void);                       /* $B%V%m%C%/$N;O$^$j(B */
void CloseBlock(void);                      /* $B%V%m%C%/$N=*$j(B */

STP  MakeFuncEntry(char *Fname);            /* $B4X?tL>$N2>EPO?(B */
void FuncDef(STP Funcp, Dtype T);           /* $B4X?tDj5AL>$NEPO?(B */
void Prototype(STP Funcp, Dtype T);         /* $B4X?t%W%m%H%?%$%W$NEPO?(B */
void EndFdecl(STP Funcp);                   /* $B4X?tDj5A$N=*$j(B */
STP  VarDecl(char *Name, int Dim);          /* $B5-9fI=$X$NJQ?tL>$NEPO?(B */
void MemAlloc(STP Symp, Dtype T, int Parm); /* $B%a%b%j3dIU$1(B */
STP  SymRef (char *Name);                   /* $B5-9fI=$NC5:w(B */
int  AllocCons(char *cp, int bl, int Nobj); /* $BDj?t$N%a%b%j3dIU$1(B */

void UndefCheck(void);                      /* $BL$Dj5A4X?t$N%A%'%C%/(B */
void PrintSymEntry(STP Symp);               /* $B5-9fI=%(%s%H%j$NI=<((B */
