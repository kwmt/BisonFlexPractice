CC = gcc
OBJFILES = lex.yy.o y.tab.o VSM.o SymTable.o NameTable.o MainFunc.o

ExprC : ${OBJFILES}
	gcc ${OBJFILES} -ly -ll -o ExprC

MainFunc.o : ../MainFunc.c ../VSM.h

SymTable.o : SymTable.c

NameTable.o : ../NameTable.c

VSM.o : ../VSM.c ../VSM.h

lex.yy.o : lex.yy.c y.tab.h ../VSM.h

lex.yy.c : ExprL.l
	flex -l ExprL.l

y.tab.o : y.tab.c ../VSM.h

y.tab.c y.tab.h : ExprL.y
	bison -dv -y ExprL.y


