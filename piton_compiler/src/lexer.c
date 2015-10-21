/*Bryan Martin Tostado
 *   creation: 9 march 2014
 *
 *   Description:
 *     Functions to perform lexical analisis for Piton
 *
 *   Notes:
 *     After clasifying a token, if its an ID, then it is inserted into the current symtab. It is the 
 *     job of the syntactic analyzer to keep symtab scope correct and to clasify the ID token as VAR or FUNC as analyzed.
 *
 */

#include "include/lexer.h"
#include "include/symtab.h"
#include "include/errors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*static function prototypes*/

/* charToType
 *   Description:
 *    returns the numerical type of the char. see diagrams
 *   Returns:
 *    the type of the transition corresponding to the char
 */
static int charToType(char c);

/* rmWhites
*   Description:
*    removes all whites and leaves buffer.offset at the next non white.
*    lineNum is incremented at every new line found
*    charNum is incremented for every space or tab. it is set to = 1 after lineNum++
*   Returns:
*     nothing
*/
static void rmWhites(buffer_t **buffer);

/* IDType
 *  Description:
 *    Tells the type (keyword,logical operator/constant/) of the lexeme
 *  Returns:
 *   The type ( as define in lexer.h
 */
static int IDType(char *symbol);

/*global variables*/
buffer_t *buffer = NULL;

static char *lexeme_ptr_list[10000];//magic number just for temprary tests
static int lexeme_ptr_list_idx = 0;
void freeLexemes()
{
    for(int i=0;i<lexeme_ptr_list_idx;i++)
    {
      if(lexeme_ptr_list[i] != NULL /*|| strcmp(lexeme_ptr_list[i],"") != 0*/) //"" is asigned as rom memory, not malloc'ed
      {
      //  printf("freeing: %s : %p\n",lexeme_ptr_list[i],&(*lexeme_ptr_list[i]));
        free(lexeme_ptr_list[i]);
      //  lexeme_ptr_list[i] = NULL;
      }
    }
    lexeme_ptr_list_idx = 0;
}


/*static global variables*/
static char logops[NUMLOGOPS][10] = {
  {"y"},
  {"o"},
  {"no"},
  };

static char logcons[NUMLOGCONS][15] = {
  {"verdadero"},
  {"falso"},
  };

static char keywords[NUMKEYWORDS][15] = {
  {"produce"},
  {"afirmar"},
  {"interrumpe"},
  {"clase"},
  {"continua"},
  {"def"},
  {"del"},
  {"otsi"},
  {"otro"},
  {"excepto"},
  {"ejec"},
  {"final"},
  {"para"},
  {"desde"},
  {"global"},
  {"si"},
  {"importar"},
  {"en"},
  {"es"},
  {"con"},
  {"mientras"},
  {"pasa"},
  {"imprime"},
  {"eleva"},
  {"regresa"},
  {"prueba"},
  {"mientras"},
  {"lee"}, 
  {"sino"}
  };

static int tranMat[33][25] = {
//          1;_  2;alpha  3;dig  4;.  5;'  6;" 7;#  8;\n 9;< 10;= 11;! 12;> 13;+ 14;- 15;* 16;/ 17;% 18;[ 19;] 20;( 21;) 22;: 23;. 24;all_else
/*0 */   {0, 1   , 1      , 2    ,31  ,4  , 7  , 9  ,ER  ,11 ,13  ,15  ,17  ,19  ,20  ,21  ,23  ,25  ,26  ,27  ,28  ,29  ,30  ,32  ,ER} ,// start
/*1 */   {0, 1   , 1      , 1    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// id
/*2 */   {0,AC   ,AC      , 2    , 3  ,AC ,AC  ,AC  ,AC  ,AC ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// int
/*3 */   {0,AC   ,AC      , 3    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// float
/*4 */   {0, 5   , 5      , 5    , 5  ,6  , 5  , 5  , 5  , 5 , 5  , 5  , 5  , 5  , 5  , 5  , 5  , 5  , 5  , 5  , 5  , 5  , 5  , 5  , 5} ,
/*5 */   {0, 5   , 5      , 5    , 5  ,6  , 5  , 5  , 5  , 5 , 5  , 5  , 5  , 5  , 5  , 5  , 5  , 5  , 5  , 5  , 5  , 5  , 5  , 5  , 5} ,
/*6 */   {0,AC   ,AC      ,AC    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// string
/*7 */   {0, 8   , 8      , 8    , 8  , 8 , 6  , 8  , 8  , 8 , 8  , 8  , 8  , 8  , 8  , 8  , 8  , 8  , 8  , 8  , 8  , 8  , 8  , 8  , 8} ,
/*8 */   {0, 8   , 8      , 8    , 8  , 8 , 6  , 8  , 8  , 8 , 8  , 8  , 8  , 8  , 8  , 8  , 8  , 8  , 8  , 8  , 8  , 8  , 8  , 8  , 8} ,
/*9 */   {0, 9   , 9      , 9    , 9  , 9 , 9  , 9  ,10  , 9 , 9  , 9  , 9  , 9  , 9  , 9  , 9  , 9  , 9  , 9  , 9  , 9  , 9  , 9  , 9} ,
/*10*/   {0,AC   ,AC      ,AC    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// #cmnt
/*11*/   {0,AC   ,AC      ,AC    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,12  ,AC  ,16  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// <
/*12*/   {0,AC   ,AC      ,AC    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// <=
/*13*/   {0,AC   ,AC      ,AC    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,14  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// =
/*14*/   {0,AC   ,AC      ,AC    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// ==
/*15*/   {0,ER   ,ER      ,ER    ,ER  ,ER ,ER  ,ER  ,ER  ,ER ,16  ,ER  ,ER  ,ER  ,ER  ,ER  ,ER  ,ER  ,ER  ,ER  ,ER  ,ER  ,ER  ,ER  ,ER} ,
/*16*/   {0,AC   ,AC      ,AC    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// !=,<>
/*17*/   {0,AC   ,AC      ,AC    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,18  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// >
/*18*/   {0,AC   ,AC      ,AC    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// >=
/*19*/   {0,AC   ,AC      ,AC    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// +
/*20*/   {0,AC   ,AC      ,AC    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// -
/*21*/   {0,AC   ,AC      ,AC    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,AC  ,AC  ,AC  ,AC  ,AC  ,22  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// *
/*22*/   {0,AC   ,AC      ,AC    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// **
/*23*/   {0,AC   ,AC      ,AC    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,24  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// /
/*24*/   {0,AC   ,AC      ,AC    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// //
/*25*/   {0,AC   ,AC      ,AC    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// %
/*26*/   {0,AC   ,AC      ,AC    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// [
/*27*/   {0,AC   ,AC      ,AC    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// ]
/*28*/   {0,AC   ,AC      ,AC    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// (
/*29*/   {0,AC   ,AC      ,AC    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// )
/*30*/   {0,AC   ,AC      ,AC    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// : 
/*31*/   {0,AC   ,AC      , 3    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// .
/*32*/   {0,AC   ,AC      ,AC    ,AC  ,AC ,AC  ,AC  ,AC  ,AC ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC  ,AC} ,// ,
         };

static int charToType(char c)
{ 
  if(isalpha(c))
    return 2;
  else  if(isdigit(c))
    return 3;
  
  switch(c)
  { 
    case '_': return 1;
//case 2 above
//case 3 above
    case '.': return 4;
    case '\'': return 5;
    case '\"': return 6;
    case '#': /*buffer->lineNum++;printf("incrementing line num to:%i\n",buffer->lineNum);*/return 7;
    case '\n':/*printf("     jumping! to: %i\n",buffer->lineNum+1); buffer->lineNum++;*//* buffer->charNum=1;*/return 8;
    case '<': return 9;
    case '=': return 10;
    case '!': return 11;
    case '>': return 12;
    case '+': return 13;
    case '-': return 14;
    case '*': return 15;
    case '/': return 16;
    case '%': return 17;
    case '[': return 18;
    case ']': return 19;
    case '(': return 20;
    case ')': return 21;
    case ':': return 22;
    case ',': return 23;
//    case '\t':
    default: return 24;
  }
  return -1; 
}

void registerBuff(buffer_t *buff)
{
  buffer = buff;
}

token_t nexTok()
{
  if(buffer == NULL)
  {
    perror("Trying to call nexTok, but buffer is null!");
    return (token_t){-1,-1,NULL};
  }
  return nextTok(buffer);
}

token_t nextTok(buffer_t *buffer)
{  
  //printf("    %i | %i",buffer->offset+1, buffer->size);
  if(buffer->offset+1 >= buffer->size) //+1 needed because offset wont save EOF
  {
      //exiting here has many implications, one is that a var at the end of file wont be saved, but thats ok
   //cleanExit(SUCCESS,"DEBUG No more tokens!! Exiting.");
    //printf("DEBUG no more tokens, returning empty token!\n");
    return (token_t){-1,-1,""};
  }

  token_t token = {-1,-1, ""};
  int lexeme_length = 0;
  int state = 0;
  int prevState = 0;
  char transition = 0; 
  int already_jumped = 0; //dirty fix for comments overcounting line jumps!
  rmWhites(&buffer);
  //check if after removing whites we are at the end
  if( buffer->offset+1 >= buffer->size)
    return (token_t){-1,-1,""};
  
  do
  {
    prevState = state;
    //protect from buffer over-offseting, for example by never ending string
    if(buffer->advance+1 == buffer->size && (state == 8 || state == 5))
    {
      pLexString(" The file ended and the string never terminated\n");
      cleanExit(FAILURE,"EXITED WITH ERRORS");
    }
    else
    { 
      transition = charToType(buffer->buf[buffer->advance++]);
      state = tranMat[state][transition];

      if(state  == 9 && (!already_jumped)) //if its a comment, jump a line since the \n is part of the comment
       { /*buffer->charNum=1;*/already_jumped++;buffer->lineNum++;    }//dirty fix..

      buffer->charNum++;
    }
  }while(state != AC && state != ER);
    buffer->charNum--; //remove lookahead .
  already_jumped = 0; //see comment for var

  //setup the token
  if(state != ER)
    buffer->advance--;  
  lexeme_length = buffer->advance - buffer->offset ;
  token.lexeme = malloc((sizeof(char)) * (lexeme_length + 1)); 
  lexeme_ptr_list[lexeme_ptr_list_idx++] = token.lexeme;
  strncpy(token.lexeme,&((buffer->buf)[buffer->offset]), lexeme_length);
  token.lexeme[lexeme_length] = '\0';
  buffer->offset = buffer->advance;
 
   
  //classify token category and type
  switch(prevState)
  {
    case 1: token.category = ID; token.type =  IDType(token.lexeme);break;
            //type is only resolved for KEY, LOG_OP. FUNC OR VAR is resolved at syntactic analysis

    case 2: token.category = NUM; token.type = INT; break;
    case 3: token.category = NUM; token.type = FLOAT; break;

    case 6: token.category = token.type = STRING; break;

    case 10: token.category = token.type = CMNT; break;

    case 11: token.category = REL_OP; token.type = LT;break;
    case 12: token.category = REL_OP; token.type = LTE;break;
    case 13: token.category = REL_OP; token.type = ASS;break;
    case 14: token.category = REL_OP; token.type = COMP;break;
    case 16: token.category = REL_OP; token.type = DIFF;break;
    case 17: token.category = REL_OP; token.type = GT;break;
    case 18: token.category = REL_OP; token.type = GTE;break;

    case 19: token.category = ARITH_OP; token.type = PLUS;break;
    case 20: token.category = ARITH_OP; token.type = MINUS;break;
    case 21: token.category = ARITH_OP; token.type = MULT;break;
    case 22: token.category = ARITH_OP; token.type = EXP;break;
    case 23: token.category = ARITH_OP; token.type = DIV;break;
    case 24: token.category = ARITH_OP; token.type = FDIV;break;
    case 25: token.category = ARITH_OP; token.type = MOD;break;

    case 26: token.category = DELIM; token.type = LB;break;
    case 27: token.category = DELIM; token.type = RB;break;
    case 28: token.category = DELIM; token.type = LP;break;
    case 29: token.category = DELIM; token.type = RP;break;
    case 30: token.category = DELIM; token.type = BS;break;
    case 31: token.category = DELIM; token.type = BE;break;
    case 32: token.category = DELIM; token.type = COMMA;break;
   
//error states
    case 4: printf("DEBUG ERROR state 4\n");break; 
    case 5: printf("DEBUG ERROR state 5\n");break;
    case 7: printf("DEBUG ERROR state 7\n");break;
    case 8: printf("DEBUG ERROR state 8\n");break;
    case 9: printf("DEBUG ERROR state 9\n");break;
    default: pLexSymString(" This symbol is not  part of the language: ",token.lexeme);//,buffer->buf[buffer->advance]);
  }
 // printf("Debug: lexeme:%s category:%s type:%s",token.lexeme,tokenCategory2String(token.category),tokenType2String(token.type)); 
  //     printf(" -  %i  - \n", buffer->charNum); 
  
  if( (prevState == ER)  || (state == ER) ) //safekeepin
    return (token_t){-1,-1,/*NULL*/""};
  
  //DONT RETURN TOKEN COMMENTS! maybe something can be done with these here..write to a file for a "javadoc" type of crap
  if(token.category == CMNT) 
  {
    free(token.lexeme);
    lexeme_ptr_list_idx--;
    token.lexeme = NULL;
    return nextTok(buffer);
  }
  else         
    return token;
}

static void rmWhites(buffer_t **buffer)
{
  while(((*buffer)->buf)[(*buffer)->offset] == ' ' ||
        ((*buffer)->buf)[(*buffer)->offset] == '\t'||
        ((*buffer)->buf)[(*buffer)->offset] == '\n')
  {
    if(((*buffer)->buf)[(*buffer)->offset] == '\n')
    {
        (*buffer)->lineNum++;
//printf("				 rm whites jumping! to: %i\n",(*buffer)->lineNum);
        (*buffer)->charNum = 1;
    }
    else
    {
      (*buffer)->charNum++;
    }
    ((*buffer)->offset)++;
  } 
  ((*buffer)->advance) = ((*buffer)->offset);
}

static int IDType(char *symbol)
{ 
  int i = 0;
  for(i = 0;i<NUMKEYWORDS;i++)
  {
    if(strcmp(symbol,&keywords[i][0]) == 0 )
    {
     return KEY;
    }
  }
  for(i = 0;i<NUMLOGOPS;i++)
  {
    if(strcmp(symbol,&logops[i][0]) == 0)
    {
      return LOG_OP;
    }
  }
  for(i = 0;i<NUMLOGCONS;i++)
  {
    if(strcmp(symbol,&logcons[i][0]) == 0)
    {
      return LOG_CONS;
    }
  }

  return VAR;//can be vAR or FUNC, assuming VAR. All code will test against VAR!
}

char* tokenCategory2String(int type)
{
  switch(type)
  {
    case -1: return "Invalid token"; break;
    case ID: return "Identifier"; break;
    case NUM: return "Number"; break;
    case STRING: return "String"; break;
    case REL_OP: return "Relational Operator"; break;
    case ARITH_OP: return "Arithmetic Operator"; break;
    case DELIM: return "Delimeter"; break;
    case CMNT: return "Comment"; break;
     default:  return "Invalid"; break;
  }
 return "Invalid";
}

char* tokenType2String(int type)
{
  switch(type)
  {
    case -1: return "Invalid token"; break;
    case VAR: return "Variable"; break;
    case FUNC: return "Function"; break;
    case KEY: return "Keyword"; break;
    case LOG_OP: return "Logical Operator"; break;
    case LOG_CONS: return "Logical Constant"; break;
    case FLOAT: return "Float"; break;
    case INT: return "Integer"; break;
    case LT: return "Less Than"; break;
    case LTE: return "Less Than Equal"; break;
    case ASS: return "Assignment"; break;
    case COMP: return "Comparison"; break;
    case DIFF: return "Difference"; break;
    case GT: return "Greater Than"; break;
    case GTE: return "Greater Than Equal"; break;
    case PLUS: return "Plus"; break;
    case MINUS: return "Minus"; break;
    case MULT: return "Multiplication"; break;
    case EXP: return "Exponent"; break;
    case DIV: return "Division"; break;
    case FDIV: return "Floor Division"; break;
    case MOD: return "Modulus"; break;
    case LB: return "Left Bracket"; break;
    case RB: return "Right Bracket"; break;
    case LP: return "Left Parenthesis"; break;
    case RP: return "Right Parenthesis"; break;
    case BS: return "Block Start"; break;
    case BE: return "Block End"; break;
    case COMMA: return "Comma"; break;
    case STRING: return "String";break;
    case AND: return "and"; break;
    case OR: return "or";break;
    case NOT: return "not";break;
    //special case
    case NO_TYPE: return "Invalid type";break;
    default: return "[empty]";  break; //returning nothing because some calls to printign errors might need this
  }
  return "UNKNOWN";
}

char* token2String(int type)
{ 
  char* msg = tokenCategory2String(type);
  if(strcmp(msg,"Invalid") != 0)
    return msg;

  return tokenType2String(type);   
}

