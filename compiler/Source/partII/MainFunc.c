/* プログラム 7.5 : 関数main() （MainFunc.c，134, 135ページ） */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "VSM.h"

int        StartP=0, SymPrintSW=0;                  /* オプション用の */
static int ExecSW=1, ObjOutSW=0, TraceSW=0, StatSW=0;   /* フラグ変数 */

static int  ErrorC=0;                               /* エラーカウンタ */
static char SourceFile[20];

extern FILE *yyin;                                  /* 入力ファイル */
                                                    /* ポインタ */
main(int argc, char *argv[])
{
  static void SetUpOpt(int, char *[]);

  SetUpOpt(argc, argv);                             /* オプションの処理 */
  if (SourceFile[0] != NULL)                        /* 入力ファイルを */
    if ((yyin=fopen(SourceFile, "r")) == NULL) {         /* オープン */
      fprintf(stderr, "Source file cannot be opened.");
      exit(-1); }                                   /* コンパイル中止 */
  yyparse();                                        /* コンパイル */
  if (ObjOutSW) DumpIseg(0, PC()-1);                /* 目的コード表示 */
  if (ExecSW || TraceSW)
    if (ErrorC == 0) {                              /* 翻訳エラーが */
      printf("Enter execution phase\n");            /* あった ? */
      if (SourceFile[0] == NULL)                    /* キーボード ? */
        rewind(stdin);                              /* 入力の再設定 */
      if (StartVSM(StartP, TraceSW) != 0)           /* プログラム実行 */
        printf("Execution aborted\n");
      if (StatSW) ExecReport(); }                   /* 実行状況の報告 */
    else
      printf("Execution suppressed\n");
}

static void SetUpOpt(int argc, char *argv[])
{
  char *s;

  if (--argc>0 && (*++argv)[0]=='-') {             /* オプション指定 ? */
    for (s = *argv+1; *s != '\0'; s++)             /* オプションの走査 */
      switch(tolower(*s)) {                        /* 小文字でチェック */
        case 'c': StatSW = 1;     break;           /* 実行データの表示 */
        case 'd': DebugSW = 1;    break;           /* デバッグモード */
        case 'n': ExecSW = 0;     break;           /* コンパイルだけ */
        case 'o': ObjOutSW = 1;   break;           /* 目的コードの表示 */
        case 's': SymPrintSW = 1; break;           /* 記号表の表示 */
        case 't': TraceSW = 1;    break; }         /* トレースモード */
    argc--; argv++; }
  if (argc > 0)                                    /* 入力ファイル名を */
    strcpy(SourceFile, *argv);                     /* コピー */
}

yyerror(char *msg)                          /* エラーメッセージ出力関数 */
{
  extern int yylineno;                      /* 現在の行番号*/

  printf("%s at line %d\n", msg, yylineno);
  ErrorC++;
}
