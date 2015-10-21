/* Copyright 2014 Bryan Martin Tostado

 This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* Bryan Martin Tostado
 * Description:
 *   virtual machine for PL0 code
 *
 * Creation Date: 12-june-2014
 *
 * Notes:
 *
 * Máquina Virtual PL/0 (Máquina de Pila).
 *
 * Formato:
 *
 * Mnemotécnico dir1, dir2
 *
 * LIT Cte, 0 Carga Constante en Pila.
 *
 * LOD Iden, 0 Carga valor Iden según TabSim a Pila.
 *
 * STO 0, Iden Guarda valor sobre Pila en Iden.
 *
 * JMP 0, Cte Salta a dirección según valor de Cte.
 *
 *        _Ex Salta a dir según valor de Etiq. _Ex.
 *
 * JMC cond,Cte Salta a dir en Cte si cond se cumple.
 *
 *       Cond,_Ex Salta a dir en _Ex si cond se cumple.
 *
 * CAL Idenf,dir Salta a código de Idenf según dir.
 *
 * OPR 0, OpCode Ejecuta operación indicado en OpCode.
 *
 * OPR Iden, 19 Lee de Entrada de Teclado.
 *
 * OpCode
 * 0 Detención de programa.
 * 1 Regresa de Subrutina (Función)
 * 2 Suma
 * 3 Resta
 * 4 Multiplicación
 * 5 División
 * 6 Modulo
 * 7 Potencia
 * 8 Menos Unitario
 * 9 Menor que
 * 10 Mayor que
 * 11 Menor o igual que
 * 12 Mayor o igual que
 * 13 Diferente de
 * 14 Igual a
 * 15 Lógico O
 * 16 Lógico Y
 * 17 Lógico NO
 * 18 Limpia Pantalla
 * 19 Lee
 * 20 Imprime sin bajar de línea
 * 21 Imprime con cambio de línea. 
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

//this was previously a define, bu tnot anymroe since the stack will be allowed to grow
int STACK_SIZE=1000;
#define STACK_INC 500 //if the stack needs to grow, this will be the growth
#define INPUT_BUFFER_SIZE 100

//these defines are for the type of the string (since we dont save the type on the stack, jus tthe string)
#define TYPE_STRING 1
#define TYPE_INT    2
#define TYPE_FLOAT  3

//types
struct fileBuffer
{
  char *buff;
  unsigned int size;
  unsigned int offset;
};
typedef struct fileBuffer fileBuffer;

struct code
{
  char *lineNum;
  char *mnemonic;
  char *opr1;
  char *opr2;
};
typedef struct code code;

struct symbolList
{
  char *dim1;
  char *dim2;
  char *value;
  struct symbolList *next;
};
typedef struct symbolList symbolList;

struct symbol
{
  char *name;
  char type;
  char valueType;
  char *dim1;
  char *dim2;
  char *value;
  symbolList *dimensionList;
};
typedef struct symbol symbol;

struct codeStack
{
  char *string;
  char doFree; //true or false flag to see if when we pop the stack, if it should be freed (not all strings all malloced)
};
typedef struct codeStack codeStack;

//global vars
int DEBUG = 0;
//function prototypes
fileBuffer makeBuffer(char *fileName);
fileBuffer readCode(fileBuffer fileBuff, code codes[], int *numCodes, int *codesLineStart);
//reads all symbols from the fileBuffer and stores them in codes. Leave the fileBuffer pointing after the symbol '@'
fileBuffer readSymbols(fileBuffer fileBuff, symbol symbols[], int *numSymbols);
int theEnd(code codes[],int line);
int getLabelJumpLine(char *label,symbol symbols[],int numSymbols);
int doLIT(code codes[], int *currentLine, codeStack* stack[], int *stack_ptr);
int doSTO(code codes[], int *codeLine, codeStack* stack[], int *stack_ptr,symbol symbols[], int numSymbols, int opr_position);
int discoverType( char* opr);
char*  doubleToString(double number, int type);

// leaves fileBuffer.offset at the next non-white
#define rmWhites(fb/*fileBUffer*/)         \
  do{                                      \
    while(fb.buff[fb.offset] == ' '  ||    \
          fb.buff[fb.offset] == '\t' ||    \
          fb.buff[fb.offset] == '\n' ){    \
      fb.offset++;                         \
    }                                      \
  }while(0)                                \

//leaves fileBuffer.offset at the start of the next white or comma
#define findNextWhiteOrComma(fb/*fileBuffer*/) \
  do{                                          \
    while(fb.buff[fb.offset] != ' '  &&        \
          fb.buff[fb.offset] != '\t' &&        \
          fb.buff[fb.offset] != '\n' &&        \
          fb.buff[fb.offset] != ','  ){        \
      fb.offset++;                             \
    }                                          \
  }while(0)                                    \

//leaves fileBuffer.offset at the start of next comma
#define findNextComma(fb/*fileBUffer*/) \
  do{                                   \
    while(fb.buff[fb.offset] != ','){   \
      fb.offset++;                      \
    }                                   \
  }while(0)                             \

#define findNextDoubleQuote(fb/*fileBUffer*/) \
  do{                                   \
    while(fb.buff[fb.offset] != '"'){   \
      fb.offset++;                      \
    }                                   \
  }while(0)                             \

void dumpStack( codeStack *stack[], int stack_ptr)
{
  for(int i=stack_ptr;i>=0;i--)
    printf(" |STACK[%i]=%s\n",i,stack[i]->string);
  printf("  -------\n");
}
void dumpSymbols(char *symbolName, symbol symbols[], int numSymbols)
{
  if(symbolName != NULL)
  {
    printf("print 'symbol' : not implemented yet!!. try just 'print' or 'p'\n");
    return;
  }
  else
  {
    int i=0,j=0;
    for(i;i<numSymbols;i++)
    {
      printf("%i) %s value:%s dim1:%s dim2:%s\n",i+1,symbols[i].name,symbols[i].value,symbols[i].dim1,symbols[i].dim2);
      if(symbols[i].dimensionList != NULL)
      {
        j=0;
        symbolList *dimensionList = symbols[i].dimensionList; 
        while(dimensionList != NULL)
        {
          printf(" %i.%i) %s value:%s dim1:%s dim2:%s\n",i+1,++j,symbols[i].name,dimensionList->value,dimensionList->dim1,dimensionList->dim2);
          dimensionList = dimensionList->next;
        }
      }
    }
  }
}

void push( codeStack *stack[], int *stack_ptr, char *string,char doFree)
{
  ++(*stack_ptr);

//NOTE!! THIS STACK GROWTH HAS NOT BEEN TESTED!
  if(*stack_ptr > STACK_SIZE)
  {
    STACK_SIZE+=STACK_INC;
    stack = realloc(stack,STACK_SIZE);
    if(stack == NULL)
    {
      printf("ERROR, out of mem. could not grow the stack to the size of %i",STACK_SIZE);
      exit(1);
    }
  }
 
  if(stack[(*stack_ptr)] == NULL)
  {
    stack[(*stack_ptr)] = malloc(sizeof(struct codeStack));  
    if(stack[(*stack_ptr)] == NULL)
    {
      printf("ERROR, out of mem. could not  allocate a stack frame");
      exit(1);
    } 
  }
  //if the struct was marked as needing to be freed, free it because
  //we are going to overwrite that ptr. 
  //NOTE! this means that you can pop() and use that pointer, but it is
  //dangerous to keep using it because subsecuent push'es will
  //corrupt/change the value! So if you want to keep a vlaue poped from
  //the stack forever, you will have allocate for it.
  else if(stack[(*stack_ptr)]->doFree)
    if(stack[(*stack_ptr)]->string != NULL)
      free(stack[(*stack_ptr)]->string);
 
  stack[(*stack_ptr)]->string = string;
  stack[(*stack_ptr)]->doFree = doFree;
//if DEBUG, print the stack after every push
if(DEBUG)
{
  printf("  __push__: %s\n",string);
  dumpStack(stack, *stack_ptr);
} 
  return;
}

char* pop( codeStack *stack[], int *stack_ptr)
{
  //we do not free the stack structure, because it will probably be used again.
  char *string = stack[(*stack_ptr)--]->string; 
//if DEBUG, print the stack after every pop
if(DEBUG)
{
  printf("  __pop__: %s\n",string);
  dumpStack(stack, *stack_ptr);
}
  return string; 
}

//could probably sort the symbol list, then binary search on it.
symbol* getSymbol(char *symbolName, symbol symbols[], int numSymbols)
{
  for(int i = 0; i< numSymbols; i++)
  {
      if(strcmp(symbolName,symbols[i].name) == 0)
      {
        return &symbols[i];
      }
  }
  printf("[ERROR] could not find symbol [%s] in the symbol table\n",symbolName);
  exit(1);
}
// these comments say what types can do the operation
//numbers and strings
    //these result ina numerical result
#define SUM        2
//these are only for numbers
#define SUB        3
#define MULT       4
#define DIV        5
#define MOD        6
#define POWER      7
#define NEG        8 //negate a number ( multiply by  -1, or do some boolean compliment )
   //these result in a boolean result)
#define LT         9 //Less Than
#define GT        10 //Greater Than
#define LTE       11 //Less Than or Equal
#define GTE       12 //Greater Than or Equal
//numbers and strings
#define DIF       13 //Different
#define EQU       14 //Equal
//just for strings
#define OR        15 // logical or
#define AND       16 // logical and
#define NOT       17 //logical not, this is for strings because we read a true or false (V,F)from stack, and invert it.



      //This function does all OPRs that operate on numbers. This means that the string pulled from the stack
      //(opr1 and opr2) will be converted to iether int or float type (depending if the string has a decimal or not)
      //and the opCode applied to the operators. As such, operators that contain non numerical values (strings)
      //should not use this, use doOPRNumericalOperation() instead, or the wrapper function doOPRStringOrNumericalOperation() instead
void doOPRNumerical(char *opr1, char *opr2, int opCode, codeStack *stack[], int *stack_ptr, int *codeLine)
{
  //so the deal here is, we know they are numerical because were
  //in here, but we dont know if its a float or int. So, we convert both to float types and do the operation,
  //but then if the result is a number (not true or false) and either op1 or opr2 was an int, we cast the result to an int if needed.
  //for exampl,e float+int = float, but int+int=int. (see all tpes in piton compiler, syntac.c)

  double opr1_d;
  double opr2_d;
    //we check for null, becasue some op codes only use 1 opernad ( like opCode 8, takes a number and makes it negative)
  if(opr1 != NULL)
    opr1_d = atof(opr1);
  if(opr2 != NULL)
    opr2_d = atof(opr2);
    //the types are needed as explained above
  int opr1_type;
  int opr2_type;

  double result_d;
  char *result_string;
  //first we do the requiered calculation specified in opCode
  switch(opCode)
  {
    case SUM: //2
      result_d = opr1_d + opr2_d;
      break;
    case SUB: //3
      result_d = opr1_d - opr2_d;
      break;
    case MULT: //4
      result_d = opr1_d * opr2_d;
      break;
    case DIV: //5
      result_d = opr1_d / opr2_d;
      break;
    case MOD: //6
      result_d = fmod(opr1_d,opr2_d);
      break;
    case POWER: //7
      result_d = pow(opr1_d,opr2_d);
      break;
    case NEG: //8
      result_d = opr2_d * -1; //could also do a compliment 1 to get the negative number, but gcc probably already puts that in?
      //boolean results
    case LT: //9
      result_string = (opr1_d < opr2_d ? "V" : "F");
      break;
    case GT: //10
      result_string = (opr1_d > opr2_d ? "V" : "F");
      break;
    case LTE: //11
      result_string = (opr1_d <= opr2_d ? "V" : "F");
      break;
    case GTE: //12
      result_string = (opr1_d >= opr2_d ? "V" : "F");
      break;
    case DIF: //13
      result_string = (opr1_d != opr2_d ? "V" : "F");
      break;
    case EQU: //14
      result_string = (opr1_d == opr2_d? "V" : "F");
      break;
    default:
      printf("ERROR, invalid opCode:%i in doOPRNumerical\n",opCode);
      exit(1);
  }
  //next, depending on the opCode and the REAL types of opr1 and opr2( that is,
  //we didnt take into account that they could be int, we did float operations)
  //we convert to the result if need be, then push it onto the stack.
  //NOTE, if the operation results in a boolean (result_string taking on V or F) just push it to stack.
  switch(opCode)
  {
    case SUM:
    case SUB:
    case MULT:
    case DIV:
    case MOD:
    case POWER:
      opr1_type = discoverType(opr1);
      opr2_type = discoverType(opr2);

      if( (opr1_type == TYPE_INT) && (opr2_type == TYPE_INT))
        result_string = doubleToString(result_d,TYPE_INT);
      else
        result_string = doubleToString(result_d,TYPE_FLOAT);
      push(stack,stack_ptr,result_string,1);
      break;
    case NEG:
      opr2_type = discoverType(opr2);
      if(opr2_type == TYPE_INT)
        result_string = doubleToString(result_d, TYPE_INT);
      else
        result_string = doubleToString(result_d, TYPE_FLOAT);
      push(stack,stack_ptr,result_string,1);
      break;
//exit(0);
      //boolean results
    case LT:
    case GT:
    case LTE:
    case GTE:
    case DIF:
    case EQU:
      //no need to do anything, just push to stack.    
      push(stack,stack_ptr,result_string,0);
      break;
  }
  //stack[++(*stack_ptr)] = result_string;
  (*codeLine)++;
}

//converts a double to string, if type = TYPE_INT, it truncates the double then to string
char*  doubleToString(double number, int type)
{
  #define RESULT_BUF_SIZE 100 // this will be the precision of the result.
  char result_buffer[RESULT_BUF_SIZE];
  char *result_string;

  if(type == TYPE_INT)
    snprintf(result_buffer,RESULT_BUF_SIZE,"%ld",(long)number);
  else
    snprintf(result_buffer,RESULT_BUF_SIZE,"%f",number);
      
  result_string = malloc(sizeof(char) * strlen(result_buffer) +1);
  if(result_string == NULL)
  {
    printf("Out of mem in numberToString");
    exit(0);
  }
  strcpy(result_string,result_buffer);
  return result_string; 
  #undef RESULT_BUF_SIZE
}

   //This functions does all string operations. If the operand  (1 or 2) is a number, it will be treated and
   //compared as if it were a string. it is safe ti call this func for  OR AND NOT since these are opcodes
   //used only by strings, but be wary when passing in a SUM DIF EQU. passing any other OPCODE is an error
void doOPRString(char *opr1, char *opr2, int opCode, codeStack *stack[], int *stack_ptr, int *codeLine)
{
   if(opr1 != NULL) // the NOT opr doesnt pop an opr1, so its null.
     if(*opr1 == '"')
       opr1++;  //remove the ". The terminating " was already removed in readCode
   if(opr2 != NULL) //just because.
     if(*opr2 == '"')
       opr2++;  //same
  //NOT is the only one that checks 1 operand, try that first
  if( opCode == NOT ) 
  {
    if((opr2 == NULL) || (*opr2 != 'V') && (*opr2 != 'F'))
    {
      printf("Error, expected to pop 'V' or 'F' for OPR 17, but got %s  line#%i\n",opr2,*codeLine);
      dumpStack(stack,*stack_ptr);
      exit(1);
    }
    char *negated_boolean;
    if(*opr2 == 'V')
      negated_boolean = "F";
    if(*opr2 == 'F')
      negated_boolean = "V";
    push(stack,stack_ptr,negated_boolean,0);
  }
  else if( (opCode  == AND) ||(opCode  == OR))
  {
    if(((opr2 == NULL) || ((*opr2 != 'V') && (*opr2 != 'F'))) ||
       ((opr1 == NULL) || ((*opr1 != 'V') && (*opr1 != 'F'))) )
    {
      printf("Error, expected to pop 'V' or 'F' for both operands int OPR %i, but got opr1:%s opr2:%s\n",opCode,opr1,opr2);
      dumpStack(stack,*stack_ptr);
      exit(1);
    }
    char *boolean_result;
    if((*opr1 == 'V') && (*opr2 == 'V'))
        boolean_result = "V";
    if((*opr1 == 'F') && (*opr2 == 'F'))
        boolean_result = "F";
    else if(((*opr1 == 'F') && (*opr2 == 'V')) ||
            ((*opr1 == 'V') && (*opr2 == 'F')))
      if(opCode == AND)
        boolean_result = "F";
      else
        boolean_result = "V";
    //stack[++(*stack_ptr)] = boolean_result;
    push(stack,stack_ptr,boolean_result,0);
  }
  else if( (opCode == DIF) ||(opCode == EQU)) 
  {
    if((opr1 == NULL) || (opr2 == NULL) )
    {
      printf("Error, opr1 or opr2 (pulled from stack)  is/are null for OPR %i in line#%i\n",opCode,*codeLine);
      dumpStack(stack,*stack_ptr);
      exit(1);
    }
    char *t="V", *f="F";
    if(opCode == DIF) //if checking for difference, invert true and false
    { t = "F"; f = "V"; }

    if(strcmp(opr1,opr2) == 0)
      //stack[++(*stack_ptr)] = t;
      push(stack,stack_ptr,t,0);
    else
      //stack[++(*stack_ptr)] = f;
      push(stack,stack_ptr,f,0);
  }
  else if( (opCode == SUM))
  {
    if((opr1 == NULL) || (opr2 == NULL))
    {
      printf("Error, opr1 or opr2 (pulled from stack)  is/are null for OPR %i in line#%i\n",opCode,*codeLine);
      dumpStack(stack,*stack_ptr);
      exit(1);
    }
    char *sum_buf = malloc(strlen(opr1) + strlen(opr2) +1 );
    if(sum_buf == NULL)
    {
      printf("ERROR, out of mem allocating mem for string concat");
      exit(1);
    }
    //there are more eficient ways to do this concat, but whateves.
    strcpy(sum_buf,opr1);
    strcat(sum_buf,opr2);
    //stack[++(*stack_ptr)] = sum_buf;
    push(stack,stack_ptr,sum_buf,1);
  }
  else
  {
    printf("ERROR, invalid opcode %i for string operation (uncompatable string operation)\n",opCode);
    exit(1);
  }
  (*codeLine)++;
  return;
}

int discoverType( char* opr)
{
  if(opr == NULL)
    return 0;

 //steps to determin what type (string, int, float) the operands are:
  //0. if it starts with a " (double quote), its a string 
  //1. check to see if the string has just a number
    //2. if it does, its an int 
  //3. if it doesnt, and just contains all numbers except one '.'
    //4. its a float
  //5.  else its a string, check to see that the opr to perform is string compatible


  int oprType = TYPE_INT;//default type
  int found_period = 0;
  int found_negative = 0;

  if(*opr == '"')  // the only way for it to be a string is if it beggings with a "
    return TYPE_STRING; 
  else 
    for(char *i=opr; *i != '\0'; i++)
    {
      if(isdigit(*i))
        continue;
      else if(*i == '-')
      {
        
        if(found_negative++) //if we find more than one negative symbol
        {  
          return TYPE_STRING;
          //printf("Error! found a non string/int/float type while trying to find the type for the string [%s]\n",opr);
          //exit(1);
        }
        else
          if(i - opr != 0) //if its the first negative symbol found, we need its index to see if its in a numerical position (ex: -34
	  {  
            return TYPE_STRING;
            //printf("Error! found a non string/int/float type while trying to find the type for the string [%s]\n",opr);
            //exit(1);
          }
      }
      else if(*i == '.')
      { 
        if(found_period++)//if we find more that one period, 
        {
          //printf("Error! found a non string/int/float type while trying to find the type for the string [%s]\n",opr);
          return TYPE_STRING;
          //exit(1);
        }
      }
      else
        return TYPE_STRING; //we found a symbol that isnt a number or a '.'
    }
  if((oprType == TYPE_INT) && (found_period == 1))
    oprType = TYPE_FLOAT;

  return oprType;
}

void doOPRStringOrNumerical(char *opr1, char *opr2, int opCode,  codeStack *stack[], int *stack_ptr, int *codeLine)
{
  // OR, AND, NOT are only performedon string operands
  if( (opCode == OR) || (opCode == AND) || (opCode == NOT) )
    return doOPRString(opr1, opr2, opCode, stack, stack_ptr, codeLine);
 
  //sum,dif and equ can also work for strings, so need to check for that
  //NOTE that this virtual machine expects the code read to be correct (as it is created by a compiler), 
  //so we directly go into a string sum if one of the types is string.
  if(( (opCode==SUM)  || (opCode==DIF) || (opCode==EQU)) && ( (discoverType(opr1)== TYPE_STRING) || (discoverType(opr2) == TYPE_STRING) ))
    return doOPRString(opr1,opr2,opCode,stack,stack_ptr,codeLine);
  
 return doOPRNumerical(opr1,opr2,opCode,stack,stack_ptr,codeLine); 
  
}

int doOPR(code codes[], int *codeLine, codeStack *stack[], int *stack_ptr, symbol symbols[], int numSymbols)
{
  char *opcode_string = (codes[*codeLine].opr2);
  int  opcode = atoi(opcode_string); //please forgive me lord, for using atoi
  char *opr1= NULL;
  char *opr2 = NULL;  
  char *result = NULL; 
//these opcodes dont pop anything from the stack 
if((opcode != 0) && (opcode != 1) && (opcode !=18) && (opcode !=19))
{
  //opr2 = stack[(*stack_ptr)--]; //check stackFree here
  opr2 = pop(stack,stack_ptr);
  //these opcodes only pop 1 stack element, so dont pop any more
  if((opcode != 8) && (opcode!= 17) && (opcode !=20) && (opcode!= 21))
    //opr1 = stack[(*stack_ptr)--]; //check stackFree here
    opr1 = pop(stack,stack_ptr);
}
if(DEBUG && (opcode != 0) && (opcode != 1) && (opcode !=18) && (opcode !=19))
  printf("OPR %i: opr1= %s, opr2= %s line#%i\n",opcode,opr1,opr2,*codeLine);

  switch(opcode)
  {
    case 0:
      //theEnd(codes,*codeLine); //this isnt necesaru, we know its the end if your in the case.
      return 1;
      break;
    case 1: //return from function
      ;
      int current_line = *codeLine; //for debug
      *codeLine = atoi(pop(stack,stack_ptr));
if(DEBUG)
  printf("OPR 1: Jumping from line#%i to %i\n",current_line,*codeLine);
      break;
    case 2:// + (sum).
      doOPRStringOrNumerical(opr1,opr2, opcode,stack, stack_ptr,codeLine);
      break;
    case 3:// - (subtract)
      //this could just call number directly, strings cant use -. Ill use StringOrNumerical incase strings can use - in the future
      doOPRStringOrNumerical(opr1,opr2, opcode,stack, stack_ptr,codeLine);
      break;
    case 4: // * (multiplication)
      //this could just call number directly, strings cant use *. Ill use StringOrNumerical incase strings can use * in the future
      doOPRStringOrNumerical(opr1,opr2, opcode,stack, stack_ptr,codeLine);
      break;
    case 5:// / (division)
      doOPRNumerical(opr1,opr2, opcode,stack, stack_ptr,codeLine);
      break;
    case 6: // % (modulo)
      doOPRNumerical(opr1,opr2, opcode,stack, stack_ptr,codeLine);
      break;
    case 7: //exponent
      doOPRNumerical(opr1,opr2, opcode,stack, stack_ptr,codeLine);
      break;
    case 8: // - (make number negative)
      doOPRNumerical(opr1,opr2, opcode,stack, stack_ptr,codeLine);
      break;
    case 9: // <  (less than)
      //this could just call number directly, strings cant use <. Ill use StringOrNumerical incase strings can use < in the future
      doOPRStringOrNumerical(opr1,opr2, opcode,stack, stack_ptr,codeLine);
      break;
    case 10:// > (greater than)
      doOPRStringOrNumerical(opr1,opr2, opcode,stack, stack_ptr,codeLine);
      break;
    case LTE:// 11, <=
      doOPRStringOrNumerical(opr1,opr2, opcode,stack, stack_ptr,codeLine);
      break;
    case 12://12, >=
      doOPRStringOrNumerical(opr1,opr2, opcode,stack, stack_ptr,codeLine);
      break;
    case 13: /* != */
      doOPRStringOrNumerical(opr1, opr2, opcode, stack, stack_ptr, codeLine);
      break;
    case 14: /* == */
      doOPRStringOrNumerical(opr1, opr2, opcode, stack, stack_ptr, codeLine);
      break;
    case 15: /* or */
      doOPRString(opr1, opr2, opcode, stack, stack_ptr, codeLine);
      break;
    case 16: /* and */
      doOPRString(opr1, opr2, opcode, stack, stack_ptr, codeLine);
      break;
    case 17: /* ! */
      doOPRString(opr1, opr2, opcode, stack, stack_ptr, codeLine);
      break;
    case 18:
if(DEBUG)
  printf("OPR 18: Clearing the screen\n");
      for(int i=0;i<80;i++)
        printf("\n");
      break;
    case 19:
      ; // to remove error "a label can only be part of a statement and a declaration is not a statement"
      char buffer[INPUT_BUFFER_SIZE];
if(DEBUG)
  printf("<<< ");
      fgets(buffer,INPUT_BUFFER_SIZE,stdin);
      char *buffer_mallocd = malloc(sizeof(char) * strlen(buffer)+1);
      if(buffer_mallocd == NULL)
      {
        printf("ERROR, out of mem allocating mem for opr 19");
        exit(1);
      }
      strcpy(buffer_mallocd,buffer);
      buffer_mallocd[strlen(buffer_mallocd)-1] = '\0';//overwrite the \n since fgets saves it
  //we set the input on the stack and call doSTO since that code already does what we want here
  //but it pulls the value from the stack, so lets just reuse that code doing this (simple) change
  // also note that the 1 value being sent to doSTO tells it to pull the var from opr1 since
  // normal STO pulls it from opr2. 

int deb = DEBUG;//so that if in debug, we dont print this stack input in debug mode!
DEBUG = 0;
      push(stack,stack_ptr,buffer_mallocd,1);
DEBUG = deb;
//      if(DEBUG)
//      {
//        symbol *sym = getSymbol(codes[*codeLine].opr1,symbols,numSymbols);
//        printf("OPR 19: %s = %s   line# %i\n",sym->name,buffer_mallocd,*codeLine);
//      }
      doSTO(codes, codeLine, stack,  stack_ptr, symbols, numSymbols,1);
      break;
    case 20:
      if(*opr2 == '"') //if it starts with ", its a string so remove the "( we removed closing " in readCode
        opr2++;
if(DEBUG)
  printf(">>> ");
      printf("%s",opr2);
      (*codeLine)++;
      break;
    case 21:
      if(*opr2 == '"') //if it starts with ", its a string so remove the "( we removed closing " in readCode
        opr2++;
if(DEBUG)
  printf(">>> ");
      printf("%s\n",opr2);
      (*codeLine)++;
      break;
  }
if(DEBUG)
  printf("\n**********\n");
  return 0;
}

int doCAL(code codes[], int *codeLine,symbol symbols[], int numSymbols)
{
  symbol *sym = getSymbol(codes[*codeLine].opr1,symbols,numSymbols);
if(DEBUG)
  printf("CAL: %s,  jumping from  line#%i to %s\n**********\n",sym->name,*codeLine,sym->dim1);

  *codeLine = atoi(sym->dim1);//getLabelJumpLine(sym->name,symbols,numSymbols); //not necessary, it would look for symbol in symtab again 
  return 1;
}

int doJMC(code codes[], int *codeLine, int codeLineStart, codeStack *stack[], int *stack_ptr, symbol symbols[], int numSymbols)
{
  char condition = *(pop(stack,stack_ptr));
  char jump_condition = *(codes[*codeLine].opr1);
if(DEBUG)
  printf("JMC  Jump if: %c, received: %c  line#%i \n",jump_condition,condition,*codeLine);

  if( (condition != 'V') && (condition != 'F') )
  {
    printf("[ERROR], expected to pop 'V' or 'F' from stack, but got [%c], line#%i",condition,*codeLine);
    exit(1);
  } 
  if(condition != jump_condition)
  {
    (*codeLine)++;
  }  
  else if(condition == jump_condition)
  {
    symbol *sym = getSymbol(codes[*codeLine].opr2,symbols,numSymbols);
    *codeLine =  atoi(sym->dim1) /*+ codeLineStart*/;
  }
  else
  {
    printf("[ERROR], could not compare poped from stack vs from code: [%c]vs[%c], code #%i",condition,jump_condition,*codeLine);
    exit(1); 
  }
if(DEBUG)
  printf("     jumping to %i\n**********\n",*codeLine);

  return 1;
}

int doJMP(code codes[], int *codeLine, int codeLineStart, codeStack *stack[], int *stack_ptr, symbol symbols[], int numSymbols)
{
  symbol *sym;
  char *jumpLine = NULL;
if(DEBUG)
  printf("JMP jumping from line#%i",*codeLine);

  //snce you can jump to a label, or a line number directly, we need to see what were dealing with
  if(*(codes[*codeLine].opr2) != '_') //all labels being with '_', if it doesnt, its an explicit number jump
  {
    jumpLine = codes[*codeLine].opr2;
    *codeLine =  atoi(jumpLine);
  }
  else
  {
    sym  = getSymbol(codes[*codeLine].opr2,symbols,numSymbols);
    jumpLine = sym->dim1; //dim1 holds the jump line
    //since the code can begin on any line number, we add the starting offset in
    *codeLine = atoi(jumpLine) /*+ codeLineStart*/;
  }
if(DEBUG)
  printf("  to %i\n**********\n",*codeLine);
}

int doSTO(code codes[], int *codeLine, codeStack * stack[], int *stack_ptr,symbol symbols[], int numSymbols, int opr_position)
{
  symbol *sym;
  char *d1 = NULL;//dimensions (if the var has any)
  char *d2 = NULL;
  char *value = NULL;
  if(opr_position == 2) //for normal STO
    sym = getSymbol(codes[*codeLine].opr2,symbols,numSymbols);
  else if(opr_position == 1) //for doing storage with OPR 19
    sym = getSymbol(codes[*codeLine].opr1,symbols,numSymbols);
  else
  {
    printf("DEBUG: doSTO's parameter 'opr_positon' can only be 1 or 2, recieved: %i\n",opr_position);
    exit(1);
  }

  if((sym->type != 'F') && (*(sym->dim1) != '0')) //if the variables is not a function (the return value) and is  dimensioned 
  {
    symbolList *symListStart = sym->dimensionList;
    symbolList *symListEnd = sym->dimensionList;
    value = pop(stack,stack_ptr);
    d1 = pop(stack,stack_ptr);

    if(*(sym->dim2) != '0')
    {
      //inverse the order or the dimensiones pulled from the stack. if there
      //are 2 dimens, we pulled j first, so make that dimen2
      char *d_temp = d1;
      d1 = pop(stack,stack_ptr);
      d2 = d_temp;
    }

    //traverse the variable dimensions list, if the dimension is already there then just overwrite the number (diemnsion value), if not
    //then create a new entry and add the number
    while(symListEnd != NULL)
    {
      sym->dimensionList = symListEnd;
      //we found an entry, just update its value

//OK, ill admit this shit is a clusterfuck, bIll fix it later... this same "logic" is seen in doLOD
      if((strcmp(sym->dimensionList->dim1, d1) == 0)) //if the first dimension matches
      {
         if(sym->dimensionList->dim2 ==  NULL) //if ther is no second dimension, were gold
         {
           free(sym->dimensionList->value);
           sym->dimensionList->value = malloc(strlen(value)+1);
           if(sym->dimensionList->value == NULL)
           {
             printf("ERROR, out of mem allocating mem for the dimension of a variable. %s[%s]",sym->name,sym->dimensionList->dim1);
             exit(1);
           }
           strcpy(sym->dimensionList->value,value);
           sym->dimensionList = symListStart;
           //quick fix, since we found and modified the vaue, just exit here. 
            (*codeLine)++;
           return 1;
         }
         else if( (strcmp(sym->dimensionList->dim2 ,d2) == 0))//if there is a second dimension, check to see if it matches
         {
           free(sym->dimensionList->value);
           sym->dimensionList->value = malloc(strlen(value)+1);
           if(sym->dimensionList->value == NULL)
           {
             printf("ERROR, out of mem allocating mem for the dimension of a variable. %s[%s][%s]",sym->name,sym->dimensionList->dim1,sym->dimensionList->dim2);
             exit(1);
           }
           strcpy(sym->dimensionList->value,value);
           sym->dimensionList = symListStart;
           //quick fix, since we found and modified the vaue, just exit here. 
            (*codeLine)++;
           return 1;
         }       
      }
      symListEnd = symListEnd->next;
      
    }
    //we didnt find an entry, so create one at the head or end of list
    if(sym->dimensionList == NULL) //create head
    {
      sym->dimensionList = malloc(sizeof(symbolList));
      if(sym->dimensionList == NULL)
      {
        printf("ERROR, out of mem allocating mem for the dimension of a variable.  %s[%s][%s]",sym->name,sym->dimensionList->dim1,sym->dimensionList->dim2);
        exit(1);
      }
      symListStart = sym->dimensionList;
    }
    else //create an entry at the end of the list
    {
      sym->dimensionList->next = /*(symbolList*)*/malloc(sizeof(symbolList));
      if(sym->dimensionList->next == NULL)
      {
        printf("ERROR, out of mem allocating mem for sym list");
        exit(1);
      }
      sym->dimensionList = sym->dimensionList->next;
    }
     
//could do a warning here if the dimension specified exceed the dimension initialized

//NOTE! see buglist 1.
      sym->dimensionList->dim1 = d1;
      sym->dimensionList->dim2 = d2;
      sym->dimensionList->value = malloc(strlen(value+1));
      if(sym->dimensionList->value == NULL)
      {
        printf("ERROR, out of mem allocating mem for symbol value");
        exit(1);
      }
      strcpy(sym->dimensionList->value,value);
      sym->dimensionList->next = NULL;
        //we make the list point back to begining since we dont know if we added to the head, or to the end.
      sym->dimensionList = symListStart;
  }
  //the variable isnt dimensioned, the  value is right there.
  else
  {
    value = pop(stack,stack_ptr);
//NOTE! see buglist 1 on why we cant free here just yet.
    //if(sym->value != NULL)
     // free(sym->value);
    sym->value = malloc(strlen(value)+1);
    if(sym->value == NULL)
    {
      printf("ERROR, out of mem allocating mem for symbol value");
      exit(1);
    }
    strcpy(sym->value,value);
  }
if(DEBUG)
  if(opr_position ==2) //pos 2 is when we call this from a STO mnemonic
    printf("STO  %s = %s  line#%i\n**********\n",sym->name,sym->value,*codeLine);
  else //we calle din from opr 19
    printf("OPR 19: %s = %s   line# %i\n**********\n",sym->name,sym->value,*codeLine);

  (*codeLine)++;
  return 1;
}

int doLOD(code codes[], int *codeLine, codeStack * stack[], int *stack_ptr, symbol symbols[], int numSymbols)
{
   symbol *sym;
   char *value = NULL;
   char *d1 = NULL;
   char *d2 = NULL; //dimensions (if the variable has any)
   sym = getSymbol(codes[*codeLine].opr1,symbols,numSymbols); 
if(DEBUG)
  printf("LOD name:%s value:%s  dim1:%s dim2=%s line#%i\n",sym->name,sym->value,sym->dim1,sym->dim2,*codeLine);

   //check to see if the symbol is a label. labels always begin with _E
   if((sym->name)[0] == '_' && (sym->name)[1] == 'E')
   {
    value = sym->dim1;//the line to jump to is held in dim1
   }
   else if( (sym->type != 'F') && *(sym->dim1) != '0') //if the variables is not a function (the return value) and is dimensioned
   {
     symbolList *symArrTmp = sym->dimensionList;
     //d1 = stack[(*stack_ptr)--];
     d1 = pop(stack,stack_ptr);
//check stack free here
     if(*(sym->dim2) != '0')
     {
       //inverse the order or the dimensiones pulled from the stack. if there
       //are 2 dimens, we pulled j first, so make that dimen2
       char *d_temp = d1;
       //d1 = stack[(*stack_ptr)--];
       d1 = pop(stack,stack_ptr);
//check stack free here
       d2 = d_temp;
     }
     while(symArrTmp != NULL)
     {
//printf("  at dim1:%s - looking for dim1:%s\n",symArrTmp->dim1,d1);
//printf("    at dim2:%s - looking for dim2:%s\n",symArrTmp->dim2,d2);
       if(strcmp(symArrTmp->dim1, d1) == 0)
         if(symArrTmp->dim2 ==  NULL) 
           {
             value = symArrTmp->value;
             break;
           }
         else if( (strcmp(symArrTmp->dim2,d2) == 0))
           {
             value = symArrTmp->value;
             break;
           }       
       symArrTmp = symArrTmp->next;
     }
     if(value == NULL)
     {
       dumpSymbols(NULL,symbols,numSymbols);
       printf("[ERROR] could not find the value for the symbol [%s] with dimensions [%s][%s] in the symbol table, line#%i\n",sym->name,d1,d2,*codeLine);
       exit(0);
     }
   }
   //else the symbol is not dimensioned, not a label or a func.  The value is right there.
   else
     value = sym->value;

   push(stack,stack_ptr,value,0); //dont free this value later, it may be used by another LOD or something.
                                  //it will be freed when the symbol table is freed at the end of the program
   (*codeLine)++;
if(DEBUG)
  printf("**********\n");
   return 1;
}

int doLIT(code codes[], int *codeLine, codeStack *stack[], int *stack_ptr)
{
if(DEBUG)
  printf("LIT: %s  line#%i\n",codes[*codeLine].opr1,*codeLine);

  push(stack,stack_ptr,codes[*codeLine].opr1,0);

if(DEBUG)
  printf("**********\n");
  (*codeLine)++;
  return 1;
}

int theEnd(code codes[],int line)
{
  if(strcmp(codes[line].mnemonic, "OPR") &&
     strcmp(codes[line].opr1, "0") &&
     strcmp(codes[line].opr2, "0"))
  {
    exit(0);
    //return 1;
  }
  return 0;
}

//returns the line the label jumps to
int getLabelJumpLine(char *label,symbol symbols[],int numSymbols)
{
  int i;
  for(int i = 0; i< numSymbols; i++)
  {
      if(strcmp(label,symbols[i].name) == 0)
        return atoi(symbols[i].dim1); //dim1 holds the line num to jump to in the PL0 code
  }
  printf("[ERROR] could not find label [%s] in the symbol table\n",label);
  exit(1);
}

int run(code codes[],int numCodes,int codeLineStart,
        symbol symbols[], int numSymbols)
{
  //NOTE* we accloate an array of pointers  to stack structures,
  //but we dont allocate for the structures. This is because when we do a push(),
  //we allocate the stack structure there. This way we dont waste a bunch of mem
  //if the program is small and will uses limited stack space.
  //Also, this is handy because we can easily make the stack bigger by
  //making this array of stack ptr's bigger (with realloc)
  codeStack *stack[STACK_SIZE]; //the PL0 stack machine
  int stack_ptr = -1;
  //first we find main (which is label _P)
  // codeLine = the current line we are processing
  int codeLine = getLabelJumpLine("_P",symbols,numSymbols);
  int theEnd = 0;
  //debug vars
  int breakPoint=-1; //if set, will stop at that line number
  char _continue=1; //if set to continue, just continuw with program execution, dont ask for debug anymore
  char *debug_console_help = "try (commands can also be abbreviated to first char):\n  help: display this message.\n  next: continue execution to the next line. You can also just hit enter for this command.\n  continue: continue execution. dont ask for debug command anymore (unless a break point is hit)\n  print: print all symbols\n  break #: make a break point at that line #. to unset, use: break -1 or similar\n  list: prints 10 lines above and below the current code line. -> points to the next command to be executed\n  dump: dumps the state of the stack as it is currently\n";
if(DEBUG)
  printf(">>debug console.\n%s",debug_console_help);
        //not using function theEnd() because it does many unecesary comparisons!
  while(/*!theEnd(codes,currentLine)*/!theEnd)//once opr case sees end, it will end it.
  {
//debug console!
if(DEBUG)
{
  if(_continue || (breakPoint == codeLine))
  {
    if(breakPoint == codeLine)
      _continue=1;
    char debug_buf[30];
    char *tok;
    printf("debug-console>");
    fgets(debug_buf,30,stdin);
    debug_buf[strlen(debug_buf) -1] = '\0'; //overwrite the \n since fgets saves it
    tok = strtok(debug_buf," ");
    while(1)
    {
      if((tok == NULL) || (strcmp(tok,"n")==0) ||  (strcmp(tok,"next")==0) ) //just hiting enter 
      {
        break;
      }
      else if( (strcmp(tok,"help")==0) || (strcmp(tok,"h")) == 0)
      {
        printf("\n%s",debug_console_help);
      }
      else if( (strcmp(tok,"c") == 0) || (strcmp(tok,"continue") == 0))
      {
        _continue = 0;
        break;
      }
      else if( (strcmp(tok,"print")==0) || (strcmp(tok,"p")) == 0)
      {
        if(((tok = strtok(NULL," ")) == NULL) || (strcmp(tok,"all")==0))
        {   
          dumpSymbols(NULL,symbols,numSymbols);
        }
        else
          dumpSymbols(tok,symbols,numSymbols);
      }
      else if( (strcmp(tok,"set")==0) || (strcmp(tok,"s")) == 0)
      {
        printf("set command not yet implemented!\n");
      }
      else if( (strcmp(tok,"break")==0) || (strcmp(tok,"b")) == 0)
      {
        if((tok = strtok(NULL," ")) == NULL)
        {   
          printf("Please provide a break point number for the break command. example: break 200\n");
        }
        else
        { 
          if(discoverType(tok) == TYPE_INT)
            breakPoint = atoi(tok);
          else
            printf("Please provide a numerical argument for the break command. example: break 200\n");
        }
      }
      else if( (strcmp(tok,"list")==0) || (strcmp(tok,"l")) == 0)
      {
        int i,j;
                             //instead of 1, this is where codeLine comes in handy!
        for( i = (codeLine-10)<codeLineStart?codeLineStart:codeLine-10;i<codeLine;i++)
          printf("   %i %s %s, %s\n",i,codes[i].mnemonic,codes[i].opr1,codes[i].opr2);
        printf("-> %i %s %s, %s\n",i,codes[codeLine].mnemonic,codes[codeLine].opr1,codes[codeLine].opr2);
        for(i=codeLine+1, j = (codeLine+11)>numCodes?numCodes:codeLine+11;i<j;i++)
          printf("   %i %s %s, %s\n",i,codes[i].mnemonic,codes[i].opr1,codes[i].opr2);
      }
      else if( (strcmp(tok,"dump")==0) || (strcmp(tok,"d")) == 0)
      {
        dumpStack(stack,stack_ptr);
      }
      else if( (strcmp(tok,"developer")==0) || (strcmp(tok,"dev")) == 0)
      {
        printf("Developed by Bryan Martin Tostado\n");
      }
      else
        printf("Unknown command: '%s'\n.%s",tok,debug_console_help);

      if(_continue)
      {
        printf("debug-console>");
        fgets(debug_buf,30,stdin);
        debug_buf[strlen(debug_buf) -1] = '\0'; //overwrite the \n since fgets saves it
        tok = strtok(debug_buf," ");
      }    
    } 
  }
}
    if(strcmp("LIT",codes[codeLine].mnemonic) == 0){
      doLIT(codes,&codeLine,stack,&stack_ptr);}
    else if(strcmp("LOD",codes[codeLine].mnemonic) == 0){
      doLOD(codes,&codeLine,stack,&stack_ptr,symbols,numSymbols);}
    else if(strcmp("STO",codes[codeLine].mnemonic) == 0){
      doSTO(codes,&codeLine,stack,&stack_ptr,symbols,numSymbols,2);}
    else if(strcmp("JMP",codes[codeLine].mnemonic) == 0){
        doJMP(codes,&codeLine,codeLineStart,stack,&stack_ptr, symbols,numSymbols);}
    else if(strcmp("JMC",codes[codeLine].mnemonic) == 0){
      doJMC(codes,&codeLine,codeLineStart,stack,&stack_ptr,symbols,numSymbols);}
    else if(strcmp("CAL",codes[codeLine].mnemonic) == 0){
      doCAL(codes,&codeLine,symbols,numSymbols);}
    else if(strcmp("OPR",codes[codeLine].mnemonic) == 0){
      theEnd = doOPR(codes, &codeLine, stack, &stack_ptr, symbols,numSymbols);}
  }
  //free codestack here. We dont knwo how many codeStack structs were actually allocated,
  //so keep freeing till we find one NULL
  int i=0;
  for(i; ;i++)
  {
    if(stack[i] == NULL)
      break;
    else
      free(stack[i]);
  }
}
//the code lines are read from the buffer. The codes structure's pointers point INTO the buffer!, thus, the buffer IS
//modified. Since there is always a space or comma after a symbol, these are replaced for '\0' so its a valid string.
fileBuffer readCode(fileBuffer fileBuff, code codes[], int *numCodes, int *codeLineStart)
{

  char *tmpCode[4]; //4 because that the #elems in struct code
  int i = 0;
  int firstCode = 1; //flag to see if were parsing the first code line. (we use this to copy out the line start #)
  *numCodes = 0;
  //codes start after '@', so make sure were after that symbol
  //this is only needed if the call to readCode() is before the call to
  //readSymbols, so this soesnt need to be implemented..for now

  //while(fileBuff.buff[fileBuff.offset] != EOF) //this causes sigsegv on files saved from vi (maybe because of fillbuff patch)
  while(fileBuff.offset+2 < fileBuff.size)//+2 for the makeBuff patch
  {
    for(i = 0 ; i<4;i++) //4 because thats the # of tokens to read per line
    {
      rmWhites(fileBuff);
      if(fileBuff.buff[fileBuff.offset] != '"') //special case for strings (they can have spaces in them)
      {
        tmpCode[i] = &(fileBuff.buff[fileBuff.offset]);
        findNextWhiteOrComma(fileBuff);
//this is where the patch described in fillBUff would go! we would check for EOF here and quit and other bs.
        fileBuff.buff[fileBuff.offset++] = '\0';
      }
      else //its a string, since it can have spaces inside it needs special treatment
      {
        tmpCode[i] = &(fileBuff.buff[fileBuff.offset++]); //save the opening " so we know its a string
        findNextDoubleQuote(fileBuff);
        fileBuff.buff[fileBuff.offset++] = '\0'; //safe to discard the closing double quote
        findNextComma(fileBuff); //we need this because we have only seen upto the last "
        fileBuff.offset++; //consume the ','
      }
      
    }
    if(firstCode)
    {
      *codeLineStart = *numCodes = atoi(tmpCode[0]);
      firstCode = 0;
    }
    codes[*numCodes].lineNum = tmpCode[0];
    codes[*numCodes].mnemonic = tmpCode[1];
    codes[*numCodes].opr1 = tmpCode[2];
    codes[*numCodes].opr2 = tmpCode[3];
    (*numCodes)++;
    //rmWhites(fileBuff);
  } 
   
  return fileBuff;
}

//the symbols are read from the buffer. The symbols structure's pointers point INTO the buffer!, thus, the buffer IS
//modified. Since there is always a comma after a symbol, these are replaced for '\0' so its a valid string.
fileBuffer readSymbols(fileBuffer fileBuff, symbol symbols[], int *numSymbols)
{
  fileBuff.offset = 0; //symbols are at the start of the file, just in case
  char *tmpSymbol[5];
  int i = 0;
  while(fileBuff.buff[fileBuff.offset] != '@')// @ marks the end of symbols and start of code
  {
    (*numSymbols)++;
    for(i = 0; i< 5; i++)//read a symbol struct 
    {
      rmWhites(fileBuff);
      tmpSymbol[i] = &(fileBuff.buff[fileBuff.offset]);
      findNextWhiteOrComma(fileBuff);
      if(fileBuff.buff[fileBuff.offset] == ',')
      {
        fileBuff.buff[fileBuff.offset++] = '\0';
      }
      else//get the last symbol.we still need to find the  ',' after adding the '\0' (becasue of stupid '#' symbol)
      {
        fileBuff.buff[fileBuff.offset++] = '\0';
        findNextComma(fileBuff);
        fileBuff.offset++; //consume the ','
      } 
    }
    //this is to get rid of the unecessary  "#," delimeter
    findNextComma(fileBuff);
    fileBuff.offset++;
    rmWhites(fileBuff);

    symbols[*numSymbols-1].name = tmpSymbol[0];
    symbols[*numSymbols-1].type = *(tmpSymbol[1]); //only need the char
    symbols[*numSymbols-1].valueType = *(tmpSymbol[2]);  //same
    symbols[*numSymbols-1].dim1 = tmpSymbol[3];
    symbols[*numSymbols-1].dim2 = tmpSymbol[4];
  }
  fileBuff.offset++; //consume the '@'
  rmWhites(fileBuff); //leave the offset at start of code

  return fileBuff;  
}

fileBuffer makeBuffer(char *fileName)
{
  fileBuffer buffer;
  FILE *file;
  long size;

  if((file = fopen(fileName,"r")) == NULL)
  {
    perror("error opening file");
    exit(1);
  }
  if(fseek(file, 0L, SEEK_END) != 0)
  {
    fprintf(stderr,"error seeking end of file");
    exit(1);
  }
  size = ftell(file) + 2; // +2 for " \0" the space is so we dont need to patch how the code is read!
                          //  the same loop will read the whole file (since it looks for spaces as delimeters)
  if((buffer.buff = (char*)malloc(sizeof(char) * size)) == NULL)
  {
    fprintf(stderr,"asigning buffer:");
    exit(1);
  }
  buffer.size = size;
  if(fseek(file, 0L, SEEK_SET) != 0)
  {
    fprintf(stderr,"error seeking begining of file:");
    exit(1);
  }
  if(fread(buffer.buff,sizeof(char),buffer.size-1,file) <= 0)
  {
    fprintf(stderr,"Error: file [%s] is empty\n",fileName);
    exit(1);
  }
  buffer.buff[size] = ' ';//this is so that we dont need to patch readCode.
  buffer.buff[size+1] = EOF;
  buffer.buff[size+2] = '\0';
  buffer.offset = 0;
//  fclose(file); // has casued sigsegv for unknown reasons..

  return buffer;
} 

int main( int argc, char * argv[])
{
  int fileIndex = 1;
  fileBuffer fileBuff;
//TODO make the symbols and code arrays, arrays of ptrs. This way
//it is easly relocatable to a bigger size and there is less mem
//wasted if not all of the array elemes are used (as is the case with the static alocation now).
//See the run(...) function, the stack has this mechanism.
//OR. see how many symbols and code lines are in the file, and static allocate fir this amount.
//this is probably a better idea since this number wont change after the initial alloc (unlike the stack)
   symbol symbols[1000]; //temporary magic number!
   int numSymbols = 0; 
  code codes [5000]; //temporary magic number!
  int numCodes = 0;
  int codeLineStart = 0; //this is needed because the code can begin with any line number...

  if(argc < 2)
  {
    printf("debes indicar el archivo a compilar (nombre completo)\n");
    return 1;
  }
//shitty getopt for now, since windows doesnt have getopt
  if(strcmp(argv[1],"d") == 0 || strcmp(argv[1],"-d") == 0)
  {
    DEBUG = 1;
    fileIndex = 2;
  }
  fileBuff = makeBuffer(argv[fileIndex]);
//  printf(fileBuff.buff);
  fileBuff = readSymbols(fileBuff,symbols,&numSymbols);
/*  for(int i = 0;i<numSymbols;i++)
  {
    printf("%s,%c,%c,%s,%s\n",symbols[i].name,symbols[i].type,symbols[i].valueType,symbols[i].dim1,symbols[i].dim2);
  }*/
  fileBuff = readCode(fileBuff,codes,&numCodes,&codeLineStart);
  /*for(int i = codeLineStart; i< numCodes; i++)
  {
    printf("%s %s %s, %s\n",codes[i].lineNum,codes[i].mnemonic,codes[i].opr1,codes[i].opr2);
  }*/
  run(codes,numCodes,codeLineStart,symbols,numSymbols); 
  free(fileBuff.buff); //cant free before since everything points into the bufer
  return 0;
}
