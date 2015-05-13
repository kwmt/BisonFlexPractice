CC = gcc
OBJFILES = lex.yy.o y.tab.o VSME.o SymTable.o NameTable.o CodeGen.o \
            ExprTree.o MainFunc.o

MiniC : ${OBJFILES}
	gcc ${OBJFILES} -ly -ll -o MiniC

MainFunc.o  : ../MainFunc.c ../VSME.h

SymTable.o : SymTable.c SymTable.h ../VSME.h

NameTable.o : ../NameTable.c

ExprTree.o:  ExprTree.c ExprTree.h SymTable.h ../VSME.h

CodeGen.o  : CodeGen.c CodeGen.h ExprTree.h SymTable.h ../VSME.h

VSME.o : ../VSME.c ../VSME.h

lex.yy.o: lex.yy.c y.tab.h SymTable.h

lex.yy.c: MiniL.l
	flex -l MiniL.l

y.tab.o: y.tab.c ../VSME.h SymTable.h ExprTree.h CodeGen.h

y.tab.c y.tab.h : MiniL.y
	bison -dv -y MiniL.y

