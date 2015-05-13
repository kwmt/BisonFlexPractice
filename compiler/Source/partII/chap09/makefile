OBJFILES = lex.yy.o y.tab.o VSM.o SymTable.o NameTable.o MainFunc.o

FuncC : ${OBJFILES}
	gcc ${OBJFILES} -ly -ll -o FuncC

MainFunc.o : ../MainFunc.c ../VSM.h

SymTable.o : SymTable.c SymTable.h

NameTable.o : ../NameTable.c

VSM.o : ../VSM.c ../VSM.h

lex.yy.o: lex.yy.c y.tab.h SymTable.h

lex.yy.c: FuncL.l
	flex -l FuncL.l

y.tab.o: y.tab.c ../VSM.h SymTable.h

y.tab.c y.tab.h : FuncL.y
	bison -y -dv FuncL.y
