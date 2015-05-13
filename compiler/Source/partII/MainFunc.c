/* $B%W%m%0%i%`(B 7.5 : $B4X?t(Bmain() $B!J(BMainFunc.c$B!$(B134, 135$B%Z!<%8!K(B */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "VSM.h"

int        StartP=0, SymPrintSW=0;                  /* $B%*%W%7%g%sMQ$N(B */
static int ExecSW=1, ObjOutSW=0, TraceSW=0, StatSW=0;   /* $B%U%i%0JQ?t(B */

static int  ErrorC=0;                               /* $B%(%i!<%+%&%s%?(B */
static char SourceFile[20];

extern FILE *yyin;                                  /* $BF~NO%U%!%$%k(B */
                                                    /* $B%]%$%s%?(B */
main(int argc, char *argv[])
{
  static void SetUpOpt(int, char *[]);

  SetUpOpt(argc, argv);                             /* $B%*%W%7%g%s$N=hM}(B */
  if (SourceFile[0] != NULL)                        /* $BF~NO%U%!%$%k$r(B */
    if ((yyin=fopen(SourceFile, "r")) == NULL) {         /* $B%*!<%W%s(B */
      fprintf(stderr, "Source file cannot be opened.");
      exit(-1); }                                   /* $B%3%s%Q%$%kCf;_(B */
  yyparse();                                        /* $B%3%s%Q%$%k(B */
  if (ObjOutSW) DumpIseg(0, PC()-1);                /* $BL\E*%3!<%II=<((B */
  if (ExecSW || TraceSW)
    if (ErrorC == 0) {                              /* $BK]Lu%(%i!<$,(B */
      printf("Enter execution phase\n");            /* $B$"$C$?(B ? */
      if (SourceFile[0] == NULL)                    /* $B%-!<%\!<%I(B ? */
        rewind(stdin);                              /* $BF~NO$N:F@_Dj(B */
      if (StartVSM(StartP, TraceSW) != 0)           /* $B%W%m%0%i%`<B9T(B */
        printf("Execution aborted\n");
      if (StatSW) ExecReport(); }                   /* $B<B9T>u67$NJs9p(B */
    else
      printf("Execution suppressed\n");
}

static void SetUpOpt(int argc, char *argv[])
{
  char *s;

  if (--argc>0 && (*++argv)[0]=='-') {             /* $B%*%W%7%g%s;XDj(B ? */
    for (s = *argv+1; *s != '\0'; s++)             /* $B%*%W%7%g%s$NAv::(B */
      switch(tolower(*s)) {                        /* $B>.J8;z$G%A%'%C%/(B */
        case 'c': StatSW = 1;     break;           /* $B<B9T%G!<%?$NI=<((B */
        case 'd': DebugSW = 1;    break;           /* $B%G%P%C%0%b!<%I(B */
        case 'n': ExecSW = 0;     break;           /* $B%3%s%Q%$%k$@$1(B */
        case 'o': ObjOutSW = 1;   break;           /* $BL\E*%3!<%I$NI=<((B */
        case 's': SymPrintSW = 1; break;           /* $B5-9fI=$NI=<((B */
        case 't': TraceSW = 1;    break; }         /* $B%H%l!<%9%b!<%I(B */
    argc--; argv++; }
  if (argc > 0)                                    /* $BF~NO%U%!%$%kL>$r(B */
    strcpy(SourceFile, *argv);                     /* $B%3%T!<(B */
}

yyerror(char *msg)                          /* $B%(%i!<%a%C%;!<%8=PNO4X?t(B */
{
  extern int yylineno;                      /* $B8=:_$N9THV9f(B*/

  printf("%s at line %d\n", msg, yylineno);
  ErrorC++;
}
