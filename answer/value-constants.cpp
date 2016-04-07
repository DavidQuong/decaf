#ifndef VALUE_CONSTANTS_H
#define VALUE_CONSTANTS_H

#include <cstdio>
#include <cstdlib>

void throwError(const char* errorMsg, int errorCode) {
    fprintf(stderr, errorMsg);
    exit(errorCode);
}

// Binary Operators
const char* VALUE_OR = "||";
const char* VALUE_AND = "&&";
const char* VALUE_EQ = "==";
const char* VALUE_NEQ = "!=";
const char* VALUE_LT = "<";
const char* VALUE_LEQ = "<=";
const char* VALUE_GT = ">";
const char* VALUE_GEQ = ">=";
const char* VALUE_PLUS = "+";
const char* VALUE_MINUS = "-";
const char* VALUE_MULT = "*";
const char* VALUE_DIV = "/";
const char* VALUE_MOD = "%";
const char* VALUE_RIGHTSHIFT = "<<";
const char* VALUE_LEFTSHIFT = ">>";

// Unary Operators
const char* VALUE_NOT = "!";
const char* VALUE_NEGATE = "-";

// Constant Values
const char* VALUE_TRUE = "true";
const char* VALUE_FALSE = "false";
const char* VALUE_VOIDTYPE = "void";
const char* VALUE_INTTYPE = "int";
const char* VALUE_BOOLTYPE = "bool";
const char* VALUE_STRINGTYPE = "string";
int VALUE_SCALAR = -1;

// Error Messages
const char* ERROR_BOOL_TO_INT = "Cannot assign a boolean value to an integer variable.\n";
const char* ERROR_INT_TO_BOOL = "Cannot assign an integer value to a boolean variable.\n";
const char* ERROR_INVALID_BOOL_OP = "Cannot perform integer operation on a boolean value or variable.\n";
const char* ERROR_INVALID_INT_OP = "Cannot perform boolean operation on an integer value or variable.\n";
const char* ERROR_NOT_INT = "Cannot negate an integer value with '!' operator.\n";
const char* ERROR_NEGATE_BOOL = "Cannot negate a boolean value with '-' operator.\n";
const char* ERROR_BINARY_OP_TYPE_MISMATCH = "Cannot perform operation with an integer and boolean (type mismatch).\n";
const char* ERROR_RETURN_MISMATCH = "Invalid type for return statement.\n";
const char* ERROR_VARIABLE_UNDECLARED = "Variable has not been declared.\n";
const char* ERROR_NO_MAIN = "There exists no main function.\n";

// Exit Values
int EXIT_NO_ERROR = 0;
int EXIT_ERROR = 1;
/* int EXIT_ASSIGN_TYPE_MISMATCH = -6; */
int EXIT_ASSIGN_TYPE_MISMATCH = 1;
int EXIT_COMPUTE_TYPE_MISMATCH = 1;
int EXIT_VARIABLE_UNDECLARED = 1;
int EXIT_NO_MAIN = 1;

#endif
