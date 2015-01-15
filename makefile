compile:  lex.yy.c y.tab.c main.c
	gcc main.c lex.yy.c y.tab.c -o compile 
lex.yy.c: pascal.l
	lex pascal.l
y.tab.c: pascal.y y.tab.h
	yacc -tvd pascal.y