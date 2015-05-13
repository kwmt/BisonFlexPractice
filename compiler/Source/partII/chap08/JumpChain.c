/* $B%W%m%0%i%`(B 8.2 : $BJ,4t@h$N4IM}!J(BJumpChain.c$B!$(B151$B%Z!<%8!K(B */

#include "../VSM.h"

#define CTRLT_SIZE 20                          /* $BJ,4t%9%?%C%/%5%$%:(B */
struct {
  int StType;                                  /* $BJ8$N%3!<%I(B*/
  int Bchain, Cchain;                          /* $BJ,4t@h%j%9%H%X%C%@(B */
} Ctable[CTRLT_SIZE];                          /* $BJ,4t%9%?%C%/(B */

static int CSptr = 0;                          /* $BJ,4t%9%?%C%/%]%$%s%?(B */

void NestIn(int St)                            /* $B7+JV$7J8!$(Bswitch$BJ8(B */
{                                              /* $B$N;O$^$j(B */
  Ctable[++CSptr].StType = St;
  Ctable[CSptr].Bchain = Ctable[CSptr].Cchain = -1;
}

void GenBrk(int JC)                            /* break$BJ8$N=hM}(B */
{
  if (CSptr > 0) {
    Cout(JC, Ctable[CSptr].Bchain);            /* $BJ,4tL?Na$N@8@.(B */
    Ctable[CSptr].Bchain = PC()-1; }           /* break$B%A%'%$%s$N99?7(B */
  else
    yyerror("Illegal use of break statement");
}

void GenConti(void)                            /* continue$BJ8$N=hM}(B */
{
  int i;

  for (i=CSptr; i>0 && Ctable[i].StType==0; i--) ;   /* switch$BJ8(B ? */
  if (i <= 0)                                  /* $B%9%?%C%/$,6u(B ? */
    yyerror("Illegal use of the continue");     
  else {
    Cout(JUMP, Ctable[i].Cchain);              /* $BJ,4tL?Na$N@8@.(B */
    Ctable[i].Cchain = PC() - 1; }             /* $B8eKd$a%A%'%$%s99?7(B */
}

void NestOut(int ContP)                        /* $B7+JV$7J8!$(Bswitch$BJ8(B */
{                                              /* $B$N=*$j(B */
    Bpatch(Ctable[CSptr].Cchain, ContP);       /* continue$BJ8$N8eKd$a(B */
    Bpatch(Ctable[CSptr].Bchain, PC());        /* break$BJ8$N8eKd$a(B */
  CSptr--;
}

