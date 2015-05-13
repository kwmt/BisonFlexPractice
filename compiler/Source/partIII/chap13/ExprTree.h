/* $B%W%m%0%i%`(B 14.1 : $B<0$NLZ$N%X%C%@%U%!%$%k!J(BExprTree.h$B!$(B320$B%Z!<%8!K(B */

enum { AELM = 100, REFOBJ, ADDROP, MEMOP, INMEM, ARGL, CONS, IDS };

typedef struct Node {
  unsigned char Op, etc;
  TDP           type;
  struct Node  *left, *right;
} Node, *NP;

#define SYMP(P) (STP)((P)->left)
#define VAR_NODE(X, Y) { X = MakeL(Y);  \
           if ((SYMP(X))->class != VAR) yyerror("Non-variable name"); }
#define CONS_NODE(X, T, D) { X = AllocNode(CONS, (NP)(D), NULL); \
                             (X)->type = T; }

NP   AllocNode(int Op, NP left, NP right);
NP   MakeL(char *Name);
NP   MakeN(int Op, NP left, NP right);
NP   RecSelect(int Op, NP tree, char *fname);
void WriteTree(NP np);
void FreeSub(NP tree);
