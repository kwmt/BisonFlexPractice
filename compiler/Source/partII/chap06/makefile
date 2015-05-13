CC = gcc
OBJFILES = lex.yy.o y.tab.o VSM.o         # マクロOBJFILESの定義

ExprC  : ${OBJFILES}                      # ExprCが依存するファイルの指定
	${CC} ${OBJFILES} -ly -ll -o ExprC

VSM.o    : ../VSM.c ../VSM.h              # VSMシミュレータの依存ファイル

lex.yy.o : lex.yy.c y.tab.h ../VSM.h      # lex.yy.oの依存ファイル

lex.yy.c : CalcL.l                        # lex.yy.cの依存ファイル
	flex -l CalcL.l

y.tab.o : y.tab.c ../VSM.h                # y.tab.oの依存ファイル

y.tab.c y.tab.h : CalcL.y                 # y.tab.cの依存ファイル
	bison -dv -y CalcL.y
