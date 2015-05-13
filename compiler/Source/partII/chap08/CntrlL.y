/* プログラム 8.1 : CntrlLコンパイラ（CntrlL.y，147〜149ページ） */

%{
#include "../VSM.h"

void SymDecl(char *);
int  SymRef(char *);

void NestIn(int),   NestOut(int), GenBrk(int),  GenConti(void),
     BeginSW(void), EndSW(void),  CaseLbl(int), DfltLbl(void);
%}

%union{
  int   Int;
  char *Name;
}

%token        TYPE IF ELSE WHILE DO FOR
%token        SWITCH CASE DEFAULT BREAK CONTI READ WRITE
%token <Int>  RELOP ADDOP MULOP PPMM NUM
%token <Name> ID

%type  <Int>  if_part opt_expr tst_expr

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
decl_list :                      /* 空規則*/
          | decl_list decl ';'
          ;
decl      : TYPE ID              { SymDecl($2); }
          | decl ',' ID          { SymDecl($3); }
          ;
s_list    : stmnt
          | s_list stmnt
          ;
stmnt     : lbl_stmnt
          | ';'
          | expr ';'             { Pout(REMOVE); }
          | READ LHS ';'         { Pout(INPUT); }
          | WRITE expr ';'       { Pout(OUTPUT); }
          | '{' s_list '}'
          | if_part              { Bpatch($1, PC()); }
          | if_part ELSE         { $<Int>$ = PC(); Cout(JUMP, -1);
                                   Bpatch($1, PC()); }
              stmnt              { Bpatch($<Int>3, PC()); }
          | WHILE                { NestIn(WHILE); $<Int>$ = PC(); }
              '(' expr ')'       { GenBrk(BEQ); }
              stmnt              { GenConti(); NestOut($<Int>2); }
          | DO                   { NestIn(DO); $<Int>$ = PC(); }
              stmnt              { $<Int>$ = PC(); }
              WHILE '(' expr ')' { Cout(BNE, $<Int>2); }
              ';'                { NestOut($<Int>4); }
          | FOR '(' opt_expr ';' { NestIn(FOR); }
              tst_expr ';'       { Cout(BNE, -1);  GenBrk(JUMP); }
              opt_expr ')'       { Cout(JUMP, $3); Bpatch($6, PC()); }
              stmnt              { GenConti(); NestOut($6+2); }
          | SWITCH '(' expr ')'  { $<Int>$ = PC(); Cout(JUMP, -1);
                                   NestIn(0);  BeginSW(); }
              stmnt              { GenBrk(JUMP); Bpatch($<Int>5, PC());
                                   EndSW(); NestOut(0); }
          | BREAK ';'            { GenBrk(JUMP); }
          | CONTI ';'            { GenConti(); }
          | error ';'            { yyerrok; }
          ;
lbl_stmnt : CASE NUM ':'         { CaseLbl($2); }
              stmnt
          | CASE ADDOP NUM ':'   { CaseLbl($2 == SUB ? -$3 : $3); }
              stmnt           
          | DEFAULT ':'          { DfltLbl(); }
              stmnt
          ;
if_part   : IF '(' expr ')'      { $<Int>$ = PC(); Cout(BEQ, -1); }
              stmnt              { $$ = $<Int>5; }
          ;
opt_expr  :                      { $$ = PC(); }
          | expr                 { Pout(REMOVE); $$ = PC(); }
          ;
tst_expr  :                      { Cout(PUSHI, 1); $$ = PC(); }
          | expr                 { $$ = PC(); }
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
                                   Cout(PUSHI, 1);}
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
