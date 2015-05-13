/* プログラム 9.2 : FuncLコンパイラ（FuncL.y，172, 174, 175, 178ページ） */

%{
#include "../VSM.h"
#include "SymTable.h"

#define TCHK(X) { if ((X)==VOID) yyerror("Type error"); }

char  *IDentry(char *Name, int len);            /* NameTable.cで定義 */
Dtype  VarRef(int OPcode, char *Vname);         /* 変数参照命令の生成 */
void   GenFuncEntry(STP func);                  /* 出入口の命令生成 */
Dtype  GenCall(char *Fname, int Nparam);        /* 関数呼出しの命令列 */

STP FuncP;                                      /* 翻訳中の関数 */

extern int StartP;                              /* 実行開始アドレス */
%}

%union{
  int   Int;
  char *Name;                                   /* 識別子へのポインタ */
  STP   SymP;                                   /* 記号表へのポインタ */
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
glbl_decl:                      /* 空規則 */
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
p_list   :                      /* 空規則 */
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
decl_list:                      /* 空規則 */
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

void GenFuncEntry(STP func)                     /* 出入口のコード */
{
  int n;

  SetI(PUSH, FP, 0);   Cout(INCFR, -1);  Pout(RET); /* 出口のコード */
  Cout(DECFR, PC()-2); SetI(POP, FP, 0);            /* 入口のコード */
  for (n = func->Nparam; n > 0; n--)            /* 引数の値をコピー */
    SetI(POP, FP, n);                           /* するためのコード */
}

Dtype VarRef(int OPcode, char *Vname)           /* 変数参照命令の生成 */
{
  STP p;

  if ((p=SymRef(Vname)) != NULL)                /* 宣言済みの変数 ? */
    if (p->class != VAR && p->class != PARAM)   /* 変数への参照 ? */
      yyerror("Function name is used as a variable");
  SetI(OPcode, p->deflvl? FP : 0, p->loc);      /* PUSH/PUSHIの生成 */
  return p->type;
}

Dtype GenCall(char *Fname, int Nparam)          /* 関数呼出しのコード */
{
  STP p;

  if ((p=SymRef(Fname)) == NULL)                /* 未定義の識別子 ? */
    return VOID;
  if (p->class == FUNC || p->class == F_PROT) { /* 関数名 ? */
    if (p->Nparam != Nparam)                    /* 実引数と仮引数の */
      yyerror("Number of arguments mismatched");    /* 個数が同じ ? */
    Cout(CALL, p->loc);
    if (p->class == F_PROT)                     /* プロトタイプ関数名 */
      p->loc = PC() -1; }                       /* 後埋めチェイン更新 */
  else
    yyerror("Non-function call");
  return p->type;
}
