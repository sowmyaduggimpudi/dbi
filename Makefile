CC = g++ -O2 -Wno-deprecated 

tag = -i

ifdef linux
tag = -n
endif


# Points to the root of Google Test, relative to where this file is.
# Remember to tweak this if you move this file.
GTEST_DIR = gtest
FUSED_GTEST_DIR = gtest/fused-src
FUSED_GTEST_H = $(FUSED_GTEST_DIR)/gtest/gtest.h
# Where to find user code.
USER_DIR = source

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra -pthread

# All tests produced by this Makefile.  Remember to add new tests you
# created to the list.
TESTS = unittest

# All Google Test headers.  Usually you shouldn't change this
# definition.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

# House-keeping build targets.

all : $(TESTS)

check : all
	$(USER_DIR)

clean :
	rm -f $(TESTS) gtest.a gtest_main.a *.o

# Builds gtest.a and gtest_main.a.

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^

# Builds a sample test.  A test should link with either gtest.a or
# gtest_main.a, depending on whether it defines its own main()
# function.


unittest: Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o y.tab.o lex.yy.o unittest.o gtest-all.o gtest_main.o 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o$@
	mv *.o bin/
	mv unittest bin/
	./bin/unittest

test: Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o y.tab.o lex.yy.o test.o
	$(CC) -o bin/test.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o y.tab.o lex.yy.o test.o -lfl
	rm -f *.o

test2: BigQ.o Pipe.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o y.tab.o lex.yy.o test2.o
	$(CC) -o bin/test2 BigQ.o Pipe.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o y.tab.o lex.yy.o test2.o -lfl -lpthread
	rm -f *.o

main: Record.o Comparison.o ComparisonEngine.o Schema.o File.o y.tab.o lex.yy.o main.o
	$(CC) -o bin/main Record.o Comparison.o ComparisonEngine.o Schema.o File.o y.tab.o lex.yy.o main.o -lfl
	rm -f *.o
	
unittest.o: source/unittest.cc \
		source/DBFile.h $(GTEST_HEADERS) \
		$(FUSED_GTEST_H)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
	source/unittest.cc

Pipe.o: source/Pipe.cc
	$(CC) -g -c source/Pipe.cc

BigQ.o: source/BigQ.cc
	$(CC) -g -c source/BigQ.cc

test2.o: source/test2.cc
	$(CC) -g -c source/test2.cc

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
