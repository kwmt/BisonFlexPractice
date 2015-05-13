/* プログラム 13.4 : TypeLの字句の定義（306, 307ページ） */

%{
#include "../VSME.h"
#include "SymTable.h"
#include "ExprTree.h"
#include "TypeDef.h"
#include "y.tab.h"

char *IDentry(char *Name, int Len), *StrScan(void);
%}

ws         [ \t\n]
sym        [+\-*/%=&().,;{}[\]]
digit      [0-9]
letter     [a-zA-Z_]
id         {letter}({letter}|{digit})*
string     \"([^\"\\\n]|\\.)*[\"\n]
other      .

%%

{ws}+           ;
{sym}           { return yytext[0]; }
"<"             { yylval.Int = BLT; return RELOP; }
"<="            { yylval.Int = BLE; return RELOP; }
"=="            { yylval.Int = BEQ; return RELOP; }
"!="            { yylval.Int = BNE; return RELOP; }
">="            { yylval.Int = BGE; return RELOP; }
">"             { yylval.Int = BGT; return RELOP; }
"->"            { return ARROW; }
"char"          { yylval.SymP = SymBTDP[CHAR]; return TYPE; }
"delete"        { return DEL; }
"else"          { return ELSE; }
"if"            { return IF; }
"int"           { yylval.SymP = SymBTDP[INT];  return TYPE; }
"new"           { return NEW; }
"NULL"          { return NIL; }
"return"        { return RETURN; }
"sizeof"        { return SIZEOF; }
"struct"        { return STRCTR; }
"type"          { return TDEF; }
"void"          { yylval.SymP = SymBTDP[VOID]; return TYPE; } 
"while"         { return WHILE; }
"write"         { return WRITE; }
"/*"            { if (SkipComment() == EOF) return EOF; }
{digit}+        { sscanf(yytext, "%d", &yylval.Int);
                  return NUM; }
{id}            { yylval.Name = IDentry(yytext, yyleng); return ID; }
{string}        { yylval.Name = StrScan(); return STRING; }
{other}         { fprintf(stderr, "Illegal char '%c' ignored\n",
                          yytext[0]); }
%%

char *StrScan(void)
{
  char *p, *q;

  if (yytext[yyleng-1] != '\"')
    yyerror("Unterminated string literal");
  for (p=yytext+1, q=yytext; p < yytext+(yyleng-1); *(q++) = *(p++))
    if (*p == '\\')
      if (*(++p) == 'n') *p = '\n';
      else if (*p == 't') *p = '\t';
  *q = '\0';
  yyleng = q - yytext;
  return yytext;
}

int SkipComment(void)
{
  int c;

  while(1) {
    while ((c=input()) != '*' && c != EOF) ;
    if (c == '*') {
      while ((c=input()) == '*') ;
      if (c == '/') return 1; }
    if (c == EOF) {
      fprintf(stderr, "Unterminated comment\n");
      return EOF; }
  }
}
