/* プログラム 7.1 : ExprLコンパイラ（ExplrL.y，118, 119ページ） */

%{
#include "../VSM.h"

void SymDecl(char *);
int  SymRef(char *);
%}

%union {
  int   Int;
  char *Name;
}

%token TYPE READ WRITE
%token <Int> ADDOP MULOP PPMM RELOP NUM
%token <Name> ID

%right '='
%right '?' ':'
%left  LOR
%left  LAND
%left  RELOP
%left  ADDOP
%left  MULOP
%right '!' PPMM UM
%left  POSOP

%%

program   : decl_list s_list     { Pout(HALT); }
          ;
decl_list :
          | decl_list decl ';'
          ;
decl      : TYPE ID              { SymDecl($2); }
          | decl ',' ID          { SymDecl($3); }
          ;
s_list    : stmnt   
          | s_list stmnt
          ;
stmnt     : expr ';'             { Pout(REMOVE); }
          | READ LHS ';'         { Pout(INPUT); }
          | WRITE expr ';'       { Pout(OUTPUT); }
          | error ';'            { yyerrok; }
          ;
LHS       : ID                   { Cout(PUSHI, SymRef($1)); }
          ;
expr      : LHS  '=' expr        { Pout(ASSGN); }
          | expr '?'             { $<Int>$ = PC(); Cout(BEQ, -1); }
              expr ':'           { $<Int>$ = PC(); Cout(JUMP, -1);
                                   Bpatch($<Int>3, PC()); }
              expr               { Bpatch($<Int>6, PC()); }
          | expr LOR expr        { Pout(OR); }
          | expr LAND expr       { Pout(AND); }
          | expr RELOP expr      { Pout(COMP); Cout($2, PC()+3);
                                   Cout(PUSHI, 0), Cout(JUMP, PC()+2);
                                   Cout(PUSHI, 1); }
          | expr ADDOP expr      { Pout($2); }
          | expr MULOP expr      { Pout($2); }
          | '(' expr ')'
          | '!' expr             { Pout(NOT); }
          | ADDOP expr %prec UM  { if ($1==SUB) Pout(CSIGN); }
          | PPMM ID              { int addr = SymRef($2);
                                   Cout(PUSH, addr); Pout($1);
                                   Pout(COPY); Cout(POP, addr); }
          | ID PPMM %prec POSOP  { int addr = SymRef($1);
                                   Cout(PUSH, addr); Pout(COPY);
                                   Pout($2); Cout(POP, addr); }
          | ID                   { Cout(PUSH, SymRef($1)); }
          | NUM                  { Cout(PUSHI, $1); }
          ;
%%
