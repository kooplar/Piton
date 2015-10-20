/*
 * Bryan Martin Tostado
 * created 13 april 2014
 *
 * Descripticion:
 *  syntax! 
 *
 *
 * Notes:
 *
 */

#include <stdio.h>


#include <string.h>
#include <stdlib.h>
#include "./include/lexer.h"
#include "./include/errors.h"
#include "./include/symtab.h"
#include "./include/codegen.h"
#include "./include/codegen.h"
int from_regresa = 0;
/*global vars*/
static token_t token;
//PL0
static char *param_name = NULL;
static char *param_name_stack[50];
static int   param_type_stack[50];
static int   param_stack_ptr = 0;
static int   called_by_param = 0;
static char *fullVarName(char*);
static char *fullVarNameAfterSearch(char*);
static char *label_loop_start; //
static char *label_loop_end;   // These two haveto be global since calls to interrumpe/continua will need to see them!
static char *label_continua= "";
static int   loop_incrementing = 1;
static int   loop_pasa = 1;
static int   continua_called = 0;
static int   declared_array = 0;
static int   only_function_call = 0; // used for function call. if it is called from a block (not an expression), then 
                                  //even though it has a return type, dont push it onto the stack. just call the func.
#ifdef PL0_MAIN_FIX
  static char *label_pl0_main_fix;
#endif
//\Pl0

static int dim1 = 0;
static int dim2 = 0;
static int type = 0; //used in cte(), INT,FLOAT,STRING,LOG_CONS to know what the value should be represented as
static char *value = NULL; //same as lexeme, used in cte() to save the number or w-e read
static int function_returns = 0;
static int func_return_type = 0;
static int function_returnd = 0;
static int in_loop = 0;
static char *in_func_name = ""; //leave as "" because globas vars will concat this
static int var_type = NO_TYPE;
static int main_defined = 0;
/*static funcs*/
static int vars();
static int cte();
static int decArreglo(); // declara arreglo
static int funcs();
static int param(char *funcBuffer, int *funcBufferCounter);
static int bloque();
//expresions
static void expr();
static void opY();
static void opNo();
static void opRel();
static void opSum();
static void opMult();
static void opExp();
static void opSig();
static void opTerm();
//
static int Lfunc(char*);
static int asigna(char*);
static int dimens();
static int lee();
static int imprime();
static int si();
static int mientras();
static int regresa();
static int interrumpe();
static int continua();
static int desde();
static int Lexp(/*PL0*/ char* /*PL0*/);
//semantic verificatio on expresions
 //this is the real call, verifyTypes doesnt work with unary operations so this '2'versions is needed
static int verifyTypes2(int,int, int,long,long);
static int verifyTypes(int,long,long);

//first dimension is operation to check, second type to check, third is type to return
#define NUM_TYPES 60
static int expr_types[NUM_TYPES][3] = {
/*ASSIGNMENTS*/
/*00*/ { ASS, INT+ASS+INT, 1 },//return type  doesnt matter here unless epxressions can have asignemts in them. implement later
/*01*/ { ASS, STRING+ASS+STRING, 1 },
/*02*/ { ASS, FLOAT+ASS+FLOAT, 1 },
/*03*/ { ASS, LOG_CONS+ASS+LOG_CONS, 1 },	 
/*04*/ { ASS, FLOAT+ASS+INT, 1 },
// {ASS, INT+ASS+FLOAT, 1 }, /*NOT ALLOWED*/
/*ADDITION*/
/*05*/ { PLUS, INT+PLUS+INT, INT },
/*06*/ { PLUS, INT+PLUS+FLOAT, FLOAT },
/*07*/ { PLUS, FLOAT+PLUS+INT, FLOAT  },
/*08*/ { PLUS, FLOAT+PLUS+FLOAT, FLOAT  },
/*09*/ { PLUS, STRING+PLUS+STRING, STRING  },
/*SUBTRACTION*/
/*10*/ { MINUS, INT+MINUS+INT, INT  },
/*11*/ { MINUS, INT+MINUS+FLOAT, FLOAT  },
/*12*/ { MINUS, FLOAT+MINUS+FLOAT, FLOAT  },
/*13*/ { MINUS, FLOAT+MINUS+INT, FLOAT  },
/*MULTIPLICATION*/
/*14*/ { MULT, INT+MULT+INT, INT  },
/*15*/ { MULT, INT+MULT+FLOAT, FLOAT },
/*16*/ { MULT, FLOAT+MULT+FLOAT, FLOAT  },
/*17*/ { MULT, FLOAT+MULT+INT, FLOAT  },
/*DIVISION*/
/*18*/ { DIV, INT+DIV+INT, INT },
/*19*/ { DIV, INT+DIV+FLOAT, FLOAT  },
/*20*/ { DIV, FLOAT+DIV+FLOAT, FLOAT  },
/*21*/ { DIV, FLOAT+DIV+INT, FLOAT  },
/*FLOOR DIVISION*/ //floor division just divides then truncates the decimal
/*22*/ { FDIV, INT+FDIV+INT, INT  },
/*23*/ { FDIV, INT+FDIV+FLOAT, INT  },
/*24*/ { FDIV, FLOAT+FDIV+FLOAT, INT  },
/*25*/ { FDIV, FLOAT+FDIV+INT, INT  },
/*MODULUS*/
/*26*/ { MOD, INT+MOD+INT, INT },
/*NEGATIVE*/
/*27*/ { MINUS, MINUS+INT, INT  },
/*28*/ { MINUS, MINUS+FLOAT, FLOAT  },
/*LOGICAL*/
/*29*/ { AND, LOG_CONS+AND+LOG_CONS, LOG_CONS  },
/*30*/ { OR,  LOG_CONS+OR+LOG_CONS, LOG_CONS  },
/*31*/ { NOT, NOT+LOG_CONS, LOG_CONS  },
/*GREATER THAN*/
/*32*/ { GT, INT+GT+INT, LOG_CONS  },
/*33*/ { GT, INT+GT+FLOAT, LOG_CONS  },
/*34*/ { GT, FLOAT+GT+FLOAT, LOG_CONS  },
/*35*/ { GT, FLOAT+GT+INT, LOG_CONS  },
/*GREATER THAN OR EQUAL*/ 
/*36*/ { GTE, INT+GTE+INT, LOG_CONS  },
/*37*/ { GTE, INT+GTE+FLOAT, LOG_CONS  },
/*38*/ { GTE, FLOAT+GTE+FLOAT, LOG_CONS  },
/*39*/ { GTE, FLOAT+GTE+INT, LOG_CONS  },
/*LESS THAN*/
/*40*/ { LT, INT+LT+INT, LOG_CONS  },
/*41*/ { LT, INT+LT+FLOAT, LOG_CONS  },
/*42*/ { LT, FLOAT+LT+FLOAT, LOG_CONS  },
/*43*/ { LT, FLOAT+LT+INT, LOG_CONS  },
/*LESS THAN OR EQUAL*/
/*44*/ { LTE, INT+LTE+INT, LOG_CONS  },
/*45*/ { LTE, INT+LTE+FLOAT, LOG_CONS  },
/*46*/ { LTE, FLOAT+LTE+FLOAT, LOG_CONS  },
/*47*/ { LTE, FLOAT+LTE+INT, LOG_CONS  },
/* DIFFERENT*/
/*48*/ { DIFF, INT+DIFF+INT, LOG_CONS  },
/*49*/ { DIFF, INT+DIFF+FLOAT,LOG_CONS  },
/*50*/ { DIFF, FLOAT+DIFF+FLOAT, LOG_CONS  },
/*51*/ { DIFF, FLOAT+DIFF+INT, LOG_CONS  },
/*52*/ { DIFF, STRING+DIFF+STRING, LOG_CONS  },
/* COMPARISON*/
/*53*/ { COMP, INT+COMP+INT, LOG_CONS  },
/*54*/ { COMP, INT+COMP+FLOAT,LOG_CONS  },
/*55*/ { COMP, FLOAT+COMP+FLOAT, LOG_CONS  },
/*56*/ { COMP, FLOAT+COMP+INT, LOG_CONS  },
/*57*/ { COMP, STRING+COMP+STRING, LOG_CONS  },
/* EXPONENT */
/*58*/ { EXP, INT+EXP+INT, INT },
/*59*/ { EXP, FLOAT+EXP+INT, INT }
 // { EXP, INT+EXP+FLOAT, INT }  //SHOULD this be allowed? depends on he code being generated
};

#define STACK_SIZE 100
static int type_stack[STACK_SIZE];
static int type_stack_ptr = 0;

#define clearTypeStack() (type_stack_ptr = 0)

static void pushType(int type)
{
 if(type_stack_ptr+1 >= STACK_SIZE)
   cleanExit(FAILURE,"Stack overflow in typestack! increase STACK_SIZE in syntax.c");

  type_stack[type_stack_ptr++] = type; 
}
static int popType()
{
  if(type_stack_ptr-1 < 0){/* printf("DEBUG trying to type pop with no elems in stack");*/
    return 0;
}
  return(type_stack[--type_stack_ptr]);  
}
static int peekType()
{
  if(type_stack_ptr-1 < 0){/* printf("DEBUG trying to type peek with no elems in stack");*/
    return 0;
}
  return(type_stack[type_stack_ptr-1]);
}
#undef STACK_SIZE

static int verifyTypes(int operator,long expr_start,long expr_end)
{
  int op2 = popType();
  int op1 = popType();
  verifyTypes2(op1,operator,op2,expr_start,expr_end);
}

static int verifyTypes2(int operand_1,int operator, int operand_2,long expr_start,long expr_end)
{  
  //if any operand is an invalid type, resulting type is no type. doing this here stops propagating error msg
  /*if( ( operand_1 == NO_TYPE ) || ( operand_2 == NO_TYPE ) )
  {  
    pushType(NO_TYPE);
    return NO_TYPE;
  }*/
  int type = operand_1 + operator + operand_2;
  int result = 0;
  for(int i = 0; i < NUM_TYPES ; i++)
  {
    if( (expr_types[i][0] == operator) && (expr_types[i][1] == type) )
    {
      result = expr_types[i][2];
      break;
    } 
  }
  if( result > 0 && ( operand_1 == INT && operator == ASS && operand_2 == FLOAT) ) //special rule, see expr_types matrix
  {
    pSemTypeConflict(operand_1,operator,operand_2,expr_start,expr_end);
    pushType(NO_TYPE);
  }
  else if( result == 0 )
  {
    pSemTypeConflict(operand_1,operator,operand_2,expr_start,expr_end);
    pushType(NO_TYPE);
  }
  else if(result >0)
  {
    pushType(result);
  }
  return result;
}

int syntax()
{ 
  token = nexTok();
  #ifdef PL0_MAIN_FIX
    label_pl0_main_fix = nextLabel();
  #endif
  while(token.type == VAR  && syntaxErrors == 0
                           && semErrors == 0 
                           && lexErrors == 0)
  {
    vars();      
  }
  //if there were errors durig global var delcarations,
  //exit inmedietly (rest of program will print false errors
  if( syntaxErrors >0 || semErrors > 0 || lexErrors > 0)
  {
    cleanExit(FAILURE,"  EXITED WITH global variable ERRORS ");
  }
  #ifdef PL0_MAIN_FIX
    codeGenPL0("JMP","0",label_pl0_main_fix);
  #endif
  while(token.type == KEY && strcmp(token.lexeme,"def") == 0)
  {
    funcs();
  //  return;
  } 
  if(buffer->offset+1 < buffer->size) //+1 needed because offset wont save EO
  {
    pSynExpectedString("keyword 'def' to define a function or recieved unexpected EOF", token.type);
    cleanExit(FAILURE,"EXITED WITH unrecoverable ERRORS");  
  }
  switch(main_defined)
  {
    case 0: pSemRawString("No function main (principal) defined");
      break;
    case 1: 
      break;
    default: pSemRawString("Too many main (principal) functions defined");
      break;
  }
  return errorCount(); 
}

static int vars()
{
  char  *variables[100]; //for cascading variables, just need to save the name becasue attribtes are all the same
  int numVars = 0;
  int constant = 0;
  do
  {
    //make a copy of the token.lexeme so when we insert it into symtab, we can free it later
    //variables[numVars] = malloc(sizeof(char) * strlen(token.lexeme) +1);
    //strcpy(variables[numVars],token.lexeme);
    variables[numVars] = token.lexeme;
    //token_t temp_token
//PL0
    param_name = token.lexeme;
//\PL0 
    if(token.type != VAR)//dont really need to check since if your called here,its because you should be here, but w-e
    {
      pSynExpected(ID,token.type);
      //token = nextTok(); //advance token
      return 0;
    }
   
    token = nexTok();
    if(token.type != ASS)
    {
      pSynExpected(ASS,token.type); 
      //token = nexTok(); //advance token
      return 0;
    }
    token = nexTok();
    numVars++;
   }while(token.type == VAR); //for cascading variable assignment
 
  if( decArreglo() || cte() ) // this order of calls is essential!
  {
 
  }
  else
  {
    pSynExpectedString("Expected constant (string,number,logical) or array ",token.type);   
    return 0; //dont need to advance, its advanced in cte() or decArreglo()
  }
  //token = nexTok();
  if(strcmp(token.lexeme,"c") == 0)
  {     
    constant = 1;
    token = nexTok();
  }

//insert to symtab here 

  //replicate the string when there are cascading vars. If this is nto done,
  //when atempting to free the value (popSymtab for example), the first token will
  //free its value and leave its pointer NULL, but the next token's pointer still
  //thinks it points to a valid address (the pointer doesnt change to NULL
  //because its a copy of the pointer, not the same. It would have been a good idea
  //to make the value element a pointer to pointer so this wouldnt be needed.
    for(int i = 0; i < numVars; i++)
    { 
      char *value_copy = NULL;
      value_copy = malloc( (sizeof(char) * strlen(value))+1);
      strcpy(value_copy,value);

      symtabInsertCurrent(variables[i],dim1,dim2,type,constant,value_copy);
//PL0
      if(!called_by_param) //if you were called in by param, you dont know the in_func_name yet and dont store default value for Pl0 code!!
      {
        value_copy = malloc( (sizeof(char) * strlen(value))+1); //we need a copy because thw above copy can be
                                                                 //freed once the symtab is poped
        strcpy(value_copy,value);
        char *full_var_name = fullVarName(variables[i]);
        char pl0_type;
        switch(type)
        {
          case INT: pl0_type = 'E'; break;
          case FLOAT: pl0_type = 'R'; break;
          case STRING: pl0_type = 'A'; break;
          case LOG_CONS: pl0_type = 'L'; break;
        }
        #ifdef PL0_MATRIX_FIX
          if(dim1 > 0)
            dim1 = 1000;
          if(dim2 > 0)
            dim2 = 1000;
        #endif
        insSymTabPL0(full_var_name,'V',pl0_type,dim1,dim2);
        if(declared_array) //arrays dont need to be initialized. declared_array is set in decArreglo
          declared_array = 0;
        else
        {
        codeGenPL0("LIT",value_copy,"0"); 
        codeGenPL0("STO","0",full_var_name); //store the default value
        } 
        
       // codegenPL0PtrFreeList(full_var_name); /*dont need to call this since its inserte into symbl table, itll free there*/
        codegenPL0PtrFreeList(value_copy);
      }
//\PL0
    }
  //type = 0; //doing this makes an error when using param(..) as it checks the type!
  value = NULL;
  return 1; 
}


static int cte() //sets global vars! only consumes token if its a constant!
{
  type = token.type; //global var type will have the type of constant 
  value = token.lexeme; //global var value will have the vale/name (lexeme) of the token

  if(type == STRING || type == INT ||
     type == FLOAT  || type == LOG_CONS)
    { 
      token = nexTok(); //consume token only if it was a constant
      return 1;
    }
  type = 0;
  value = NULL;
  return 0;  
}

static int decArreglo()
{
  dim1 = dim2 = 0;
 
  if(token.type != LB)//expect left bracket coming in or else this isnt the right derivation
    return 0;

 value = malloc(strlen("Array")+1); //could be null, but just leave this to prevent dereferencing errors. we malloc because 
                                    //later when we call free, this will be freed also. Just easier to free than to make this 
                                    // free excpetion here. this will cause some memory to be wasted, but its ok for now
 strcpy(value,"Array");
 declared_array = 1; 
 type = INT; //assume all arrays hold ints

  token = nexTok();   
  if(token.type != INT ) //only ints allowed to specify dimension
  {
    pSynExpected(INT,token.type);
    return 0;
  }
  dim1 = atoi(token.lexeme);
  if(dim1 <= 0)
  {
    pSynString("The dimension cannot be <= 0");
  }
  
  token = nexTok();
  if(token.type != RB) //close first dimension
  {  
    pSynExpected(RB,token.type);
    return 0;
  }  

  token = nexTok();
  if( token.type == LB) //if its a matrix
  {
    token = nexTok();
    if( token.type != INT )
    {
      pSynExpected(INT,token.type);
      return 0;
    }
    dim2 = atoi(token.lexeme);
    if(dim2 <= 0)
    {
      pSynString("The dimension cannot be <= 0");
    }

    token = nexTok();
    if(token.type != RB) //close second dimension
    {
      pSynExpected(RB,token.type);
      return 0;
    }
    token = nexTok(); //consume the token
  }

  return 1;
}

static int funcs()
{
  char funcBuffer[200]; // the func name is inserted with its param into symtab, like so-> func$I$S  I=integer,S=string, F =float  
  int funcBufferCounter = 0;  
  int returnType = NO_TYPE; //type can be INT, FLOAT, STRING, LOGICAL
  char *funcName = NULL; //save func name to print errors, as token will advance past function name
  function_returns = 0;
  function_returnd = 0;
  func_return_type = NO_TYPE; 
//PL0
  int pl0_line_num;
  int defining_main=0;
//\Pl0

  if( strcmp(token.lexeme,"def") != 0 )
  {
    pSynExpectedString("def",token.type);
    //return 0;
  }
  
  pushSymtab();
  
  token = nexTok();
  if( token.type != VAR)
  {
    pSynExpected(ID,token.type);
    cleanExit(FAILURE," Unrecoverable Error while defining a function");
  }
  funcName = token.lexeme;
//PL0
  pl0_line_num = code_ptr;
//\PL0
  if(strcmp(funcName,"principal") == 0)//should also check to see if it has no params!
  {
    main_defined++;
//PL0
    char *label_P = malloc(strlen("_P")+1); //need to have heap memory since we will call free on all labels later
    strcpy(label_P,"_P");
    insLabelSymTabPL0(label_P);
    #ifdef PL0_MAIN_FIX
      codeGenPL0("JMP","0","1"); //jump to first line of code to initialize vars;
      insLabelSymTabPL0(label_pl0_main_fix);
    #endif
    defining_main = 1;
//\PL0
  }
  funcBufferCounter = strlen(token.lexeme);
  if(funcBufferCounter >=99)
    printf("Warning, function name %s is greater than 100 chars (for declaration)\n",funcName);
  strncpy(funcBuffer,token.lexeme,funcBufferCounter);
  funcBuffer[funcBufferCounter] = '$'; //dollah sign means func yo

  token = nexTok();
  if( token.type != LP )
  {
    pSynExpected(LP,token.type);
    //this error is minimal, continue 
  }

  token = nexTok();
  if(token.type == VAR )
  {
    param(funcBuffer,&funcBufferCounter);
  }
  if(token.type != RP)//no else if because leaving funcs we need to see right paren
  {
    pSyn2Expected(RP,COMMA,token.type);
    //continue from this error
  }

  token = nexTok();
  if( cte() ) //if the function has a return type
  {
    function_returns = 1;
    returnType = func_return_type = type;
  }
// finish the full function signature and create a copy to export out
  funcBuffer[++funcBufferCounter] = '\0';
  char *tmpFuncName = malloc(funcBufferCounter);
  if(tmpFuncName == NULL)
  {
    cleanExit(FAILURE,"FALTAL ERROR. out of memory in funcs()");
  }
  strcpy(tmpFuncName,funcBuffer);
//need to do this because symtab saves the pointer, so once this func returns funcBuffer will cease to exist!
  symtabInsertHead(tmpFuncName,0,0,returnType,0,NULL);
//PL0
  //insert the parameters in reverse order. the call to param(...) above set the param stack, now we dump it into coegen structs
  in_func_name = tmpFuncName; //any sigsegv errors, this is where it  might happen if in_func_name is freed after tmpFuncName
  for(int i = param_stack_ptr-1; i >=0; i--) //if you were called in by param, you dont know the in_func_name yet!!
  {
    char *full_var_name = fullVarName(param_name_stack[i]);
    char pl0_type;
    switch(param_type_stack[i])
    {
      case INT: pl0_type = 'E'; break;
      case FLOAT: pl0_type = 'R'; break;
      case STRING: pl0_type = 'A'; break;
      case LOG_CONS: pl0_type = 'L'; break;
      default: pl0_type = 'Z'; break;
    }
    insSymTabPL0(full_var_name,'P',pl0_type,dim1,dim2);
    codeGenPL0("STO","0",full_var_name);
  }
  param_stack_ptr = 0;
//\PL0

  
  if(token.type == VAR)// check local variables
  {
    while(token.type == VAR)
    {
      if ( !vars() )
      {
        token = nexTok();
        continue; //this error is recoverable
      }
      
      if(token.type != COMMA)
      {
        if(token.type == VAR)
          pSynExpected(COMMA,token.type);
        else
          break;
      }
      else 
        token = nexTok(); //consume the commma
    }
  }

//PL0
  if(!main_defined)//main is set above
  {
    char *codegen_func_name = malloc(strlen(tmpFuncName)+2); //its own copy so no bs with free
    strcpy(codegen_func_name,tmpFuncName);
    char pl0_return_type;
    switch(returnType)
    {
      case INT: pl0_return_type = 'E'; break;
      case FLOAT: pl0_return_type = 'R'; break;
      case STRING: pl0_return_type = 'A'; break;
      case LOG_CONS: pl0_return_type = 'L'; break;
      case NO_TYPE: pl0_return_type = 'I';break;
    }
    insSymTabPL0(codegen_func_name,'F',pl0_return_type,pl0_line_num,0);
  }
//\PL0

  //do block here

  bloque();

  if(!function_returnd && function_returns)
    pSemFunc(funcName," Did not have a return statement");
   
  popSymtab(); 
//PL0
  if(defining_main)
    codeGenPL0("OPR","0","0");
  else
    codeGenPL0("OPR", "0", "1");
  
//\Pl0
  return 1;
}

static int param(char *funcBuffer, int *funcBufferCounter)
{
//PL0
  called_by_param = 1;
//\PL0
  do{
    if(token.type == COMMA) //if we came back here by do while, consume the comma token
      token = nexTok(); 

    if(token.type != VAR)
    {
      pSynExpected(VAR,token.type);
      return 0;
    }
    if ( !vars() ) //calling vars() will enable cascading, might want to make a var() that only checks for one var. might help e/ codgen too
    {
      pSynString("Error while declaring function parameters");
      cleanExit(FAILURE,"Unrecoverable error while declaring function parameters");
      return 0;
    }
//PL0
    param_name_stack[param_stack_ptr] = param_name; //create param stack to inverse the order. param_name set in var()
    param_type_stack[param_stack_ptr++] = type;
//PL\0
    char t;
    switch(type) //type was set in the call to vars()->cte()
    {
      case INT: t = 'I'; break;
      case FLOAT: t = 'F'; break;
      case STRING: t = 'S'; break;
      case LOG_CONS: t = 'L'; break;
      default: printf("Debug symtab.c, type of local var is unknown \n"); break;
    }
    funcBuffer[++(*funcBufferCounter)] = t;
    funcBuffer[++(*funcBufferCounter)] = '$';

  }while(token.type == COMMA);
  //}while( (token.type == COMMA ? (token = nexTok()) : 0 )); //doesnt work?
  //
//PL0
  called_by_param = 0;
//\PL0
  return 1;
}

static int bloque()
{
  if(token.type != BS)
    pSynExpected(BS,token.type); //recoverable error 

  else    
    token = nexTok();//consume the BS
  
  while(token.type != BE)
  { 
 //could use a switch for this next code also
    if(token.type == VAR) 
    {
      char *varName = token.lexeme;
      token = nexTok(); //consume the ID name
      only_function_call = 1; //see global declaration
      if(Lfunc(varName) || asigna(varName) )
        only_function_call = 0;
      else
      {
        pSynString("Expected assignment or function call");
       // cleanExit(FAILURE,"");//this is gana cascade a whole bunch of false errors
        break;
      }
    }
    else if (token.type == KEY)
      if( !( lee() || imprime() || si() || mientras() || regresa() ||
             interrumpe() || continua() || desde()  ))
      {
        pSynString("Expected one of the following keywords: [si,desde,mientras,regresa,imprime,lee] or block end [.]");
        break;
      }  

      else ; //false else so the next else wont be associated with thue upper if
    else
    {
      if(!from_regresa){
      pSynString("Expected variable (for function call or assignment), or \n" 
                       "one of the following keywords: [si,desde,mientras,regresa,imprime,lee] or block end [.]");
from_regresa = 0;
}
      break;
    }     
  }      
  
  if(token.type != BE)
    pSynExpected(BE,token.type);
  else
    token = nexTok(); //consume the BE
 

  return 1;
}

static void expr()
{
  //clearTypeStack(); //clearing stack causes asigna() to lose the type it pushed in!
  //token is already advanced coming in here
  int token_type = 0;
  long expr_start = buffer->offset - strlen(token.lexeme);
  do
  {
    if( strcmp(token.lexeme,"o") == 0 )
    {
      token_type = 1; 
      token = nexTok();
    }
  opY();  

  if(token_type)
  {
    long expr_end = buffer->offset - strlen(token.lexeme);
    verifyTypes(OR,expr_start,expr_end); //need to use a macro directly because "o" is a string keyword
//PL0
    codeGenPL0("OPR", "0", "15");
//\PL0
  } 

  }while( strcmp(token.lexeme,"o") == 0 );

  
}

static void opY()
{
  int token_type = 0;
  long expr_start = buffer->offset - strlen(token.lexeme);
  do
  {
    if( strcmp(token.lexeme,"y") == 0 )
    {
      token_type = 1; 
      token = nexTok();
    }
  opNo();

   if(token_type)
   {
     long expr_end = buffer->offset - strlen(token.lexeme);
     verifyTypes(AND,expr_start,expr_end); //need to use a macro directly because "y" is a string keyword
//PL0
    codeGenPL0("OPR", "0", "16");
//\PL0
   }


  }while( strcmp(token.lexeme,"y") == 0 );

}

static void opNo()
{
  int token_type = 0; //is this necesary to do something like if(token_type[0]) ?
  long expr_start = buffer->offset - strlen(token.lexeme);
  if( strcmp(token.lexeme,"no") == 0 )
  {
    token_type = 1;
    token = nexTok();
  }
  opRel();

  if(token_type)
  {
    long expr_end = buffer->offset - strlen(token.lexeme);
    verifyTypes2(0,NOT,popType(),expr_start,expr_end); //need to use a macro directly because "no" is a string keyword
//PL0
    codeGenPL0("OPR", "0", "17");
//\PL0
  }  
 
}

static void opRel()
{
  int token_type = 0;
  long expr_start = buffer->offset - strlen(token.lexeme);
  do
  {
  if(token.category == REL_OP)
  {
    token_type = token.type;
    token = nexTok();
  }
  opSum();
  
  if(token_type)
  {
    long expr_end = buffer->offset - strlen(token.lexeme);
    verifyTypes(token_type,expr_start,expr_end);
//PL0
    switch(token_type)
    {
      case LT: codeGenPL0("OPR","0","9"); break;
      case LTE: codeGenPL0("OPR","0","11"); break;
      case GT: codeGenPL0("OPR","0","10"); break;
      case GTE: codeGenPL0("OPR","0","12"); break;
      case DIFF: codeGenPL0("OPR","0","13"); break;
      case COMP: codeGenPL0("OPR","0","14"); break;
      case ASS: printf("DEBUG: asignment in expresions not yet implemented!\n"); break;
    }
//\PL0
  }
  
  }while(token.category == REL_OP);
}

static void opSum()
{
  int token_type = 0;
  int unary = 0; //asume the operator is unary coming in  
  long expr_start = buffer->offset - strlen(token.lexeme);
  do
  {
    if(token.category == ARITH_OP && (token.type == PLUS ||
                                      token.type == MINUS  )
                                  && unary                )
    {
      token_type = token.type; 
      token = nexTok();
    }
    opMult();

    if(token_type)// same as == PLUS || == MINUS, since as long as its != 0 it has iether PLUS or MINUS
    {
      long expr_end = buffer->offset - strlen(token.lexeme);
      verifyTypes(token_type,expr_start,expr_end);
//PL0
      switch(token_type)
      {
        case PLUS: codeGenPL0("OPR","0","2");
          break;
        case MINUS: codeGenPL0("OPR","0","3");
          break;
      }
//\PL0
    }

    if(token.type == PLUS || token.type == MINUS )
      unary = 1;      

  }while(token.type == PLUS || token.type == MINUS);
}

static void opMult()
{
  int token_type = 0;
  long expr_start = buffer->offset - strlen(token.lexeme); 
  do
  {
  if(token.category == ARITH_OP && (token.type == MULT ||
                                    token.type == DIV  ||
                                    token.type == FDIV ||
                                    token.type == MOD    ))
  {
    token_type = token.type;
    token = nexTok();
  }
  opExp();
 
  if(token_type)
  {
    long expr_end = buffer->offset - strlen(token.lexeme);
    verifyTypes(token_type,expr_start,expr_end);
//PL0
    switch(token_type)
    {
      case MULT: codeGenPL0("OPR","0","4");
        break;
      case MOD: codeGenPL0("OPR","0","6");
        break;
      case DIV: codeGenPL0("OPR","0","5");
        break;
      case FDIV: codeGenPL0("OPR","0","5"); //same as DIV for now
        break;
    }
//\PL0
  }


  }while(token.type == MULT || token.type == DIV ||
         token.type == FDIV || token.type == MOD   );
}

static void opExp()
{
  int token_type = 0;
  long expr_start = buffer->offset - strlen(token.lexeme);
  do
  {
  if( token.type == EXP )
  {
    token_type = token.type;
    token = nexTok();
  }
  opSig();

  if(token_type == EXP)
  {
    long expr_end = buffer->offset - strlen(token.lexeme);
    verifyTypes(token_type,expr_start,expr_end); 
//PL0
    codeGenPL0("OPR","0","7");
//\PL0
  }

  }while( token.type == EXP );
}

static void opSig()
{
  int token_type = 0;
  long expr_start = buffer->offset - strlen(token.lexeme); 
  if( token.type == MINUS )
  {
    token_type = token.type;
    token = nexTok();
  }
  opTerm();
  if(token_type == MINUS)
  {
    long expr_end = buffer->offset - strlen(token.lexeme);
    verifyTypes2(0,token_type,popType(),expr_start,expr_end);
//PL0
    codeGenPL0("OPR", "0", "8");
//\PL0
  }

}

static void opTerm()
{

  if(token.type == LP)
  {
    token = nexTok(); //consume LP
    expr();
    if(token.type != RP)
      pSynExpected(RP,token.type);
    else
      token = nexTok(); //consume RP
  }

  else if( cte() )
  {
    pushType(type);
//PL0
    if(type == LOG_CONS)
    {
      if(strcmp("verdadero", value) == 0)
        codeGenPL0("LIT","V","0");
      else
        codeGenPL0("LIT","F","0");
    }
    else
      codeGenPL0("LIT",value,"0"); //value is set by call to cte()
//\PL0
    value = NULL;
    type = ERR;
  }
  else
    if( token.type == VAR)
    {
      char *varName = token.lexeme;
      token = nexTok(); //consume the Var since the next 2 calls are ambiguous if the first token is var anyways
      if( Lfunc(varName) )
        ; //codegen is in Lfunc
      else //its a variable
      { 
        symtabEntry_t *symbol = NULL;
        if( (symbol = symtabLookupCurrent(varName)) == NULL)
        {
          pSemUndecSym(varName);
          pushType(NO_TYPE);
          return;
        }
        dimens();
        if( (dim2 != 0)  && (symbol->dim2 == 0) || //case for not having dimens, and  using them
            (dim1 != 0)  && (symbol->dim1 == 0) )
            pSemVar(varName,"Is being used with too MANY dimensions");
        else 
          if( (dim2 == 0)  && (symbol->dim2 != 0) || //case for  having dimens, and  not using them
            (dim1 == 0)  && (symbol->dim1 != 0) )
            pSemVar(varName,"Is being used with too FEW dimensions");
       
        pushType(symbol->type); //even with dimensions errors, push the type to stack to stop false err propagation
//PL0
        char *full_var_name = fullVarNameAfterSearch(varName);
        codeGenPL0("LOD",full_var_name,"0");
        codegenPL0PtrFreeList(full_var_name);
//\PL0
      }
    }
  else
    pSynString("Expected function call, constant, variable, or '(");     
}


static int Lfunc(char *varName)
{
   char LfuncBuff[200];
   int LfuncBuff_ptr = 0;
   int type = 0;

 /*  if( token.type != VAR)
   {
     return 0; 
   }*/
   LfuncBuff_ptr = strlen(varName);
   if(LfuncBuff_ptr >= 99)
   {
     printf("Warning, function call name %s exceeds 100 chars\n");
   }
   strncpy(LfuncBuff,varName, LfuncBuff_ptr );
   LfuncBuff[LfuncBuff_ptr] = '$';

   if( token.type != LP)
   {
     return 0; //dont print error as it might be an = or something else valid, error will be caught after all commands are tried
   }   
   token = nexTok();
//PL0 
   char *label = nextLabel();
   codeGenPL0("LOD",label,"0");
//\PL0

  
   if(token.type == RP)//if the funcall has no params
     ;
   else
   {
     do{
        if(token.type == COMMA) //if we came back here by do while, consume the comma token
          token = nexTok();
   
       expr();
       type = popType();
       char t;
       switch(type)
       {
         case INT: t = 'I'; break;
         case FLOAT: t = 'F'; break;
         case STRING: t = 'S'; break;
         case LOG_CONS: t = 'L'; break;
         case NO_TYPE: t = '?'; break;
         default: printf("Debug symtab.c, type of local var is unknown \n"); break;
       }
       LfuncBuff[++LfuncBuff_ptr] = t;
       LfuncBuff[++LfuncBuff_ptr] = '$';
     }while(token.type == COMMA);
  }
   if(token.type != RP )
     pSynExpected(RP,token.type);
 
   else
     token = nexTok(); //consume RP

   LfuncBuff[++LfuncBuff_ptr] = '\0';
   symtabEntry_t* symbol = NULL;

   if( (symbol = symtabLookupHead(LfuncBuff)) == NULL) /*lookup head, it has funcs. call to current would work also, just wasteful)*/
   {
     pSemUndecFunc(LfuncBuff);
     pushType(NO_TYPE);     
     return 1; //we return 1 even though it wasnt found, but because this Lfunc() was the right derivation
   }
   else 
     pushType(symbol->type); 



//PL0
  char *func_name_copy = malloc( (sizeof(char)) * (strlen(LfuncBuff))+2);
  strcpy(func_name_copy,LfuncBuff);
  codeGenPL0("CAL",func_name_copy,"0");
  insLabelSymTabPL0(label);
//BUG NOTE: if the function returns, but its value isnt used it is still
//  being pushed onto the stack, but it shouldnt. this is why the keyword 
//  DISCARD in some languages was needed i guess.
//FIXED: if the functions returns a value, but it wasnt a single call (not called form bloque())
  if((symbol->type != NO_TYPE ) && (only_function_call==0)) 
    codeGenPL0("LOD",func_name_copy, "0");
  codegenPL0PtrFreeList(func_name_copy);
//\PL0

   return 1;
} 

static int dimens()
{
  dim1 = dim2 = 0; /*global vars. these will be used to say if there were dimensions or not, not 
                       to store the actual offset of the dimension, that will be done in the code generated*/

  if(token.type != LB)
    return 0;
 
  token = nexTok(); //consume the left bracket '['
  //clearTypeStack();
  long expr_start = buffer->offset - strlen(token.lexeme);
  expr();
  long expr_end = buffer->offset - strlen(token.lexeme);
  dim1 = popType(); //this pops the type, not the number ( dimension to offset). the real offset should be in the generated code
 
  if(dim1 != INT)
    pSemExpectedType(INT,dim1,expr_start,expr_end,"for the first dimension"); 

  if(token.type != RB)
  {
    pSynExpected(RB,token.type);
    return 1; //it had dimens, just missing ']'
  }
  else 
   token = nexTok(); //consume ']'

  if(token.type != LB)
    return 1;
  
  token = nexTok();
  //clearTypeStack();
  int dim1_tmp = dim1;
  expr_start = buffer->offset - strlen(token.lexeme);
  expr();
  expr_end = buffer->offset - strlen(token.lexeme);
  dim1 = dim1_tmp; //this is needed because the above expr() can go into dimens again and set dim1=dim2=0!!
  dim2 = popType(); //same as above

  if(dim2 != INT)
    pSemExpectedType(INT,dim2,expr_start,expr_end,"for the second dimension");

  if(token.type != RB)
  {
    pSynExpected(RB,token.type);
    return 1; 
  }
  else
    token = nexTok();//consume the RB ']' token

  return 1;
  
}

static int asigna(char *varName)
{
  symtabEntry_t *symbol = NULL;
  int undecSymbol = 0; //flag for undeclared symbol
  long asigna_start = buffer->offset - strlen(varName) - strlen(token.lexeme) -1 ;//since we read token after var name, -1 for whitespace...
//PL0
  char *full_var_name;
//\PL0
  if( (symbol = symtabLookupCurrent(varName)) == NULL)
  {
    pSemUndecSym(varName);
   // pushType(NO_TYPE);  // pushing this propagates errors, just push the type the expression evaluates to to reduce false errors
   undecSymbol= 1;
  }
  else
  {
    pushType(symbol->type);
    if(symbol->constant)
      pSemVar(symbol->lexeme,"Was declared to be constant, it cannot be asigned to");

    dimens();
  
     if( (dim2 != 0)  && (symbol->dim2 == 0) || //case for not having dimens, and  using them
         (dim1 != 0)  && (symbol->dim1 == 0) )
         pSemVar(varName,"Is being used with too MANY dimensions");
     else
       if( (dim2 == 0)  && (symbol->dim2 != 0) || //case for  having dimens, and  not using them
           (dim1 == 0)  && (symbol->dim1 != 0) )
         pSemVar(varName,"Is being used with too FEW dimensions");
//PL0
     full_var_name = fullVarNameAfterSearch(varName);
//\PL0
  }
//  else
  // token = nexTok();

  if(token.type != ASS ) //we expect ASS because funcs() was already evaluated to be false!
    pSynExpected(ASS,token.type);

  else
    token = nexTok(); //consume dat ASS ;)

  expr();
//PL0
  codeGenPL0("STO","0",full_var_name);
  codegenPL0PtrFreeList(full_var_name);
//\PL0
  long expr_end = buffer->offset - strlen(token.lexeme);

  if(undecSymbol)
    pushType(peekType()); //for correct ASS evaluation and to stop false error propagation

  return verifyTypes(ASS,asigna_start,expr_end);  
  
}

static int lee()
{
  int dimens_called = 0; //dirty check to see if dimens was called ( as it advances toke, if not weil advance it)

  if(strcmp(token.lexeme,"lee") != 0)
    return 0; //not an error, this just wasnt the right derivation
   
  token = nexTok();
  if(token.type != LP)
    pSynExpected(RP,token.type);  
  
  else
    token = nexTok();
  
  if(token.type != VAR)
    pSynExpected(VAR,token.type);

  else //token is a VAR, but check if its been declared
  {
    char *varName = token.lexeme;
    symtabEntry_t *symbol = NULL;
    
    if( (symbol = symtabLookupCurrent(varName)) == NULL)
       pSemUndecSym(varName); 
    else
    {
      if(symbol->constant)
        pSemVar(symbol->lexeme,"Was declared to be constant, it cannot be asigned to");
    
      token = nexTok(); //consume the var token   
      dimens() ; dimens_called=1;
      if( (dim2 != 0)  && (symbol->dim2 == 0) || //case for not having dimens, and  using them
          (dim1 != 0)  && (symbol->dim1 == 0) )
        pSemVar(varName,"Is being used with too MANY dimensions");
      else
        if( (dim2 == 0)  && (symbol->dim2 != 0) || //case for  having dimens, and  not using them
            (dim1 == 0)  && (symbol->dim1 != 0) )
          pSemVar(varName,"Is being used with too FEW dimensions");
//PL0
      char *full_var_name = fullVarNameAfterSearch(varName);/*malloc( sizeof(varName) + sizeof(in_func_name) + 2);
      strcat(full_var_name,varName);
      if(found_in_depth > 1) //its a local func
      strcat(full_var_name,in_func_name); */
      codeGenPL0("OPR",full_var_name,"19");
      codegenPL0PtrFreeList(full_var_name);
//\PL0
    }
  }
  
  if(!dimens_called) token = nexTok(); //see comment for dimens_called
  
  if(token.type != RP)
    pSynExpected(RP,token.type);
  
  else
    token = nexTok(); //consume the RP

  return 1;
}

static int imprime()
{

  if(strcmp(token.lexeme,"imprime") != 0)
    return 0; //not an error, just not the right derivation

  token = nexTok(); //comsume "imprime"

  if(token.type != LP)
    pSynExpected(LP,token.type);

  else
    token = nexTok(); //consume LP

  do
  {

    if(token.type == COMMA)
    {
      token = nexTok(); //consume the comma
//PL0
      if( (strcmp(token.lexeme,"\"\\n\"") == 0)  || (strcmp(token.lexeme,"'\\n'") == 0 ))
      {
        codeGenPL0("LIT","","0");
        codeGenPL0("OPR","0","21");
        token = nexTok(); // consume the line jump token
        if(token.type == RP) //if line jump is the last thing to print, then dont print after do while (stack is empty)
        {  
           break;
        } 
        if(token.type == COMMA)
          token = nexTok();
       } 
    }
//\Pl0
    clearTypeStack();
    expr();
//PL0
    codeGenPL0("OPR","0","20");
//\PL0    
  }while(token.type == COMMA);

 
  if(token.type != RP)
    pSyn2Expected(RP,COMMA,token.type);

  else
    token = nexTok();

  return 1;
}

static int si()
{
  if(strcmp(token.lexeme,"si") != 0) 
    return 0;

  token = nexTok(); //consume "si"

  long expr_start = buffer->offset - strlen(token.lexeme);
  expr();
  long expr_end = buffer->offset - strlen(token.lexeme);
//PL0
  char *label_start_of_else = nextLabel();
  codeGenPL0("JMC","F",label_start_of_else);
//\PL0

  int type;
  if( (type = popType()) != LOG_CONS)
    pSemExpectedType(LOG_CONS,type,expr_start,expr_end,"for command [si]");

  bloque();

  while(strcmp( token.lexeme,"otsi") == 0)
  {
//PL0
    char *label_end_of_otsi = nextLabel();
    codeGenPL0("JMP","0",label_end_of_otsi);
    insLabelSymTabPL0(label_start_of_else);
    label_start_of_else = nextLabel();
//\Pl0
    token = nexTok(); //consume "otsi"
    long expr_start = buffer->offset - strlen(token.lexeme);
    expr();
    long expr_end = buffer->offset - strlen(token.lexeme);
//PL0
    codeGenPL0("JMC","F",label_start_of_else);
//\PL0
    if( (type = popType()) != LOG_CONS)
      pSemExpectedType(LOG_CONS,type,expr_start,expr_end,"for command [otsi]");
   
    bloque();   
//PL0
    insLabelSymTabPL0(label_end_of_otsi);
//\Pl0
  }
  
  if( strcmp(token.lexeme,"sino") == 0)
  {
//PL0
    char *label_end_of_else = nextLabel();
    codeGenPL0("JMP","0",label_end_of_else);
    insLabelSymTabPL0(label_start_of_else);
//\PL0
    token = nexTok(); //consume "sino"
    bloque();
//PL0
   insLabelSymTabPL0(label_end_of_else);
//\PL0
  }
//PL0
  else
    insLabelSymTabPL0(label_start_of_else);
//\PL0
  return 1;

}

static int mientras()
{
  if(strcmp(token.lexeme,"mientras") != 0)
    return 0;

  in_loop = 1;
  token = nexTok(); //consume "mientras"

//PL0
  label_loop_start = nextLabel();
  label_loop_end  = nextLabel();
  char *tmp_label_loop_start = label_loop_start;
  char *tmp_label_loop_end = label_loop_end;
  insLabelSymTabPL0(label_loop_start);
//\Pl0
 
  expr();

//Pl0
  codeGenPL0("JMC","F",label_loop_end);
//\PL0

  int type;
  if( (type = popType()) != LOG_CONS)
     pSynExpected(LOG_CONS,type);
 bloque();
 label_loop_end = tmp_label_loop_end; //since label_loop is global, calls to block can corrupt it
 label_loop_start = tmp_label_loop_start; //same

//PL0
  codeGenPL0("JMP","0",label_loop_start);
  insLabelSymTabPL0(label_loop_end);
//\PL0

  in_loop = 0;
  return 1; 
}

static int regresa()
{
from_regresa = 0;
  if(strcmp(token.lexeme,"regresa") != 0)
    return 0;

from_regresa = 1;

  token = nexTok();

  if(function_returns)
  {
    int type;
    long expr_start = buffer->offset - strlen(token.lexeme);
    clearTypeStack();
    expr();
    long expr_end = buffer->offset - strlen(token.lexeme);
    
    if( (type = popType()) != func_return_type)
      pSemExpectedType(func_return_type,type,expr_start,expr_end,"to satisfy return type");
//PL0
    codeGenPL0("STO", "0", in_func_name);
//\PL0
  }
  function_returnd = 1;
//PL0
  codeGenPL0("OPR", "0", "1");
//\PL0
  return 1;
}

static int interrumpe()
{
  if(strcmp(token.lexeme,"interrumpe") != 0)
    return 0;

  token = nexTok(); //consume "imprime"

  if(!in_loop)
    pSemString("keyword [interrumpe] cannt be used outside of a loop (mientras or desde)");
  else
    codeGenPL0("JMP","0",label_loop_end); 
  
  return 1;
}

static int continua()
{
  if(strcmp(token.lexeme,"continua") != 0)
    return 0;

  token = nexTok();
  
  if(!in_loop)
    pSemString("keyword [continua] cannot be used outside of a loop (mientras or desde)");

  else
    if(loop_pasa)//since desde does its increment and check at the end of the loop (so we can 'touch'the last value
    {  
      continua_called = 1;
      label_continua= nextLabel();
      codeGenPL0("JMP","0",label_continua);
    }
    else
      codeGenPL0("JMP","0",label_loop_start); 
  return 1;
}
static int desde()
{
  if(strcmp(token.lexeme,"desde") != 0)
    return 0;
  
  token = nexTok(); //consume "desde
  dim1 = dim2 = 0; 
  var_type = NO_TYPE;
  in_loop = 1;
//PL0
  char *full_var_name = "Error"; //so you wont dereference null when doing codegen incase there a syntax/semantic errors
  label_loop_start = nextLabel();
  label_loop_end = nextLabel();
   // since the label vars are global, calls to a bloque can change its value and corrupt the labels! thus the temprary vars
  char *tmp_label_loop_end = label_loop_end;
  char *tmp_label_loop_start = label_loop_start; 
//\PL0
  if(token.type != VAR)
    pSynExpected(VAR,token.type);
  
  else //token is a VAR, but check if its been declared
  {
    char *varName = token.lexeme;
    symtabEntry_t *symbol = NULL;

    if( (symbol = symtabLookupCurrent(varName)) == NULL)
    {
      pSemUndecSym(varName);
      token = nexTok(); //consume the var token
    }
    else
    {
      if(symbol->constant)
        pSemVar(symbol->lexeme,"Was declared to be constant, it cannot be asigned to");

      var_type = symbol->type;
      token = nexTok(); //consume the var token   
      dimens() ;
      if( (dim2 != 0)  && (symbol->dim2 == 0) || //case for not having dimens, and  using them
          (dim1 != 0)  && (symbol->dim1 == 0) )
        pSemVar(varName,"Is being used with too MANY dimensions");
      else
        if( (dim2 == 0)  && (symbol->dim2 != 0) || //case for  having dimens, and  not using them
            (dim1 == 0)  && (symbol->dim1 != 0) )
          pSemVar(varName,"Is being used with too FEW dimensions");
//PL0
        full_var_name = fullVarNameAfterSearch(varName);
//\PL0
    }
  }
  
  if(strcmp(token.lexeme,"en") != 0)
    pSynExpectedString("keyword [en]",token.type);  
 
  else
    token = nexTok();//consume "en"
 
  Lexp(full_var_name);
//PL0
  if(!loop_pasa) //if its an explicit array loop
  {
    insLabelSymTabPL0(label_loop_start);
    codeGenPL0("STO","0",full_var_name);
    codeGenPL0("LOD",full_var_name,"0");
    codeGenPL0("LIT","2$$$###...","0");//compare against end marker. the 2 digit is needed because of shitty vm
    codeGenPL0("OPR","0","13");
    codeGenPL0("JMC","F",label_loop_end);
  }
//\PL0

  bloque();
   // since the label vars are global, calls to a bloque can change its value and corrupt the labels!
  label_loop_end = tmp_label_loop_end;
  label_loop_start = tmp_label_loop_start;
//PL0
  if(loop_pasa) //if its a "pasa" type loop 
  {
    if(continua_called)//so continua will come here to increment and check condition. loop_start doesnt do this, loop_end exits. this the need.
    {
      insLabelSymTabPL0(label_continua);
      continua_called = 0;
    }
    codeGenPL0("LOD",full_var_name,"0");
    codeGenPL0("OPR","0","2"); //add the increment as defined by "pasa"
    codeGenPL0("STO","0",full_var_name);
    //check to see if we need to loop again, or exit loop
    codeGenPL0("LOD",full_var_name,"0");
    if(loop_incrementing)
      codeGenPL0("OPR","0","12");
    else
      codeGenPL0("OPR","0","11");
   
    codeGenPL0("JMC","F",label_loop_end);
  }
  codegenPL0PtrFreeList(full_var_name);
  codeGenPL0("JMP","0",label_loop_start);
  insLabelSymTabPL0(label_loop_end);
//\PL0
  in_loop = 0;
  return 1;
}

static int Lexp(/*PL0*/char *full_var_name/*\PL0*/)
{
  if(token.type == LB) //array 
  {
//PL0
  loop_pasa = 0;
  codeGenPL0("LIT","2$$$###...","0"); //insert end marker. the 2 digit is needed because of shitty vm
  int flop_index = code_ptr ; //-1 because code_ptr always points to the next to write
//\PL0
    token = nexTok(); //consume '['
    do
    {
      if(token.type == COMMA)
        token = nexTok();
      clearTypeStack();
      long expr_start = buffer->offset - strlen(token.lexeme);
      expr();
      
      long expr_end = buffer->offset - strlen(token.lexeme);      
      if(peekType() != var_type)
        pSemExpectedType(var_type,popType(),expr_start,expr_end," for explicit array in desde command");

    }while(token.type == COMMA);
//PL0
    codeGenPL0Flop(flop_index);
//\PL0
    if(token.type != RB)
      pSynExpected(RP,token.type);
  
    else
      token = nexTok(); //consume ']'
  }

  else //expresion list
  { 
    int type;
    loop_pasa = 1;
    clearTypeStack();
    long expr_start = buffer->offset - strlen(token.lexeme);
    expr();
    long expr_end = buffer->offset - strlen(token.lexeme);
    type = popType();
    if(type != var_type)
      pSemExpectedType(var_type,type,expr_start,expr_end,"for start condition in desde");
//PL0
    codeGenPL0("STO","0",full_var_name);
    //dont add to symtab pointer list here becaus eit was a parameter! it was already added by whoever called this func
//\PL0

    if( strcmp(token.lexeme,".") != 0)
      pSynExpectedString("list operator [.]",token.type);

    else
      token = nexTok(); //consume '.'

    if( strcmp(token.lexeme,".") != 0)
      pSynExpectedString("list operator [.]",token.type);

    else
      token = nexTok(); //consume '.'

    clearTypeStack();

//PL0
    insLabelSymTabPL0(label_loop_start);
//\PL0

    expr_start = buffer->offset - strlen(token.lexeme);
    expr();
    expr_end = buffer->offset - strlen(token.lexeme);
    type = popType();
    if(type != var_type)
      pSemExpectedType(var_type,type,expr_start,expr_end,"for end condition in desde");

    if(strcmp(token.lexeme,"pasa") != 0)
      pSynExpectedString("keyword [pasa]",token.type);

    else
      token = nexTok(); //consume "pasa"

    clearTypeStack();
    expr_start = buffer->offset - strlen(token.lexeme);
    expr();
    expr_end = buffer->offset - strlen(token.lexeme);
//PL0
   if(previousCodeGenPL0(1).dir2[0] == '8')//if the loop is decrementing 
     loop_incrementing = 0; //
   else
     loop_incrementing = 1;
//\PL0
    
    type = popType();
    if(type != INT)
      pSemExpectedType(INT,type,expr_start,expr_end,"for increment/decrement in desde");
  }
}

//PL0
static char* fullVarName(char* varName)
{
  char *fvn = malloc( strlen(varName) + strlen(in_func_name) + 1);
  if(fvn == NULL)
    cleanExit(FAILURE,"Out of mem in func fullVarName\n");
  strcpy(fvn,varName);
  strcat(fvn,in_func_name);
  return fvn;
}
//after doing a symtab search, this will complete the full var name
static char *fullVarNameAfterSearch(char *varName)
{
  char *fvn = malloc( strlen(varName) + strlen(in_func_name) + 1);
  if(fvn == NULL)
    cleanExit(FAILURE,"Out of mem in func fullVarNameAfterSearch\n");
  strcpy(fvn,varName);
  if(found_in_depth > 1) //its a local func
  {
    strcat(fvn,in_func_name);
    found_in_depth = 0;
  }
  return fvn;
}
//\PL0

