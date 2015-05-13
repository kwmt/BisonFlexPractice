%{
#include <stdio.h>
#include "mem.h"
%}

/* yylvalの型 */
%union{
  int Int; /* 数値を返したい時 */
  char *Name; /*  */
}

%token <Int>NUM
%token <Name>ID
%token IF
%token NL
%right '='
%left  '+' '-'
%left  '*' '/'
%left  '>'
%right '^'
%right UMINUS

%type <Name>line


%%
line : /* 空規則 */
	 {
	    printf("empty\n");
	 }
     | stmnt
	 {
      	printf("stmnt -> line\n");
        $$ = $<Name>1;
        printf("%s\n", $$);

     }
     ;
stmnt : expr ';'
	  {
      	printf("expr -> stmnt\n");
	  }
      | LHS expr ';'
	  {
      	printf("LHS expr -> stmnt\n");
	  }
| error ';' {yyerrok;}
      ;

LHS   : ID
	  {
      	printf("ID -> LHS\n");
	  }
      ;
expr  : expr '=' expr
	  {
      	printf("expr = expr -> expr\n");
	  }
	  | expr '>' expr
	  {
      	printf("expr > expr -> expr\n");
        /* リスト作成  */
	  }
	  | ID
      {
      	printf("ID -> expr\n");
      }
	  | NUM
      {
      	printf("NUM -> expr\n");
      }
      ;

%%
/* コンパイルするのに必要  */
yyerror(char *msg)
{
  /* printf("%s\n", msg); */
}

extern tree_node *tree_root;
extern tree_node *create_new_node(int num);
extern void insert_tree(int num, tree_node *node);
extern void print_tree(tree_node *node);



int main(int argc, char **argv)
{
#if YYDEBUG
  extern int yydebug;
  yydebug = 1;
#endif
  extern FILE *yyin;
  FILE *fp;

  /* ファイルを開く */
  fp = fopen(argv[1], "r");
  yyin = fp;


  insert_tree(1, tree_root);

  tree_node *child_1;
  child_1 = create_new_node(2);

  insert_tree(3, tree_root);

  add_node_left(tree_root, child_1);
  
  print_tree(tree_root);
  
  /* 解析メイン */
  yyparse();

  /* ファイルを閉じる */
  fclose(fp);
  return 0;
}

