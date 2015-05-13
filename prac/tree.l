%{
#include <stdio.h>
#include <stdlib.h>
#include "y.tab.h"

/* エラー表示用の行番号のカウンタ */
int LineNumber;

int yywrap( void )
{
return 1;
}

%}

/* 正規表現定義 */
ws     	[ \t]
symbol  [=;>]
digit  	[0-9]
letter  [a-zA-Z_]
id      {letter}({letter}|{digit})*
other	.	 

%%

{ws}+
{symbol} { return yytext[0]; }
"if"    {
			printf("tree.l:if\n");
			return IF;
	    }

"\n"	{
			LineNumber++;
			printf("LineNumer=%d\n", LineNumber);
			return NL;
		}
{digit}+ {
			int num;
			num = atoi(yytext);
			printf("tree.l:digit:%d\n", num);
			return NUM;
		}
{id}    {
	 		yylval.Name = yytext;
			printf("tree.l:id:%s\n", yytext);
			return ID;
		}
{other}	{
			fprintf(stderr, "Illegal char '%c' ignored\n", yytext[0]);
		}

