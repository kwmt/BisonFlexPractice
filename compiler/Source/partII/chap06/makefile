CC = gcc
OBJFILES = lex.yy.o y.tab.o VSM.o         # $B%^%/%m(BOBJFILES$B$NDj5A(B

ExprC  : ${OBJFILES}                      # ExprC$B$,0MB8$9$k%U%!%$%k$N;XDj(B
	${CC} ${OBJFILES} -ly -ll -o ExprC

VSM.o    : ../VSM.c ../VSM.h              # VSM$B%7%_%e%l!<%?$N0MB8%U%!%$%k(B

lex.yy.o : lex.yy.c y.tab.h ../VSM.h      # lex.yy.o$B$N0MB8%U%!%$%k(B

lex.yy.c : CalcL.l                        # lex.yy.c$B$N0MB8%U%!%$%k(B
	flex -l CalcL.l

y.tab.o : y.tab.c ../VSM.h                # y.tab.o$B$N0MB8%U%!%$%k(B

y.tab.c y.tab.h : CalcL.y                 # y.tab.c$B$N0MB8%U%!%$%k(B
	bison -dv -y CalcL.y
