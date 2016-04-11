#ifndef VALUE_CONSTANTS_H
#define VALUE_CONSTANTS_H

void throwError(const char* errorMsg, int errorCode);

// Binary Operators
extern const char* VALUE_OR;
extern const char* VALUE_AND;
extern const char* VALUE_EQ;
extern const char* VALUE_NEQ;
extern const char* VALUE_LT;
extern const char* VALUE_LEQ;
extern const char* VALUE_GT;
extern const char* VALUE_GEQ;
extern const char* VALUE_PLUS;
extern const char* VALUE_MINUS;
extern const char* VALUE_MULT;
extern const char* VALUE_DIV;
extern const char* VALUE_MOD;
extern const char* VALUE_RIGHTSHIFT;
extern const char* VALUE_LEFTSHIFT;

// Unary Operators
extern const char* VALUE_NOT;
extern const char* VALUE_NEGATE;

// Constant Values
extern const char* VALUE_TRUE;
extern const char* VALUE_FALSE;
extern const char* VALUE_VOIDTYPE;
extern const char* VALUE_INTTYPE;
extern const char* VALUE_BOOLTYPE;
extern const char* VALUE_STRINGTYPE;
extern const int VALUE_SCALAR;

// Error Messages
extern const char* ERROR_BOOL_TO_INT;
extern const char* ERROR_INT_TO_BOOL;
extern const char* ERROR_INVALID_INT_OP;
extern const char* ERROR_INVALID_BOOL_OP;
extern const char* ERROR_NOT_INT;
extern const char* ERROR_NEGATE_BOOL;
extern const char* ERROR_BINARY_OP_TYPE_MISMATCH;
extern const char* ERROR_GENERIC_TYPE_MISMATCH;
extern const char* ERROR_RETURN_MISMATCH;
extern const char* ERROR_VARIABLE_UNDECLARED;
extern const char* ERROR_FUNCTION_IS_VOID;
extern const char* ERROR_INDEX_TOO_LOW;
extern const char* ERROR_NO_MAIN;


// Exit Values
extern const int EXIT_NO_ERROR;
extern const int EXIT_ERROR;
extern const int EXIT_ASSIGN_TYPE_MISMATCH ;
extern const int EXIT_COMPUTE_TYPE_MISMATCH;
extern int EXIT_VARIABLE_UNDECLARED;
extern const int EXIT_NO_MAIN;



#endif
