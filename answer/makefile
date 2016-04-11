
lexlib=l
yacclib=y
llvm_as=llvm-as # TODO Change to 3.3
llvm_config=llvm-config # TODO - Change to 3.3
bindir=.
cp=/bin/cp -f
rm=/bin/rm -f
mv=/bin/mv -f
targets=
cpptargets=
# deps=decaf-ast.cc llvm-codegen.cc symboltable.cc
deps=expr-asts.cpp llvm-pass.cpp llvm-util.cpp symbol-table.cpp value-constants.cpp
pass=decaf-pass.cpp
# llvmcpp=decaf-llvm
llvmcpp=decaf-codegen

# all: decaf-codegen
all: $(llvmcpp)

decaf-codegen: $(llvmcpp) $(cpp)
	$(cp) decaf-llvm decaf-codegen

$(targets): %: %.y
	@echo "compiling yacc file:" $<
	@echo "output file:" $@
	bison -o$@.tab.c -d $<
	flex -o$@.lex.c $@.lex
	gcc -o $(bindir)/$@ $@.tab.c $@.lex.c -l$(yacclib) -l$(lexlib)
	$(rm) $@.tab.c $@.tab.h $@.lex.c

$(cpptargets): %: %.y
	@echo "compiling cpp yacc file:" $<
	@echo "output file:" $@
	bison -b $@ -d $<
	$(mv) $@.tab.c $@.tab.cc
	flex -o$@.lex.cc $@.lex
	g++ -o $(bindir)/$@ $@.tab.cc $@.lex.cc -l$(yacclib) -l$(lexlib)
	$(rm) $@.tab.h $@.tab.cc $@.lex.cc

$(llvmcpp): %: %.y
	@echo "compiling cpp yacc file:" $<
	@echo "output file:" $@
	bison -b $@ -d $<
	$(mv) $@.tab.c $@.tab.cc
	flex -o$@.lex.cc $@.lex
	gcc -g -c decaf-stdlib.c
	g++ -o $(bindir)/$@ $@.tab.cc $@.lex.cc $(deps) decaf-stdlib.o -Wl,--no-as-needed `$(llvm_config) --cppflags --ldflags --libs core jit native` -l$(yacclib) -l$(lexlib)
	$(rm) $@.tab.h $@.tab.cc $@.lex.cc

$(llvmtargets): %: %.ll
	@echo "using llvm to compile file:" $<
	$(llvm_as) $<
	`$(llvm_config) --bindir`/llc -disable-cfi $@.bc
	gcc $@.s ../decaf-stdlib.c -o $(bindir)/$@

clean:
	$(rm) decaf-codegen $(targets) $(cpptargets) $(llvmtargets) $(llvmcpp)
	$(rm) *.tab.h *.tab.c *.lex.c
	$(rm) *.bc *.s *.o
	$(rm) -r *.dSYM

