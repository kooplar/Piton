/*Bryan Martin Tostado
 *   created: 9 march 2014
 *
 *   Description:
 *     functions for lexical analisis
 *
 *   Notes:
 *
 */
#ifndef __LEXER__
  #define __LEXER__

#include "files.h"

#define NUMKEYWORDS 29
#define NUMLOGOPS 3
#define NUMLOGCONS 2

#define ERR        -1
#define ER         -1
#define AC        999
// ******************* Token categories
#define ID          1
#define NUM         2
#define STRING      3
#define REL_OP      4 //RELATIONAL OPERATOR
#define ARITH_OP    5 //ARITHMETIC OPERATOR
#define DELIM       6 //DELIMITER
#define CMNT        7
// 8-10 RESERVED

// ******************Token types

// for CATEGORY ID
#define VAR        11
#define FUNC       12
#define KEY        13 //keyword
#define LOG_OP     14 //LOGICAL OPERATOR (y, o, no)
#define LOG_CONS    38 //LOGICAL CONSTANT (verdadero,falso) 
//for CATEGORY NUM
#define FLOAT      15
#define INT        16
//for CATEGORY STRING
//for CATEGORY REL_OP
#define LT         17 //LESS THAN
#define LTE        18 //LESS THAN OR EQUAL
#define ASS        19 //ASSIGNMENT
#define COMP       20 //COMPARISON
#define DIFF       21
#define GT         22 //GREATER THAN
#define GTE        23 //GREATER THAN OR EQUAL
//for CATEGORY ARITH_OP
#define PLUS       24
#define MINUS      25
#define MULT       26
#define EXP        27 //EXPONENT
#define DIV        28
#define FDIV       29 //FLOOR DIVISION
#define MOD        30 //MODULUS (%)
//for CATEGORY DELIM
#define LB         31 //LEFT BRACKET
#define RB         32 //RIGHT BRACKET
#define LP         33 //LEFT PARENTHESIS
#define RP         34 //RIGHT PAREN
#define BS         35 //BLOCK START
#define BE         36 //BLOCK END
#define COMMA      37 //COMMA, for lists
//38 taken

//quick fix for semantic check on keywords y,o,no
#define AND     39
#define OR      40
#define NOT     41
#define LOGICAL 42

#define NO_TYPE 444 //for semantic, when an invalid type is produced

/*Global Vars*/
extern buffer_t *buffer;

/*definitions*/
struct token_t
{
  int category;
  int type;
  char *lexeme;
  //int type (int, float, etc)
};
typedef struct token_t token_t;

/*FUNCTIONS*/

/*registerBuff
 *  Description:
 *    saves the buffer ponter locally so you dont haveto pass it in to get the next token everytime.
 *
 *  Notes:
 *    DONT USE THIS IF YOURE GOING TO COMPILE USING THREADS!!
 *
 *  Returns:
 *    
 */
void registerBuff(buffer_t *buff);

/* nextTok 
 *   Description:
 *    finds the next token by using the automata beggining from buffer's offset value 
 *   
 *   Returns:
 *    returns the next token found 
 */
token_t nextTok(buffer_t *buffer);
/* nexTok
 *  Description:
 *    Calls nextTok with buffer = to the locally saved buffer from registerBuff call
 *  Returns:
 *    returns the call to nextToken(..)
 **/
token_t nexTok();


/*these 2 convert from a numeric representation to string*/
char* tokenCategory2String(int type);
char* tokenType2String(int type);
//calls the above two in an attemp to match any num to category/type
char* token2String(int type);

/* frees all the lexemes that have been returnd by nexTok or nextTok so far */
void freeLexemes();




  #endif /* __LEXER__ */
