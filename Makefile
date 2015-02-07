CC = g++ -O2 -Wno-deprecated 

tag = -i

ifdef linux
tag = -n
endif

test: Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o y.tab.o lex.yy.o test.o
	$(CC) -o bin/test.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o y.tab.o lex.yy.o test.o -lfl
	rm -f *.o
	./bin/test.out
	
main: Record.o Comparison.o ComparisonEngine.o Schema.o File.o y.tab.o lex.yy.o main.o
	$(CC) -o bin/main Record.o Comparison.o ComparisonEngine.o Schema.o File.o y.tab.o lex.yy.o main.o -lfl
	rm -f *.o
	
test.o: source/test.cc
	$(CC) -g -c source/test.cc

main.o: source/main.cc
	$(CC) -g -c source/main.cc
	
Comparison.o: source/Comparison.cc
	$(CC) -g -c source/Comparison.cc
	
ComparisonEngine.o: source/ComparisonEngine.cc
	$(CC) -g -c source/ComparisonEngine.cc
	
DBFile.o: source/DBFile.cc
	$(CC) -g -c source/DBFile.cc

File.o: source/File.cc
	$(CC) -g -c source/File.cc

Record.o: source/Record.cc
	$(CC) -g -c source/Record.cc

Schema.o: source/Schema.cc
	$(CC) -g -c source/Schema.cc
	
y.tab.o: source/Parser.y
	yacc -d source/Parser.y
	sed $(tag) y.tab.c -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/" 
	g++ -c source/y.tab.c

lex.yy.o: source/Lexer.l
	lex  source/Lexer.l
	gcc  -c source/lex.yy.c

clean: 
	mv *.o bin/
	rm -f *.o
	rm -f *.out
	rm -f y.tab.c
	rm -f lex.yy.c
	rm -f y.tab.h
