/* $B%W%m%0%i%`(B 9.2 : FuncL$B%3%s%Q%$%i!J(BFuncL.y$B!$(B172, 174, 175, 178$B%Z!<%8!K(B */

%{
#include "../VSM.h"
#include "SymTable.h"

#define TCHK(X) { if ((X)==VOID) yyerror("Type error"); }

char  *IDentry(char *Name, int len);            /* NameTable.c$B$GDj5A(B */
Dtype  VarRef(int OPcode, char *Vname);         /* $BJQ?t;2>HL?Na$N@8@.(B */
void   GenFuncEntry(STP func);                  /* $B=PF~8}$NL?Na@8@.(B */
Dtype  GenCall(char *Fname, int Nparam);        /* $B4X?t8F=P$7$NL?NaNs(B */

STP FuncP;                                      /* $BK]LuCf$N4X?t(B */

extern int StartP;                              /* $B<B9T3+;O%"%I%l%9(B */
%}

%union{
  int   Int;
  char *Name;                                   /* $B<1JL;R$X$N%]%$%s%?(B */
  STP   SymP;                                   /* $B5-9fI=$X$N%]%$%s%?(B */
}

%token        IF   ELSE  READ  WRITE RETURN
%token <Int>  TYPE RELOP ADDOP MULOP NUM
%token <Name> ID

%type <Int>   decl expr LHS arg_list if_part
%type <SymP>  f_head

%right '='
%left  RELOP
%left  ADDOP
%left  MULOP
%right UM

%%

program  : glbl_decl            { UndefCheck(); StartP = PC();
                                  Cout(SETFR, FRAME_BOTTOM);
                                  GenCall(IDentry("main", 4), 0);
                                  Pout(HALT); }
         ;
glbl_decl:                      /* $B6u5,B'(B */
         | glbl_decl decl ';'
         | glbl_decl func_def
         | glbl_decl error ';'  { yyerrok; }
         ;
decl     : TYPE ID              { VarDecl($2, $1, VAR); }
         | TYPE f_head          { Prototype($2, $1); }
         | decl ',' ID          { VarDecl($3, $1, VAR); }
         | decl ',' f_head      { Prototype($3, $1); }
         ;
f_head   : ID                   { $<SymP>$ = MakeFuncEntry($1); }
             '(' p_list ')'     { $$ = $<SymP>2; }
         ;
p_list   :                      /* $B6u5,B'(B */
         | p_decl
         | p_list ',' p_decl
         ;
p_decl   : TYPE ID              { VarDecl($2, $1, PARAM); }
         ;
func_def : TYPE f_head '{'      { FuncDef($2, $1); FuncP = $2;
                                  GenFuncEntry($2); }
             decl_list
             st_list '}'        { if ($2->type == INT)
                                    Cout(PUSHI, 0);
                                  Cout(JUMP, ($2->loc)-3);
                                  EndFdecl($2); }
         ;
block    : '{'                  { OpenBlock(); }
             decl_list st_list
           '}'                  { CloseBlock(); }
         ;
decl_list:                      /* $B6u5,B'(B */
         | decl_list decl ';'
         ;
st_list  : stmnt
         | st_list stmnt
         ;
stmnt    : block
         | expr ';'             { if ($1 != VOID) Pout(REMOVE); }
         | READ LHS ';'         { Pout(INPUT); }
         | WRITE expr ';'       { Pout(OUTPUT); TCHK($2); }
         | if_part              { Bpatch($1, PC()); }
         | if_part ELSE         { $<Int>$ = PC(); Cout(JUMP, -1);
                                  Bpatch($1, PC()); }
             stmnt              { Bpatch($<Int>3, PC()); }
         | RETURN ';'           { if (FuncP->type != VOID)
                                    Cout(PUSHI, 0);
                                  Cout(JUMP, (FuncP->loc)-3); }
         | RETURN expr ';'      { if (FuncP->type != INT)
                                    yyerror("Meaningless expression");
                                  Cout(JUMP, (FuncP->loc)-3);
                                  TCHK($2); }
         ;
if_part  : IF '(' expr ')'      { $<Int>$ = PC(); Cout(BEQ, -1);
                                  TCHK($3); }
             stmnt              { $$ = $<Int>5; }
         ;
LHS      : ID                   { $$ = VarRef(PUSHI, $1); }
         ;
expr     : LHS '=' expr         { Pout(ASSGN); TCHK($3); }
         | expr RELOP expr      { TCHK($1); TCHK($3); Pout(COMP);
                                  Cout($2, PC()+3);   Cout(PUSHI, 0);
                                  Cout(JUMP, PC()+2); Cout(PUSHI, 1); }
         | expr ADDOP expr      { Pout($2); TCHK($1); TCHK($3); }
         | expr MULOP expr      { Pout($2); TCHK($1); TCHK($3); }
         | ADDOP expr %prec UM  { if ($1 == SUB) Pout(CSIGN); }
         | '(' expr ')'         { $$ = $2; }
         | ID '(' arg_list ')'  { $$ = GenCall($1, $3); }
         | ID                   { $$ = VarRef(PUSH, $1); }
         | NUM                  { Cout(PUSHI, $1); $$ = INT; }
         ;
arg_list :                      { $$ = 0; }
         | expr                 { $$ = 1; TCHK($1); }
         | arg_list ',' expr    { $$ = $1 +1; TCHK($3); }
         ;
%%

void GenFuncEntry(STP func)                     /* $B=PF~8}$N%3!<%I(B */
{
  int n;

  SetI(PUSH, FP, 0);   Cout(INCFR, -1);  Pout(RET); /* $B=P8}$N%3!<%I(B */
  Cout(DECFR, PC()-2); SetI(POP, FP, 0);            /* $BF~8}$N%3!<%I(B */
  for (n = func->Nparam; n > 0; n--)            /* $B0z?t$NCM$r%3%T!<(B */
    SetI(POP, FP, n);                           /* $B$9$k$?$a$N%3!<%I(B */
}

Dtype VarRef(int OPcode, char *Vname)           /* $BJQ?t;2>HL?Na$N@8@.(B */
{
  STP p;

  if ((p=SymRef(Vname)) != NULL)                /* $B@k8@:Q$_$NJQ?t(B ? */
    if (p->class != VAR && p->class != PARAM)   /* $BJQ?t$X$N;2>H(B ? */
      yyerror("Function name is used as a variable");
  SetI(OPcode, p->deflvl? FP : 0, p->loc);      /* PUSH/PUSHI$B$N@8@.(B */
  return p->type;
}

Dtype GenCall(char *Fname, int Nparam)          /* $B4X?t8F=P$7$N%3!<%I(B */
{
  STP p;

  if ((p=SymRef(Fname)) == NULL)                /* $BL$Dj5A$N<1JL;R(B ? */
    return VOID;
  if (p->class == FUNC || p->class == F_PROT) { /* $B4X?tL>(B ? */
    if (p->Nparam != Nparam)                    /* $B<B0z?t$H2>0z?t$N(B */
      yyerror("Number of arguments mismatched");    /* $B8D?t$,F1$8(B ? */
    Cout(CALL, p->loc);
    if (p->class == F_PROT)                     /* $B%W%m%H%?%$%W4X?tL>(B */
      p->loc = PC() -1; }                       /* $B8eKd$a%A%'%$%s99?7(B */
  else
    yyerror("Non-function call");
  return p->type;
}
