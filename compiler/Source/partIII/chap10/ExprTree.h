/* $B%W%m%0%i%`(B 11.1 : $B<0$NLZ$N%X%C%@%U%!%$%k!J(BExprTree.h$B!$(B229$B%Z!<%8!K(B */

enum { AND=100, OR, NOT, INC, DEC, AELM, SUBL, ARGL, CONS, IDS };

typedef struct Node {                       /* $B<0$NLZ$N@aE@$NDj5A(B */
  unsigned char Op, type, etc;              /* $B1i;;;R!$7?!$B0@-(B */
  struct Node *left, *right;                /* $B:81&$NItJ,LZ$N%]%$%s%?(B */
} Node, *NP;                                /* $B@aE@$H@aE@$N%]%$%s%?7?(B */

typedef struct {                            /* $BDj?tI=%(%s%H%j(B */
  double Dcons;                             /* $BCM(B */
  int    addr;                              /* Dseg$B%"%I%l%9(B */
} DCdesc;

extern DCdesc DCtbl[];                      /* $BDj?tI=(B */

#define INTV(P) (int)((P)->left)            /* int$B7?$X$NJQ49(B */
#define SYMP(P) (STP)((P)->left)            /* $B5-9fI=$X$N%]%$%s%?(B */
#define MakeC(X, T, D) { (X) = AllocNode(CONS, (NP)(D), NULL);\
                            (X)->type = T; }
#define INTCHK(X) { if ((X->type) != INT && (X->type) != CHAR)\
                              yyerror("Non-integer type expression"); }
#define VAR_NODE(X, Y) { X = MakeL(Y);  \
           if ((SYMP(X))->class != VAR) yyerror("Non-variable name"); }

NP   AllocNode(int Op, NP left, NP right);  /* $B@aE@MQ$N%a%b%j$N3MF@(B */
NP   MakeL(char *Name);                     /* $BMU$N:n@.(B */
NP   MakeN(int Op, NP left, NP right);      /* $B@aE@$N:n@.(B */
NP   TypeConv(NP tree, Dtype T);            /* $B7?JQ49(B */
int  Wcons(double value);                   /* $BDj?tI=$X$NEPO?(B */
void WriteTree(Node *root);                 /* $B<0$NLZ$NI=<((B */
void FreeSubT(NP tree);                     /* $B<0$NLZ$N%a%b%j3+J|(B */
