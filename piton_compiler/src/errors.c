/*
 * Bryan Martin Tostado
 * created  April 2014
 *
 * Descripticion:
 *  Various entry points to print different kinds of error msgs. Most error messages contain line and char number. clean Exit will release all held
 *   memory and terminate the program. Any exit from this software should exit using cleanExit 
 *
 *
 * Notes:
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include "./include/errors.h"
#include "./include/lexer.h"
#include "./include/codegen.h"
#include "./include/symtab.h"

/*global vars*/
int syntaxErrors = 0;
int lexErrors = 0;
int semErrors = 0;
/*FUNCS*/

void cleanExit(int status,char *msg)
{
  char *exitStatus;
  switch(status)
  {
    case SUCCESS: exitStatus = "SUCCESS"; break;
    case FAILURE: exitStatus = "FAILURE"; break;
    default : "DEBUG, wrong param passed into cleanExit"; break;
  }
  printf("[EXIT] [%s] %s \n ",exitStatus,msg);
  //cleanup memory here
 /* free(buffer->buf);
  free(buffer); 
  codegenFreePL0(); //causes sigsegv in some situations. debug func codegenPL0PtrFreeList(char*), maybe nulls are passed in
  freeLexemes();
  freeSymtabs();
  */
   exit(status);
}

int errorCount()
{
  return syntaxErrors+lexErrors+semErrors;
}

//lex errors
/* -----------------------------------------------------------------------------------*/

void pLexSym(char *msg,char symbol)
{
  printf(" %ld : %ld - [Lex Error] %s  [%c]\n",buffer->lineNum,buffer->charNum,msg,symbol);
  lexErrors++;
}
void pLexSymString(char *msg,char* symbol)
{
  printf(" %ld : %ld - [Lex Error] %s  [%s]\n",buffer->lineNum,buffer->charNum,msg,symbol);
  lexErrors++;
}
void pLexString(char *msg)
{
  printf(" %ld : %ld - [Lex Error] %s \n",buffer->lineNum,buffer->charNum,msg);
  lexErrors++;
}

//syntax errors
/* ------------------------------------------------------------------------------*/

void pSynExpected( int expected,int got)
{
   printf(" %ld : %ld - [Syntax Error] Expected %s got %s\n",buffer->lineNum,buffer->charNum,token2String(expected),token2String(got));  
   syntaxErrors++;
}

void pSyn2Expected( int expected1,int expected2,int got)
{
   printf(" %ld : %ld -  [Syntax Error] Expected %s or %s got %s\n",buffer->lineNum,buffer->charNum,token2String(expected1),token2String(expected2),token2String(got));
   syntaxErrors++;
}

void pSynExpectedString( char *expected, int got)
{
   printf(" %ld : %ld -  [Syntax Error] Expected %s got %s\n",buffer->lineNum,buffer->charNum,expected,token2String(got));
   syntaxErrors++;
}

void pSynRawString(char *msg)
{
  printf(" [Syntax Error] %s\n"/*buffer->lineNum,buffer->charNum*/,msg);
  syntaxErrors++;
}

void pSynString(char *msg)
{ 
  printf(" %ld : %i - [Syntax Error] %s\n",buffer->lineNum,buffer->charNum,msg);
  syntaxErrors++;
}


//semantic errors
/* ------------------------------------------------------------------*/

static void getExprString(long expr_start,long expr_end, char  b[], int buff_size)
{
   int i = 0;
   for(; (expr_start <= expr_end) && i< buff_size; expr_start++,i++)
     b[i] = buffer->buf[expr_start];

   b[i-1] = '\0';
}

void pSemString(char *msg)
{
  printf(" %ld : %i - [Semantic Error] %s \n",buffer->lineNum,buffer->charNum,msg);
  semErrors++;
}
void pSemRawString(char *msg)
{
  printf(" [Semantic Error] %s \n",msg);
  semErrors++;
}

void pSemDupeSymbol(char *msg)
{
  printf(" %ld : %i - [Semantic Error] The identifier : [%s] was already previously declared \n",buffer->lineNum,buffer->charNum,msg);
  semErrors++;
}

void pSemUndecSym(char *symbolName)
{
  printf(" %ld : %i - [Semantic Error] The identifier : [%s] has not been (previously) declared\n",buffer->lineNum,buffer->charNum,symbolName);     
  semErrors++;
}
void pSemTypeConflict(int operand_1, int operator, int operand_2,long expr_start,long expr_end)
{

  char m[100];
  getExprString(expr_start,expr_end,m,100);

  printf(" %ld : %i - [Semantic Error] Type conflict in the operation : %s %s %s  [%s] \n",
           buffer->lineNum,buffer->charNum,
           token2String(operand_1),token2String(operator),token2String(operand_2),
           m); 
  semErrors++;
}
void pSemUndecFunc(char *func)
{
  printf(" %ld : %i - [Semantic Error] The function type [",buffer->lineNum,buffer->charNum);
  int idx = 0;

  for( idx = 0; (func[idx] != '$') && (func[idx] != '\0') ; idx++)
    ; 
  printf("%.*s(",idx,func);
  
  for( ++idx; func[idx] != '\0'; idx+=2) //+2 to skip the '$'
  {
    switch(func[idx])
    {
      case 'I': printf(" Integer "); break;
      case 'L': printf(" Logical "); break;
      case 'S': printf(" String "); break;
      case 'F': printf(" Float "); break;
      case '?': printf(" Unknown "); break;
    }
  }
 printf(")] has not been declared \n");
 semErrors++;
}
void pSemVar(char *var, char *msg)
{
  printf(" %ld : %i - [Semantic Error] The variable [%s] %s\n",buffer->lineNum,buffer->charNum,var,msg);
  semErrors++;
}
void pSemExpectedType(int expected,int got, long expr_start,long expr_end, char *m2)
{
  char m[100];
  getExprString(expr_start,expr_end,m,100);

  printf(" %ld : %i - [Semantic Error] Expected the type: %s to result from the expresion [%s], but got type: %s %s\n",
         buffer->lineNum,buffer->charNum,
         token2String(expected),m,token2String(got),m2);
  semErrors++;
}

void pSemFunc(char* funcName, char *msg)
{
  printf(" %ld : %i - [Semantic Error] The function [%s] %s\n" ,buffer->lineNum,buffer->charNum,funcName,msg);
  semErrors++;  
}

