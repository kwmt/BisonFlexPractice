/* $B%W%m%0%i%`(B 13.2 : $B7?%j%9%H$N%X%C%@%U%!%$%k!J(BTypeDef.h$B!$(B299$B%Z!<%8!K(B */

extern STP  SymBTDP[];                     /* $B4pK\7?$N5-9fI=%(%s%H%j(B */
extern TDP  BasicTDP[];                    /* $B4pK\7?$N7?%j%9%H$NI=(B */

#define VOIDP BasicTDP[VOID]                    /* void$B$N7?%j%9%H(B*/
#define CHARP BasicTDP[CHAR]                    /* char$B$N7?%j%9%H(B*/
#define INTP  BasicTDP[INT]                     /* int$B$N7?%j%9%H(B*/
#define UNIVP BasicTDP[REF]                     /* null$B$N7?%j%9%H(B*/

#define MAKE_ARRAY(P, N) { P = MakeTdesc(ARRAY, NULL); P->numelm = N; }

void SetUpSymTab(void);                         /* $B5-9fI=$N=i4|@_Dj(B */
TDP  MakeTdesc(Dtype Constrct, TDP Texpr);      /* $B7?%;%k$N@8@.(B */
void ArrayDesc(TDP Atp, TDP ElmnT);             /* $BG[Ns7?$N%;%k$N@_Dj(B */
TDP  RefType(TDP Texpr);                        /* $B%]%$%s%?7?(B */
int  SameType(TDP T1, TDP T2);                  /* $BF1$87?$NH=Dj(B */
void CmptblType(TDP OrgT, TDP ActT);            /* $BBeF~E,9g@-$N%F%9%H(B */
void PrintType(TDP tp);                         /* $B7?%j%9%H$NI=<((B */
