/* $B%W%m%0%i%`(B 11.2 : $B%3!<%I@8@.$N%X%C%@%U%!%$%k!J(BCodeGen.h$B!$(B232$B%Z!<%8!K(B */

enum { FJ, TJ };                            /* $BJ,4t>r7o(B */
enum { PRE, POST };                         /* $BA0CV!$8eCV1i;;;R(B */

void  GenFuncEntry(STP FuncP);              /* $B4X?t$N=PF~8}$N%3!<%I(B */
void  ExprSmnt(NP tree);                    /* $B<0J8$N%3!<%I@8@.(B */
int   CtrlExpr(NP tree, int jc);            /* $B@)8fJ8$N%3!<%I@8@.(B */
void  GenReturn(STP func, NP tree);         /* return$BJ8$N%3!<%I@8@.(B */
void  Epilogue(void);                       /* $B%3%s%Q%$%k=*N;;~$N=hM}(B */
