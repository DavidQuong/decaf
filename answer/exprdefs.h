
#ifndef _DECAF_DEFS
#define _DECAF_DEFS

#include "expr-asts.h"
#include "llvm/IR/Type.h"
#include <deque>
#include <string>
#include <vector>

using namespace llvm;
using namespace std;

extern int lineno;
extern int tokenpos;
extern char* yytext;

extern "C"
{
  int yyerror(const char* s);
  int yyparse(void);
  int yylex(void);  
  int yywrap(void);
}

#endif

