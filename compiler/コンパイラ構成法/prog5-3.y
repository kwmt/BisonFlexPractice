/* プログラム5.3 : 整数と実数の混合演算を行うYaccプログラム（81ページ） */

%{
#define ARITH(E, X, OP, Y) \
  if (X.Type == INT && Y.Type == INT) { \
    E.Type = INT; \
    E.V_fld.I = X.V_fld.I OP Y.V_fld.I; }\
  else {\
    E.Type = DBL; \
    E.V_fld.D = (X.Type==INT ? (double)X.V_fld.I : X.V_fld.D) \
                OP (Y.Type==INT ? (double)Y.V_fld.I : Y.V_fld.D); }
%}

%union{
  int    ival;
  double rval;
  struct {
    enum {INT, DBL} Type;
    union {
      int    I;
      double D;
    } V_fld;
  } Val;
}

%token <ival> INTC
%token <rval> REALC
%type  <Val>  expr

%left  '+' '-'

%%
calc   : expr '\n'            { if ($1.Type == INT)
                                  printf("%d\n", $1.V_fld.I);
                                else
                                  printf("%f\n", $1.V_fld.D); }
       ;
expr   : expr '+' expr        { ARITH($$, $1, +, $3); }
       | expr '-' expr        { ARITH($$, $1, -, $3); }
       | INTC                 { $$.Type = INT; $$.V_fld.I = $1; }
       | REALC                { $$.Type = DBL; $$.V_fld.D = $1; }
       ;
%%

main()
{
  yyparse();
}
