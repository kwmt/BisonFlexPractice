CC = gcc
OBJFILES = lex.yy.o y.tab.o VSME.o SymTable.o NameTable.o TypeDef.o \
           CodeGen.o ExprTree.o MainFunc.o

TypeL : ${OBJFILES}
	${CC} ${OBJFILES} -ly -ll -o TypeL

MainFunc.o  : ../MainFunc.c ../VSME.h

SymTable.o : SymTable.c SymTable.h TypeDef.h ../VSME.h

TypeDef.o : TypeDef.c SymTable.h TypeDef.h ../VSME.h

NameTable.o : ../NameTable.c

CodeGen.o  : CodeGen.c SymTable.h ExprTree.h ../VSME.h

ExprTree.o: ExprTree.c SymTable.h ExprTree.h ../VSME.h

VSME.o : ../VSME.c ../VSME.h

lex.yy.o: lex.yy.c y.tab.h SymTable.h TypeDef.h ExprTree.h

lex.yy.c: TypeL.l
	flex -l TypeL.l

y.tab.o: y.tab.c ../VSME.h SymTable.h TypeDef.h ExprTree.h

y.tab.c y.tab.h : TypeL.y
	bison -dv -y TypeL.y
