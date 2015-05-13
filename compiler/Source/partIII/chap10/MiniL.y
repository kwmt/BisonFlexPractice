/* プログラム 10.3と11.3 : MiniLコンパイラ（MiniL.y，206〜207,  */
/*                                                   234〜235ページ） */

%{
#include "../VSME.h"
#include "SymTable.h"
#include "ExprTree.h"
#include "CodeGen.h"

STP FuncP;
extern int yyleng;
%}

%union{
  NP     NodeP;
  int    Int;
  double Dbl;
  char  *Name;
  STP    SymP;
}

%token        DO ELSE FOR IF LAND LOR READ RETURN STATIC WRITE WHILE
%token <Int>  ADDOP MULOP PPMM RELOP TYPE NUM CNUM
%token <Dbl>  RNUM
%token <Name> ID STRING

%type <Int>   type_spec decl dim_list if_part
%type <SymP>  f_head declatr
%type <NodeP> expr opt_expr cast_expr primary sub_list arg_list

%right '='
%left  LOR
%left  LAND
%left  RELOP
%left  ADDOP
%left  MULOP
%right UM PPMM '!'
%left  POSOP

%%

program  : glbl_def              { Epilogue(); }
         ;
glbl_def :                       /* 空規則 */
         | glbl_def decl ';'
         | glbl_def func_def 
         | glbl_def error ';'    { yyerrok; }
         ;
decl     : type_spec declatr     { MemAlloc ($2, $1, 0); }
         | type_spec f_head      { Prototype($2, $1); }
         | decl ',' declatr      { MemAlloc ($3, $1, 0); }
         | decl ',' f_head       { Prototype($3, $1); }
         ;
type_spec: TYPE
         | STATIC                { $$ = SALLOC | INT; }
         | STATIC TYPE           { $$ = SALLOC | $2; }
         ;
declatr  : ID dim_list           { $$ = VarDecl($1, $2); }
         ;
dim_list :                       { $$ = 0; }
         | dim_list '[' ']'      { AStable[$1] = -1; $$ = $1 + 1; }
         | dim_list '[' NUM ']'  { AStable[$1] = $3; $$ = $1 + 1; }
         ;
f_head   : ID                    { $<SymP>$ = MakeFuncEntry($1); }
           '(' p_list ')'        { $$ = $<SymP>2; }
         ;
p_list   :                       /* 空規則 */
         | p_decl
         | p_list ',' p_decl
         ;
p_decl   : TYPE declatr          { MemAlloc($2, $1, PARAM); }
         | TYPE '&' declatr      { MemAlloc($3, $1, PARAM | BYREF); }
         ;
func_def : type_spec f_head '{'  { FuncDef($2, $1); FuncP = $2;
                                   GenFuncEntry($2); }
             decl_list
             st_list '}'         { GenReturn($2, NULL);
                                   EndFdecl($2); }
         ;
block    : '{'                   { OpenBlock(); }
             decl_list st_list
           '}'                   { CloseBlock(); }
         ;
decl_list:
         | decl_list decl ';'
         ;
st_list  : stmnt
         | st_list stmnt
         ;
stmnt    : block
         | ';'
         | expr ';'              { ExprStmnt($1); }
         | if_part               { Bpatch($1, PC()); }
         | if_part ELSE          { $<Int>$ = PC(); Cout(JUMP, -1);
                                   Bpatch($1, PC()); }
           stmnt                 { Bpatch($<Int>3, PC()); }
         | WHILE                 { $<Int>$ = PC(); }
             '(' expr ')'        { $<Int>$ = CtrlExpr($4, FJ); }
             stmnt               { Cout(JUMP, $<Int>2);
                                   Bpatch($<Int>6, PC()); }
         | FOR '(' opt_expr ';'  { ExprStmnt($3); $<Int>$ = PC(); }
             opt_expr ';'        { $<Int>$ = CtrlExpr($6, FJ); }
             opt_expr ')'
             stmnt               { ExprStmnt($9); Cout(JUMP, $<Int>5);
                                   Bpatch($<Int>8, PC()); }
         | DO                    { $<Int>$ = PC(); }
             stmnt WHILE
             '(' expr ')' ';'    { Bpatch(CtrlExpr($6, TJ), $<Int>2); }
         | RETURN ';'            { GenReturn(FuncP, NULL); }
         | RETURN expr ';'       { GenReturn(FuncP, $2); }
         | error ';'             { yyerrok; }
         ;
if_part  : IF '(' expr ')'       { $<Int>$ = CtrlExpr($3, FJ); }
             stmnt               { $$ = $<Int>5; }
         ;
opt_expr :                       { $$ = NULL; }
         | expr
         ;
expr     : primary '=' expr      { $$ = MakeN(ASSGN, $1, $3); }
         | expr LOR expr         { $$ = MakeN(OR, $1, $3); }
         | expr LAND expr        { $$ = MakeN(AND, $1, $3); }
         | expr RELOP expr       { $$ = MakeN(COMP, $1, $3);
                                   $$->etc = $2; }
         | expr ADDOP expr       { $$ = MakeN($2, $1, $3); }
         | expr MULOP expr       { $$ = MakeN($2, $1, $3); }
         | cast_expr
         ;
cast_expr: primary
         | '(' TYPE ')' cast_expr { $$ = TypeConv($4, $2); }
         ;
primary  : ADDOP primary %prec UM { $$ = ($1==SUB)?
                                         MakeN(CSIGN, $2, NULL): $2; }
         | PPMM primary          { $$ = MakeN($1, $2, NULL);
                                   $$->etc = PRE; }
         | primary PPMM %prec POSOP
                                 { $$ = MakeN($2, $1, NULL);
                                  $$->etc = POST; }
         | '!' primary           { $$ = MakeN(NOT, $2, NULL); }
         | READ '(' primary ','
             primary ')'         { $$ = MakeN(INPUT, $3, $5); }
         | WRITE '(' primary ')' { $$ = MakeN(OUTSTR, $3, NULL); }
         | WRITE '(' primary ','
             expr ')'            { $$ = MakeN(OUTPUT, $3, $5); }
         | ID '(' arg_list ')'   { $$ = MakeN(CALL, MakeL($1), $3); }
         | '(' expr ')'          { $$ = $2; }
         | ID sub_list           { $$ = MakeN(AELM, MakeL($1), $2); }
         | ID                    { VAR_NODE($$, $1) }
         | STRING                { MakeC($$, C_ARY,
                                       AllocCons($1, CBSZ, yyleng+1)); }
         | NUM                   { MakeC($$, INT, $1); }
         | CNUM                  { MakeC($$, INT, $1); }
         | RNUM                  { MakeC($$, DBL, Wcons($1)); }
         ;
sub_list : '[' expr ']'          { $$ = $2; }
         | sub_list '[' expr ']' { $$ = MakeN(SUBL, $1, $3); }
         ;
arg_list :                       { $$ = NULL; }
         | expr
         | arg_list ',' expr     { $$ = MakeN(ARGL, $1, $3); }
         ;
%%
