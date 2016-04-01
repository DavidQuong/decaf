%{

#include "exprdefs.h"
#include "expr-asts.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm-util.h"
#include "symbol-table.h"
#include "value-constants.h"
#include <cstdio>
#include <cstring>
#include <deque>
#include <map>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

using namespace llvm;
using namespace std;

/* extern "C" int yyparse(); */
/* extern int yylex(); */ 
extern char* yytext;
extern int yyleng;
extern int yylineno;

/* void yyerror(char const* s); */
char* copyString(char* str, int length);
char escapeCharacter(char escapedChar);
void generateExterns(vector<ExternExprAst*>* externList);
void generateClass(deque<ExprAst*>* exprList);

%}

/* %code requires { */
/*     #include "exprdefs.h" */
/* } */

%union {
    class vector<ExternExprAst*>* externList;
    class ExternExprAst* externExpr;
    class vector<Type*>* typeList;
    class deque<ExprAst*>* exprList;
    class Type* type;
    class vector<pair<Type*,char*>*>* paramList;
    class pair<Type*,char*>* param;
    class ExprAst* expr;
    /* class Value* val; */
    int num;
    char* str;
}

%token T_AND T_ASSIGN T_BOOLTYPE T_BREAK T_CLASS T_COMMENT T_COMMA T_CONTINUE T_DIV T_DOT T_ELSE T_EQ T_EXTENDS T_EXTERN T_FALSE T_FOR T_GEQ T_GT T_IF T_INTTYPE T_LCB T_LEFTSHIFT T_LEQ T_LPAREN T_LSB T_LT T_MINUS T_MOD T_MULT T_NEQ T_NEW T_NOT T_NULL T_OR T_PLUS T_RCB T_RETURN T_RIGHTSHIFT T_RPAREN T_RSB T_SEMICOLON T_STRINGTYPE T_TRUE T_VOID T_WHILE 
%token <str> T_ID T_STRINGCONSTANT 
%token <num> T_INTCONSTANT T_CHARCONSTANT

%type <externList> externs
%type <externExpr> extern
%type <exprList> class declarations preceeding_declaration following_declaration method_declarations method_block variable_declarations variable_declaration variable_ints variable_bools statements method_arguments

%type <typeList> extern_parameters
%type <type> extern_parameter

%type <paramList> method_parameters
%type <param> method_parameter

%type <expr> method_declaration variable_int variable_bool statement block assignment method_call method_argument expression p1_expression p2_expression p3_expression p4_expression p5_expression root_expression expression_variable constant

%type <num> number
%type <str> identifier string extern_type method_type type boolean_constant p1_operator p2_operator p3_operator p4_operator p5_operator unary_operator

%%

program: push_symtbl externs push_symtbl class                                                                                          { generateExterns($2);
                                                                                                                                          generateClass($4);
                                                                                                                                          popSymbolTable();
                                                                                                                                          popSymbolTable(); } 
    ;

externs: externs extern                                                                                                                 { vector<ExternExprAst*>* externList = $1;
                                                                                                                                          externList->push_back($2);
                                                                                                                                          $$ = externList; }
    | extern                                                                                                                            { vector<ExternExprAst*>* externList = new vector<ExternExprAst*>;
                                                                                                                                          externList->push_back($1);
                                                                                                                                          $$ = externList; }
    | /* No extern declarations */                                                                                                      { $$ = new vector<ExternExprAst*>; } 
    ;

extern: T_EXTERN method_type identifier T_LPAREN extern_parameters T_RPAREN T_SEMICOLON                                                 { Type* type = getLLVMType($2);
                                                                                                                                          char* id = $3;
                                                                                                                                          vector<Type*>* paramTypes = $5;
                                                                                                                                          $$ = new ExternExprAst(type, id, paramTypes); }
    ;

extern_parameters: extern_parameters T_COMMA extern_parameter                                                                           { vector<Type*>* typeList = $1;
                                                                                                                                          typeList->push_back($3);
                                                                                                                                          $$ = typeList; }
    | extern_parameter                                                                                                                  { vector<Type*>* typeList = new vector<Type*>;
                                                                                                                                          typeList->push_back($1);
                                                                                                                                          $$ = typeList; }
    | /* No extern parameters */                                                                                                        { $$ = new vector<Type*>; }
    ;

extern_parameter: extern_type                                                                                                           { $$ = getLLVMType($1); }
    ;

class: T_CLASS identifier T_LCB declarations T_RCB                                                                                      { $$ = $4; }
    ;


declarations: preceeding_declaration                                                                                                    { $$ = $1; }
    ;

preceeding_declaration: T_INTTYPE field_ints T_SEMICOLON following_declaration                                                          { /* Reserved */ }
    | T_BOOLTYPE field_bools T_SEMICOLON following_declaration                                                                          { /* Reserved */ }
    | field_int_assign following_declaration                                                                                            { /* Reserved */ }
    | field_bool_assign following_declaration                                                                                           { /* Reserved */ }
    | T_INTTYPE identifier T_LPAREN method_parameters T_RPAREN method_block method_declarations                                         { Type* type = getLLVMType(VALUE_INTTYPE);
                                                                                                                                          char* id = $2;
                                                                                                                                          vector<pair<Type*,char*>*>* paramList = $4;
                                                                                                                                          deque<ExprAst*>* stmtList = $6;
                                                                                                                                          ExprAst* expr = new FunctionExprAst(type, id, paramList, stmtList);
                                                                                                                                          deque<ExprAst*>* exprList = $7;
                                                                                                                                          exprList->push_front(expr);
                                                                                                                                          $$ = exprList; }

    | T_BOOLTYPE identifier T_LPAREN method_parameters T_RPAREN method_block method_declarations                                        { Type* type = getLLVMType(VALUE_BOOLTYPE);
                                                                                                                                          char* id = $2;
                                                                                                                                          vector<pair<Type*,char*>*>* paramList = $4;
                                                                                                                                          deque<ExprAst*>* stmtList = $6;
                                                                                                                                          ExprAst* expr = new FunctionExprAst(type, id, paramList, stmtList);
                                                                                                                                          deque<ExprAst*>* exprList = $7;
                                                                                                                                          exprList->push_front(expr);
                                                                                                                                          $$ = exprList; }

    | method_declarations                                                                                                               { $$ = $1; }
    ;

following_declaration: T_INTTYPE field_ints T_SEMICOLON following_declaration                                                           { /* Reserved */ }
    | T_BOOLTYPE field_bools T_SEMICOLON following_declaration                                                                          { /* Reserved */ }
    | field_int_assign following_declaration                                                                                            { /* Reserved */ }
    | field_bool_assign following_declaration                                                                                           { /* Reserved */ }
    | T_INTTYPE identifier T_LPAREN method_parameters T_RPAREN method_block method_declarations                                         { Type* type = getLLVMType(VALUE_INTTYPE);
                                                                                                                                          char* id = $2;
                                                                                                                                          vector<pair<Type*,char*>*>* paramList = $4;
                                                                                                                                          deque<ExprAst*>* stmtList = $6;
                                                                                                                                          ExprAst* expr = new FunctionExprAst(type, id, paramList, stmtList);
                                                                                                                                          deque<ExprAst*>* exprList = $7;
                                                                                                                                          exprList->push_front(expr);
                                                                                                                                          $$ = exprList; } 
    | T_BOOLTYPE identifier T_LPAREN method_parameters T_RPAREN method_block method_declarations                                        { Type* type = getLLVMType(VALUE_BOOLTYPE);
                                                                                                                                          char* id = $2;
                                                                                                                                          vector<pair<Type*,char*>*>* paramList = $4;
                                                                                                                                          deque<ExprAst*>* stmtList = $6;
                                                                                                                                          ExprAst* expr = new FunctionExprAst(type, id, paramList, stmtList);
                                                                                                                                          deque<ExprAst*>* exprList = $7;
                                                                                                                                          exprList->push_front(expr);
                                                                                                                                          $$ = exprList; }
    | method_declarations                                                                                                               { $$ = $1; }
    ;

field_int_assign: T_INTTYPE identifier T_ASSIGN constant T_SEMICOLON                                                                    { /* Reserved */ }
    ;

field_ints: field_ints T_COMMA field_int                                                                                                { /* Reserved */ }
    | field_int                                                                                                                         { /* Reserved */ }
    ;

field_int: identifier field_quantity                                                                                                    { /* Reserved */ }
    ;

field_bool_assign: T_BOOLTYPE identifier T_ASSIGN constant T_SEMICOLON                                                                  { /* Reserved */ }
    ;

field_bools: field_bools T_COMMA field_bool                                                                                             { /* Reserved */ }
    | field_bool                                                                                                                        { /* Reserved */ }
    ;

field_bool: identifier field_quantity                                                                                                   { /* Reserved */ }
          ;


field_quantity: T_LSB number T_RSB                                                                                                      { /* Reserved */ }
    | /* Not an array - scalar value */                                                                                                 { /* Reserved */ }
    ;



method_declarations: method_declarations method_declaration                                                                             { deque<ExprAst*>* exprList = $1;
                                                                                                                                          exprList->push_back($2);
                                                                                                                                          $$ = exprList; }
    | method_declaration                                                                                                                { deque<ExprAst*>* exprList = new deque<ExprAst*>;
                                                                                                                                          exprList->push_back($1);
                                                                                                                                          $$ = exprList; }
    | /* No method declarations */                                                                                                      { $$ = new deque<ExprAst*>; }
    ;

method_declaration: T_INTTYPE identifier T_LPAREN method_parameters T_RPAREN method_block                                               { Type* type = getLLVMType(VALUE_INTTYPE);
                                                                                                                                          char* id = $2;
                                                                                                                                          vector<pair<Type*,char*>*>* paramList = $4;
                                                                                                                                          deque<ExprAst*>* stmtList = $6;
                                                                                                                                          FunctionExprAst* expr = new FunctionExprAst(type, id, paramList, stmtList);
                                                                                                                                          $$ = expr; }
                                                                                                                                          
    | T_BOOLTYPE identifier T_LPAREN method_parameters T_RPAREN method_block                                                            { Type* type = getLLVMType(VALUE_BOOLTYPE);
                                                                                                                                          char* id = $2;
                                                                                                                                          vector<pair<Type*,char*>*>* paramList = $4;
                                                                                                                                          deque<ExprAst*>* stmtList = $6;
                                                                                                                                          FunctionExprAst* expr = new FunctionExprAst(type, id, paramList, stmtList);
                                                                                                                                          $$ = expr; }
    | T_VOID identifier T_LPAREN method_parameters T_RPAREN method_block                                                                { Type* type = getLLVMType(VALUE_VOIDTYPE);
                                                                                                                                          char* id = $2;
                                                                                                                                          vector<pair<Type*,char*>*>* paramList = $4;
                                                                                                                                          deque<ExprAst*>* stmtList = $6;
                                                                                                                                          FunctionExprAst* expr = new FunctionExprAst(type, id, paramList, stmtList);
                                                                                                                                          $$ = expr; }
    ;

method_parameters: method_parameters T_COMMA method_parameter                                                                           { vector<pair<Type*,char*>*>* paramList = $1;
                                                                                                                                          paramList->push_back($3); 
                                                                                                                                          $$ = paramList; }
    | method_parameter                                                                                                                  { vector<pair<Type*,char*>*>* paramList = new vector<pair<Type*,char*>*>;
                                                                                                                                          paramList->push_back($1);
                                                                                                                                          $$ = paramList; }
    | /* No method parameters */                                                                                                        { $$ = new vector<pair<Type*,char*>*>; }
    ;

method_parameter: type identifier                                                                                                       { Type* type = getLLVMType($1);
                                                                                                                                          char* id = $2; 
                                                                                                                                          $$ = new pair<Type*,char*>(type, id); }
    ;

method_block: T_LCB variable_declarations statements T_RCB                                                                              { deque<ExprAst*>* exprList = new deque<ExprAst*>;
                                                                                                                                          deque<ExprAst*>* varList = $2;
                                                                                                                                          exprList->insert(exprList->end(), varList->begin(), varList->end());
                                                                                                                                          // Combine variable declaration list and statement list.
                                                                                                                                          deque<ExprAst*>* stmtList = $3;
                                                                                                                                          exprList->insert(exprList->end(), stmtList->begin(), stmtList->end());
                                                                                                                                          $$ = exprList; }
    ;




variable_declarations: variable_declarations variable_declaration                                                                       { deque<ExprAst*>* existingVarList = $1;
                                                                                                                                          deque<ExprAst*>* newVarList = $2;
                                                                                                                                          // Append the variable declarations onto the existing list.
                                                                                                                                          existingVarList->insert(existingVarList->end(), newVarList->begin(), newVarList->end());
                                                                                                                                          $$ = existingVarList;}
    | variable_declaration                                                                                                              { $$ = $1; }
    | /* No variable declarations */                                                                                                    { $$ = new deque<ExprAst*>; }
    ;

variable_declaration: T_INTTYPE variable_ints T_SEMICOLON                                                                               { $$ = $2; }
    | T_BOOLTYPE variable_bools T_SEMICOLON                                                                                             { $$ = $2; }
    ;

variable_ints: variable_ints T_COMMA variable_int                                                                                       { deque<ExprAst*>* varList = $1;
                                                                                                                                          varList->push_back($3);
                                                                                                                                          $$ = varList; }
    | variable_int                                                                                                                      { deque<ExprAst*>* varList = new deque<ExprAst*>; 
                                                                                                                                          varList->push_back($1);
                                                                                                                                          $$ = varList; }
    ;
             
variable_int: identifier                                                                                                                { Type* type = getLLVMType(VALUE_INTTYPE);
                                                                                                                                          char* id = $1;
                                                                                                                                          $$ = new VarDeclExprAst(type, id); }
    ;

variable_bools: variable_bools T_COMMA variable_bool                                                                                    { deque<ExprAst*>* varList = $1;
                                                                                                                                          varList->push_back($3);
                                                                                                                                          $$ = varList; }
    | variable_bool                                                                                                                     { deque<ExprAst*>* varList = new deque<ExprAst*>; 
                                                                                                                                          varList->push_back($1);
                                                                                                                                          $$ = varList; }
    ;
              
variable_bool: identifier                                                                                                               { Type* type = getLLVMType(VALUE_BOOLTYPE);
                                                                                                                                          char* id = $1;
                                                                                                                                          $$ = new VarDeclExprAst(type, id); }

statements: statements statement                                                                                                        { deque<ExprAst*>* stmtList = $1;
                                                                                                                                          stmtList->push_back($2);
                                                                                                                                          $$ = stmtList; }
    | statement                                                                                                                         { deque<ExprAst*>* stmtList = new deque<ExprAst*>;
                                                                                                                                          stmtList->push_back($1);
                                                                                                                                          $$ = stmtList; }
    | /* No statements */                                                                                                               { $$ = new deque<ExprAst*>; }
    ;

statement: block                                                                                                                        { $$ = $1; }
    | assignment T_SEMICOLON                                                                                                            { $$ = $1; }
    | method_call T_SEMICOLON                                                                                                           { $$ = $1; }
    | statement_if                                                                                                                      { /* Reserved */ }
    | statement_while                                                                                                                   { /* Reserved */ }
    | statement_for                                                                                                                     { /* Reserved */ }
    | statement_return T_SEMICOLON                                                                                                      { /* Reserved */ }
    | T_BREAK T_SEMICOLON                                                                                                               { /* Reserved */ }
    | T_CONTINUE T_SEMICOLON                                                                                                            { /* Reserved */ }
    ;

block: T_LCB variable_declarations statements T_RCB                                                                                     { deque<ExprAst*>* exprList = new deque<ExprAst*>;
                                                                                                                                          deque<ExprAst*>* varList = $2;
                                                                                                                                          exprList->insert(exprList->end(), varList->begin(), varList->end());
                                                                                                                                          // Combine variable declaration list and statement list.
                                                                                                                                          deque<ExprAst*>* stmtList = $3;
                                                                                                                                          exprList->insert(exprList->end(), stmtList->begin(), stmtList->end());
                                                                                                                                          $$ = new BlockExprAst(exprList); }
     ;

statement_if: T_IF T_LPAREN expression T_RPAREN block statement_else                                                                    { /* Reserved */ }
    ;

statement_else: T_ELSE block                                                                                                            { /* Reserved */ }
    | /* No else (optional) */                                                                                                          { /* Reserved */ }
    ;

statement_while: T_WHILE T_LPAREN expression T_RPAREN block                                                                             { /* Reserved */ }
    ;

statement_for: T_FOR T_LPAREN assignments T_SEMICOLON expression T_SEMICOLON assignments T_RPAREN block                                 { /* Reserved */ }
    ;

statement_return: T_RETURN T_LPAREN expression T_RPAREN                                                                                 { /* Reserved */ }
    | T_RETURN T_LPAREN T_RPAREN                                                                                                        { /* Reserved */ }
    | T_RETURN                                                                                                                          { /* Reserved */ }
    ;

assignments: assignments T_COMMA assignment                                                                                             { /* Reserved */ }
    | assignment                                                                                                                        { /* Reserved */ } 
    | /* No assignments */                                                                                                              { /* Reserved */ }

assignment: identifier T_ASSIGN expression                                                                                              { char* id = $1; 
                                                                                                                                          ExprAst* resultAst = $3;
                                                                                                                                          $$  = new VarAssignExprAst(id, resultAst); }
                                                                                                                                          
                                                                                                                                         
    | identifier T_LSB expression T_RSB T_ASSIGN expression                                                                             { /* Reserved */ }
    ;

method_call: identifier T_LPAREN method_arguments T_RPAREN                                                                              { char* id = $1;
                                                                                                                                          deque<ExprAst*>* argList = $3;
                                                                                                                                          $$ = new FunctionCallExprAst(id, argList); }
    ;

method_arguments: method_arguments T_COMMA method_argument                                                                              { deque<ExprAst*>* argList = $1;
                                                                                                                                          argList->push_back($3);
                                                                                                                                          $$ = argList; }
    | method_argument                                                                                                                   { deque<ExprAst*>* argList = new deque<ExprAst*>;
                                                                                                                                          argList->push_back($1);
                                                                                                                                          $$ = argList; }
    | /* No argument passed */                                                                                                          { $$ = new deque<ExprAst*>; }
    ;

method_argument: expression                                                                                                             { $$ = $1; }
                                                                                                                                          
    ;

expression: p1_expression                                                                                                               { $$ = $1; }
    ;

p1_expression: p2_expression                                                                                                            { $$ = $1; }
    | p1_expression p1_operator p2_expression                                                                                           { $$ = new BinaryExprAst($2, $1, $3); }
    ;
p2_expression: p3_expression                                                                                                            { $$ = $1; }
    | p2_expression p2_operator p3_expression                                                                                           { $$ = new BinaryExprAst($2, $1, $3); }
    ;

p3_expression: p4_expression                                                                                                            { $$ = $1; }
    | p3_expression p3_operator p4_expression                                                                                           { $$ = new BinaryExprAst($2, $1, $3); }
    ;

p4_expression: p5_expression                                                                                                            { $$ = $1; }
    | p4_expression p4_operator p5_expression                                                                                           { $$ = new BinaryExprAst($2, $1, $3); }
    ;

p5_expression: root_expression                                                                                                          { $$ = $1; }
    | p5_expression p5_operator root_expression                                                                                         { $$ = new BinaryExprAst($2, $1, $3); }
    ;

root_expression: expression_variable                                                                                                    { $$ = $1; }
    | method_call                                                                                                                       { $$ = $1; }
    | constant                                                                                                                          { $$ = $1; }
    | unary_operator root_expression                                                                                                    { $$ = new UnaryExprAst($1, $2); }
    | T_LPAREN expression T_RPAREN                                                                                                      { $$ = $2; }
    ;

expression_variable: identifier                                                                                                         { char* id = $1;
                                                                                                                                          $$ = new VarExprAst(id); }
    | identifier T_LSB expression T_RSB                                                                                                 { /* Reserved */ }
    ;

identifier: T_ID                                                                                                                        { $$ = strdup(yytext); }
    ;

extern_type: T_STRINGTYPE                                                                                                               { $$ = (char*) VALUE_STRINGTYPE; }
    | method_type                                                                                                                       { $$ = $1; }
    ;

method_type: T_VOID                                                                                                                     { $$ = (char*) VALUE_VOIDTYPE; }
    | type                                                                                                                              { $$ = $1; }
    ;

type: T_INTTYPE                                                                                                                         { $$ = (char*) VALUE_INTTYPE; }
    | T_BOOLTYPE                                                                                                                        { $$ = (char*) VALUE_BOOLTYPE; }
    ;

constant: boolean_constant                                                                                                              { char* boolVal = $1; 
                                                                                                                                          ExprAst* expr;
                                                                                                                                          if (strcmp(boolVal, VALUE_TRUE) == 0) { 
                                                                                                                                              expr = new BoolConstExprAst(true);
                                                                                                                                          } else { 
                                                                                                                                              expr = new BoolConstExprAst(false);
                                                                                                                                          } 
                                                                                                                                          $$ = expr; }
    | number                                                                                                                            { $$ = new IntConstExprAst($1); }
    | string                                                                                                                            { $$ = new StringConstExprAst($1); }
    ;

boolean_constant: T_TRUE                                                                                                                { $$ = (char*) VALUE_TRUE; }
    | T_FALSE                                                                                                                           { $$ = (char*) VALUE_FALSE; }
    ;

string: T_STRINGCONSTANT                                                                                                                { $$ = copyString(yytext, yyleng); }
    ;

number: T_INTCONSTANT                                                                                                                   { $$ = $1; }
    | T_CHARCONSTANT                                                                                                                    { $$ = $1;}
    ;

p1_operator: T_OR                                                                                                                       { $$ = (char*) VALUE_OR; }
    ;

p2_operator: T_AND                                                                                                                      { $$ = (char*) VALUE_AND; }
    ;

p3_operator: T_EQ                                                                                                                       { $$ = (char*) VALUE_EQ; }
    | T_NEQ                                                                                                                             { $$ = (char*) VALUE_NEQ; }
    | T_LT                                                                                                                              { $$ = (char*) VALUE_LT; }
    | T_LEQ                                                                                                                             { $$ = (char*) VALUE_LEQ; }
    | T_GT                                                                                                                              { $$ = (char*) VALUE_GT; }
    | T_GEQ                                                                                                                             { $$ = (char*) VALUE_GEQ; }
    ;

p4_operator: T_PLUS                                                                                                                     { $$ = (char*) VALUE_PLUS; }
    | T_MINUS                                                                                                                           { $$ = (char*) VALUE_MINUS; }
;

p5_operator: T_MULT                                                                                                                     { $$ = (char*) VALUE_MULT; }
    | T_DIV                                                                                                                             { $$ = (char*) VALUE_DIV; }
    | T_MOD                                                                                                                             { $$ = (char*) VALUE_MOD; }
    | T_RIGHTSHIFT                                                                                                                      { $$ = (char*) VALUE_RIGHTSHIFT; }
    | T_LEFTSHIFT                                                                                                                       { $$ = (char*) VALUE_LEFTSHIFT; }
    ;

unary_operator: T_NOT                                                                                                                   { $$ = (char*) VALUE_NOT; }
    | T_MINUS                                                                                                                           { $$ = (char*) VALUE_NEGATE; }
    ;

push_symtbl: /* Empty */                                                                                                                { pushSymbolTable(); }
    ;


%%

// Entry point to program
int main() {
    initializeLLVM();

    int exitVal = yyparse();
    /* printf("====================================================================================\n"); */
    getModule()->dump();
    /* debug(); */

    return 0;
    //return (exitVal >= EXIT_FAILURE ? EXIT_FAILURE : EXIT_SUCCESS);
}

/* Report syntax at with line number and text that caused such to standard error. */
/* void yyerror(char const* s) { */
/*     fprintf(stderr,"%d: %s at %s\n", yylineno, s, yytext); */
/*     exit(EXIT_FAILURE); */
/* } */

/* Copy the provided string, while also converting the escape characters contained within the string to 
   their normal representation. The allocated string may allocate more space than what is actually needed.*/
char* copyString(char* str, int length) {
    char* newStr = (char*) malloc(sizeof(char) * length);
    
    char* ptr = newStr;
    while (*str != '\0') {
         // Convert the next character to escaped character current
         // character is an escape character.
        if (*str == '\\') {
            str++;
            *ptr = escapeCharacter(*str);
        } else {
            *ptr = *str;
        } 
        ptr++;
        str++;
    }
    *ptr = '\0';

    return newStr;
}

/* Given a character, return the escaped version of said character. The program will exit with a failure code if not a 
   recognized escape character. */
char escapeCharacter(char escapedChar) {
    char returnChar;
    switch(escapedChar) {
        case 'n':
            returnChar = '\n';
            break;
        case 'r':
            returnChar = '\r';
            break;
        case 't':
            returnChar = '\t';
            break;
        case 'v':
            returnChar = '\v';
            break;
        case 'f':
            returnChar = '\f';
            break;
        case 'a':
            returnChar = '\a';
            break;
        case 'b':
            returnChar = '\b';
            break;
        case '\\':
            returnChar = '\\';
            break;
        case '\'':
            returnChar = '\'';
            break;
        case '"':
            returnChar = '"';
            break;
        case '\0':
            returnChar = '\0';
            break;
        default:
            fprintf(stderr, "Invalid escaped character.\n");
            exit(EXIT_FAILURE);
     }
    
    return returnChar;
}

/* Generate code for all externs. */
void generateExterns(vector<ExternExprAst*>* externList) {
    for (vector<ExternExprAst*>::iterator it = externList->begin(); it != externList->end(); it++) {
        ExternExprAst* expr = *(it);
        expr->generateCode();
    }
}

/* Generate code for the class. */
void generateClass(deque<ExprAst*>* exprList) {
    for (deque<ExprAst*>::iterator it = exprList->begin(); it != exprList->end(); it++) {
        ExprAst* expr = *(it);
        expr->generateCode();
    }

    for (deque<ExprAst*>::iterator it = exprList->begin(); it != exprList->end(); it++) {
        FunctionExprAst* expr = (FunctionExprAst*) *(it);
        expr->generateDeferedCode();
    }
}
