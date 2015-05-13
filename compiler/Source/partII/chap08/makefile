CC = gcc
OBJFILES = lex.yy.o y.tab.o VSM.o SymTable.o NameTable.o JumpChain.o \
            LabelDef.o MainFunc.o

CntrlC : ${OBJFILES}
	gcc ${OBJFILES} -ly -ll -o CntrlC

MainFunc.o : ../MainFunc.c ../VSM.h

JumpChain.o : JumpChain.c ../VSM.h

LabelDef.o : LabelDef.c ../VSM.h
    
SymTable.o : SymTable.c

NameTable.o : ../NameTable.c

VSM.o : ../VSM.c ../VSM.h

lex.yy.o: lex.yy.c y.tab.h

lex.yy.c: CntrlL.l
	flex -l CntrlL.l

y.tab.o: y.tab.c ../VSM.h

y.tab.c y.tab.h : CntrlL.y
	bison -dv -y CntrlL.y


