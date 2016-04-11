%{
#include "exprdefs.h"
#include "decaf-codegen.tab.h"
#include <stdbool.h>

// Constants
#define ESCAPE_CHAR_COUNT                         10
#define DEFAULT_LINE_POSITION                      1

extern char escapeCharacter(char escapedChar);

void updateLinePosition();
void verifyString(char* str, int strlen);
void verifyCharacterString(char* str, int strlen);
void verifyCharacterInBounds(char character);
bool isEscapedCharacter(char character);
int convertChararacterString(char* str);
void printError(const char* errorMsg);

const char* ERROR_MESSAGE_INVALID_ESCAPE_SEQUENCE = "unknown escape sequence in string constant";
const char* ERROR_MESSAGE_NEWLINE_IN_STRING = "newline in string constant";
const char* ERROR_MESSAGE_CHAR_EMPTY = "char constant has zero width";
const char* ERROR_MESSAGE_CHAR_TOO_LONG = "char constant length is greater than one";
const char* ERROR_MESSAGE_UNTERMINATED_CHAR = "unterminated char constant";
const char* ERROR_MESSAGE_UNEXPECTED_CHAR = "unexpected character in input";

const int LOWER_ASCII_BOUND_A = 7;
const int UPPER_ASCII_BOUND_A = 13;
const int LOWER_ASCII_BOUND_B = 32;
const int UPPER_ASCII_BOUND_B = 126;

const int CHAR_INDEX_OFFSET = 1;
const int ESCAPE_CHAR_INDEX_OFFSET = 1;

const int INT_VALUE_ESCAPE_CHAR = 92;
// Includes n, r, t, v, f, a, b, \, ', and "
const int ESCAPED_CHARS[ESCAPE_CHAR_COUNT] = {110, 114, 116, 118, 102, 97, 98, 92, 39, 34};

int linePos = DEFAULT_LINE_POSITION;

%}

%option yylineno

newline \n
carriage_return \r
horizontal_tab \t
vertical_tab \v
form_feed \f
space [ ]
whitespace ({newline}|{carriage_return}|{horizontal_tab}|{vertical_tab}|{form_feed}|{space})

num [0-9]
hex [a-fA-F0-9]
hex_num 0(x|X){hex}+
int ({num}+|{hex_num})

char_alpha [a-zA-Z]
char_alphanum ({char_alpha}|{num})
char_escape \\(n|r|t|v|f|a|b|\\|\'|\")

/* This excludes the ' and " characters, so it can be 
   re-used for matching both characters and strings. */
char_special (!|#|\$|%|&|\(|\)|\*|\+|,|-|\.|\/|:|;|\<|=|\>|\?|@|\[|\]|\^|_|`|\{|\||\}|~)
char_common ({char_alphanum}|{char_escape}|{char_special})

char_lit ({char_common}|\"|{space})
str_lit ({char_common}|\'|{space}|{horizontal_tab})

identifier ({char_alpha}|_)({char_alphanum}|_)*

%%

&&                                          { updateLinePosition();
                                              return T_AND; }
=                                           { updateLinePosition();
                                              return T_ASSIGN; }
bool                                        { updateLinePosition();
                                              return T_BOOLTYPE; }
break                                       { updateLinePosition();
                                              return T_BREAK; }
\'{char_lit}*\'                             { updateLinePosition();
                                              yylval.num = convertChararacterString(yytext);
                                              return T_CHARCONSTANT; }
\'                                          { printError(ERROR_MESSAGE_UNTERMINATED_CHAR); }
class                                       { updateLinePosition();
                                              return T_CLASS; }
\/\/({char_lit}|{space}|\')*\n              /* Ignore comments */
,                                           { updateLinePosition();
                                              return T_COMMA; }
continue                                    { updateLinePosition();
                                              return T_CONTINUE; }
\/                                          { updateLinePosition();
                                              return T_DIV; }
\.                                          { updateLinePosition();
                                              return T_DOT; }
else                                        { updateLinePosition();
                                              return T_ELSE; }
==                                          { updateLinePosition();
                                              return T_EQ; }
extends                                     { updateLinePosition();
                                              return T_EXTENDS; }
extern                                      { updateLinePosition();
                                              return T_EXTERN; }
false                                       { updateLinePosition();
                                              return T_FALSE; }
for                                         { updateLinePosition();
                                              return T_FOR; }
\>=                                         { updateLinePosition();
                                              return T_GEQ; }
\>                                          { updateLinePosition();
                                              return T_GT; }
if                                          { updateLinePosition();
                                              return T_IF; }
{int}                                       { updateLinePosition();
                                              yylval.num = strtol(yytext, NULL, 0);
                                              return T_INTCONSTANT; }
int                                         { updateLinePosition();
                                              return T_INTTYPE; }
\{                                          { updateLinePosition();
                                              return T_LCB; }
\<\<                                        { updateLinePosition();
                                              return T_LEFTSHIFT; }
\<=                                         { updateLinePosition();
                                              return T_LEQ; }
\(                                          { updateLinePosition();
                                              return T_LPAREN; }
\[                                          { updateLinePosition();
                                              return T_LSB; }
\<                                          { updateLinePosition();
                                              return T_LT; }
-                                           { updateLinePosition();
                                              return T_MINUS; }
%                                           { updateLinePosition();
                                              return T_MOD; }
\*                                          { updateLinePosition();
                                              return T_MULT; }
!=                                          { updateLinePosition();
                                              return T_NEQ; }
new                                         { updateLinePosition();
                                              return T_NEW; }
!                                           { updateLinePosition();
                                              return T_NOT; }
null                                        { updateLinePosition();
                                              return T_NULL; }
\|\|                                        { updateLinePosition();
                                              return T_OR; }
\+                                          { updateLinePosition();
                                              return T_PLUS; }
\}                                          { updateLinePosition();
                                              return T_RCB; }
return                                      { updateLinePosition();
                                              return T_RETURN; }
\>\>                                        { updateLinePosition();
                                              return T_RIGHTSHIFT; }
\)                                          { updateLinePosition();
                                              return T_RPAREN; }
\]                                          { updateLinePosition();
                                              return T_RSB; }
;                                           { updateLinePosition();
                                              return T_SEMICOLON; }
\"{str_lit}*\"                              { updateLinePosition();
                                              return T_STRINGCONSTANT; }
\"                                          { printError(ERROR_MESSAGE_NEWLINE_IN_STRING); }
string                                      { updateLinePosition();
                                              return T_STRINGTYPE; }
true                                        { updateLinePosition();
                                              return T_TRUE; }
void                                        { updateLinePosition();
                                              return T_VOID; }
while                                       { updateLinePosition();
                                              return T_WHILE; }
{identifier}                                { updateLinePosition();
                                              return T_ID; }
{whitespace}+                               { updateLinePosition(); }
.|\n                                        { printError(ERROR_MESSAGE_UNEXPECTED_CHAR); }                                              

                                              

%%

/* Update the position on the current line, used to keep of position
   for error reporting. */
void updateLinePosition() {
    for (int i = 0; i < yyleng; i++) {
        char charValue = yytext[i];
        if (charValue == '\n') {
            linePos = DEFAULT_LINE_POSITION;
        } else {
            linePos += 1;
        }       
    }
}

/* Verify that the provided string of characters is valid. An error 
   will be thrown if an unexpected or illegal character is found 
   within. */
void verifyString(char* str, int strlen) {
    // Ignore first and last characters, always equal to ".
    for (int i = CHAR_INDEX_OFFSET; i < strlen - CHAR_INDEX_OFFSET; i++) {
        verifyCharacterInBounds(str[i]);
    }
    
    int actualStrLen = strlen - (CHAR_INDEX_OFFSET * 2);
    // Handle special invalid case for string "\"
    if (actualStrLen == 1 && str[CHAR_INDEX_OFFSET] == INT_VALUE_ESCAPE_CHAR) {
        printError(ERROR_MESSAGE_NEWLINE_IN_STRING);
    }

    /* Check that all escape characters in the string are followed by a valid
       escaped character (i.e., \n, \t, etc.). */
    for (int i = CHAR_INDEX_OFFSET; i < strlen - (CHAR_INDEX_OFFSET + 1); i++) {
        char currChar = str[i];
        char nextChar = str[i + 1];
        
        // Invalid combination, character cannot be escaped.
        if (currChar == INT_VALUE_ESCAPE_CHAR && !isEscapedCharacter(nextChar)) {
            printError(ERROR_MESSAGE_INVALID_ESCAPE_SEQUENCE);
        }

        // Increment i again to prevent if an escape character is escaping /.
        if (currChar == INT_VALUE_ESCAPE_CHAR && nextChar == INT_VALUE_ESCAPE_CHAR) {
            i++;
        }
    }
}

/* Verify that the provided string of characters is valid. An error
   will be thrown if an unexpected or illegal character is found 
   within, or if it contains too many or an insufficent amount of
   characters. */
void verifyCharacterString(char* str, int strlen) {
    // Ignore first and last character, always equal to '.
    for (int i = CHAR_INDEX_OFFSET; i < strlen - 1; i++) {
        verifyCharacterInBounds(str[i]);
    }

    int actualStrLen = strlen - (CHAR_INDEX_OFFSET * 2);
    switch (actualStrLen) {
        case 0:
            printError(ERROR_MESSAGE_CHAR_EMPTY); 
            break;
        case 1:
            if (str[CHAR_INDEX_OFFSET] == INT_VALUE_ESCAPE_CHAR) {
                printError(ERROR_MESSAGE_UNTERMINATED_CHAR);
            }
            break;
        case 2:
            if (str[CHAR_INDEX_OFFSET] == INT_VALUE_ESCAPE_CHAR) {
                char secondChar = str[CHAR_INDEX_OFFSET + 1];
                if (!isEscapedCharacter(secondChar)) {
                    printError(ERROR_MESSAGE_CHAR_TOO_LONG);
                }
            } else {
                printError(ERROR_MESSAGE_CHAR_TOO_LONG);
            }
            break;
        default:
            printf(ERROR_MESSAGE_CHAR_TOO_LONG);
            break;
    }
}

/* Verify that the provided ASCII characters value is between
   7 - 13 or 32-126. */
void verifyCharacterInBounds(char character) {
   int charVal = character;

   // Value is between 7 - 13
   bool inLowerBound = (LOWER_ASCII_BOUND_A <= charVal && charVal <= UPPER_ASCII_BOUND_A);
   // Value is between 32 - 126
   bool inUpperBound = (LOWER_ASCII_BOUND_B <= charVal && charVal <= UPPER_ASCII_BOUND_B);

   if (!inLowerBound && !inUpperBound) {
       printError(ERROR_MESSAGE_UNEXPECTED_CHAR);
   }
}

/* Determine if the provided character can be "escaped", such that 
   \<character> is valid. */
bool isEscapedCharacter(char character) {
    for (int i = 0; i < ESCAPE_CHAR_COUNT; i++) {
        if (character == ESCAPED_CHARS[i]) {
            return true;
        }
    }
    return false;
}

/* Print the provided error message to the standard error
   stream and half the program with a failed status. */
void printError(const char* errorMsg) {
    fprintf(stderr, "Error: %s\n", errorMsg);
    fprintf(stderr, "Lexical error: line %d, position %d\n", yylineno, linePos);
    exit(EXIT_FAILURE);
}

/* Convert the provided character string to an int, the
   string should be delimited by a single quote, and should
   contain a single character or escape character */
int convertChararacterString(char* str) {
    int returnVal = str[CHAR_INDEX_OFFSET];

    if (str[CHAR_INDEX_OFFSET] == '\\') {
        returnVal = escapeCharacter(str[CHAR_INDEX_OFFSET + 1]);
    }
    
    return returnVal;
}
