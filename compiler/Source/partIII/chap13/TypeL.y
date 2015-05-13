/* プログラム 13.3 と 14.2 : TypeLコンパイラ（TypeL.y，301, 303, 
/*                                                     321, 322ページ ） */

%{
#include "../VSME.h"
#include "SymTable.h"
#include "ExprTree.h"
#include "TypeDef.h"

extern void GenFuncEntry(STP func),       Epilogue(void),
            GenRetur(STP func, NP tree),  GenNewDel(int OP, NP tree),
            GenExpr(int Op, NP tree);
extern int  yyleng, CtrlExpr(NP lt);

STP FuncP;
%}

%union{
  NP    NodeP;
  int   Int;
  char *Name;
  STP   SymP;
  TDP   TypeP;
}

%token        ARROW DEL ELSE IF NEW NIL RETURN SIZEOF STRCTR TDEF
              WHILE WRITE
%token <Int>  NUM   RELOP
%token <Name> ID    STRING
%token <SymP> TYPE

%type <Int>   p_list    if_part
%type <SymP>  decl      type_name f_head    p_decl   declatr
%type <TypeP> type_expr type_prim size_list array_sz
%type <NodeP> expr      primary   arg_list

%right '='
%left  RELOP
%left  '+' '-'
%left  '*' '/' '%'
%right '&' UOP
%left  '.' '['   ARROW
%%

program  :                           { SetUpSymTab(); }
           decl_list                 { Epilogue(); }
         ;
decl_list:                           /* 空規則 */
         | decl_list type_def
         | decl_list decl ';'
         | decl_list func_def
         ;
type_def : TDEF ID '=' type_expr ';' { TypeDef($2, $4); }
         ;
type_expr: type_prim
         | type_prim size_list       { ArrayDesc($2, $1); $$ = $2; }
         ;
type_prim: type_name                 { $$ = $1->type; }
         | STRCTR                    { BeginStruct(); }
             '{' decl_list '}'       { $$ = EndStruct(); }
         | type_prim '*'             { $$ = RefType($1); }
         | '(' type_expr ')'         { $$ = $2; }
         ;
type_name: TYPE
         | ID                        { $$ = TypeName($1); }
         ;
size_list: array_sz
         | array_sz size_list        { $1->compnt = $2; }
         ;
array_sz : '[' ']'                   { MAKE_ARRAY($$, -1); }
         | '[' NUM ']'               { MAKE_ARRAY($$, $2); }
         ;
decl     : type_name declatr         { MemAlloc ($2, $1->type, 0); }
         | type_name f_head          { Prototype($2, $1->type); }
         | decl ',' declatr          { MemAlloc ($3, $1->type, 0); }
         | decl ',' f_head           { Prototype($3, $1->type); }
         ;
declatr  : ID                        { $$ = NewID($1, NULL); }
         | ID size_list              { $$ = NewID($1, $2); }
         ;
f_head   : ID                        { $<SymP>$ = MakeFuncEntry($1); }
             '(' p_list ')'          { $$ = $<SymP>2; $$->Nparam = $4; }
         ;
p_list   :                           { $$ = 0; }
         | p_decl                    { $$ = 1; }
         | p_list ',' p_decl         { $$ = $1 + 1; }
         ;
p_decl   : type_expr declatr         { MemAlloc($2, $1, PARAM); }
         ;
func_def : type_name f_head '{'      { FuncDef($2, $1); FuncP = $2;
                                       GenFuncEntry($2); }
             decl_list st_list '}'   { EndFdecl($2); }
         ;
block    : '{'                       { OpenBlock(); }
             decl_list st_list '}'   { CloseBlock(); }
         ;
st_list  : stmnt
         | st_list stmnt
         ;
stmnt    : block
         | expr ';'                  { GenExpr(ASSGN, $1); }
         | if_part                   { Bpatch($1, PC()); }
         | if_part ELSE              { $<Int>$ = PC(); Cout(JUMP, -1);
                                        Bpatch($1, PC()); }
             stmnt                   { Bpatch($<Int>3, PC()); }
         | WHILE                     { $<Int>$ = PC(); }
             '(' expr ')'            { $<Int>$ = CtrlExpr($4); }
              stmnt                  { Cout(JUMP, $<Int>2);
                                       Bpatch($<Int>6, PC()); }
         | NEW '(' primary ')' ';'   { GenNewDel(MALLOC, $3); }
         | DEL '(' primary ')' ';'   { GenNewDel(MFREE,  $3); }
         | WRITE '(' STRING          { Cout(PUSHI, AllocCons($3,
                                                    CBSZ, yyleng+1)); }
             ',' expr ')' ';'        { GenExpr(OUTPUT, $6); }
         | RETURN ';'                { GenReturn(FuncP, NULL); }
         | RETURN expr ';'           { GenReturn(FuncP, $2); }
         ;
if_part  : IF '(' expr ')'           { $<Int>$ = CtrlExpr($3); }
             stmnt                   { $$ = $<Int>5; }
         ;
expr     : primary '=' expr          { $$ = MakeN(ASSGN, $1, $3); }
         | expr '+' expr             { $$ = MakeN(ADD,   $1, $3); }
         | expr '-' expr             { $$ = MakeN(SUB,   $1, $3); }
         | expr '*' expr             { $$ = MakeN(MUL,   $1, $3); }
         | expr '/' expr             { $$ = MakeN(DIV,   $1, $3); }
         | expr '%' expr             { $$ = MakeN(MOD,   $1, $3); }
         | expr RELOP expr           { $$ = MakeN(COMP,  $1, $3); 
                                       $$->etc = $2; }
         | primary
         ;
primary  : '-' primary     %prec UOP { $$ = MakeN(CSIGN,  $2, NULL); }
         | '*' primary     %prec UOP { $$ = MakeN(REFOBJ, $2, NULL); }
         | '&' primary               { $$ = MakeN(ADDROP, $2, NULL); }
         | SIZEOF '(' type_expr ')'  { CONS_NODE($$, INTP, $3->msize); }
         | ID '(' arg_list ')'       { $$ = MakeN(CALL,MakeL($1),$3); }
         | primary '[' expr ']'      { $$ = MakeN(AELM, $1, $3); }
         | primary '.' ID            { $$ = RecSelect(MEMOP, $1, $3); }
         | primary ARROW ID          { $$ = RecSelect(INMEM, $1, $3); }
         | '(' expr ')'              { $$ = $2; }
         | ID                        { VAR_NODE($$, $1); }
         | NUM                       { CONS_NODE($$, INTP, $1) }
         | NIL                       { CONS_NODE($$, UNIVP, 0) }
         ;
arg_list :                           { $$ = NULL; }
         | expr
         | arg_list ',' expr         { $$ = MakeN(ARGL, $1, $3); }
         ;
%%
