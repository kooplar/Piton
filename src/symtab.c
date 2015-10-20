/*
 * Bryan Martin Tostado
 * created 13 march 2014
 *
 * Descripticion:
 *  handles files
 *
 *
 * Notes:
 *
 */


#include "include/symtab.h"
#include "include/lexer.h"
#include "include/errors.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//Pl0
int found_in_depth;
//\PL0

/* static prototypes */
static void initializeSymtab(); /* initializes symtab. Create first symtab*/
static int popSymtab_real(); /*popSymtab calls this after checking if its ok to pop*/

/* vars*/
static int symtabDepth = 1; //depth (block, scope) of the symtab. 1 is first (globals)
/*should be static*/symtabLevel_t *symtabHead = NULL; //first symtab, has globals and fucs
symtabLevel_t *symtabCurrent = NULL;
/* funcs*/

static void initializeSymtab()
{
  static int called_init = 0;
  if(!called_init)
  { 
    called_init = 1; 
    pushSymtab(); 
  }
  return;
}

int pushSymtab()
{
  initializeSymtab(); //this is ok, initializeSymtab is protected from multpile calls

  symtabLevel_t *symtab = NULL;
  if(( symtab = (symtabLevel_t*)(malloc(sizeof(symtabLevel_t)))) == NULL)
  {
    perror("out of mem in pushSymtab(): ");
    exit(0);
    return 0;
  }

  symtab->depth = symtabDepth;
  symtab->next = NULL;
  symtab->prev = symtabCurrent;//no error if called into here by initializeSymtab or first call to newSymtab() as symtabCurrent is NULL;
  if(symtabCurrent!= NULL) //if its not the first symtab being created
  {
    symtabCurrent->next = symtab;
  }
  symtabCurrent= symtab;
  if( symtabHead == NULL) //if hea is null, this symtab is first being created
  {
    symtabHead = symtabCurrent;
  }

  for(int i = 0; i < HASHSIZE; i++)
  {
    /*symtabCurrent->entry[i].lexeme= NULL;
    symtabCurrent->entry[i].dim1 = 0;
    symtabCurrent->entry[i].dim2 = 0;
    symtabCurrent->entry[i].type = ERR;
    symtabCurrent->entry[i].nextSymbol= NULL;
    symtabCurrent->entry[i].value = NULL;
    symtabCurrent->entry[i] = NULL;*/
    symtabCurrent->entry[i] = NULL;
  }
  return symtabDepth++;
}

int popSymtab()
{
  //Error conditions:
  //1. if currentSymTab is NULL
  //2. if currentSymtab is = symtabHead
  if(symtabCurrent == NULL)
  {
    return 0;  
  }
  if(symtabCurrent == symtabHead)
  {
    perror("DEBUG ERROR, attempting to pop symtab HEAD\n");
    return 0;
  }

  return popSymtab_real();
}

static int popSymtab_real()
{
  for(int i = 0;i< HASHSIZE; i++)
  {
    if(symtabCurrent->entry[i] != NULL)
    {
      if(symtabCurrent->entry[i]->lexeme != NULL)
        free(symtabCurrent->entry[i]->lexeme);/*symtabCurrent->entry[i]->value = NULL;*/ //this POS was the bug..
      if(symtabCurrent->entry[i]->value != NULL)
        free(symtabCurrent->entry[i]->value); 

      symtabEntry_t *symbolBucket = symtabCurrent->entry[i]->nextSymbol;
      while( symbolBucket  != NULL)
      {
        if(symbolBucket->lexeme != NULL)
          free(symbolBucket->lexeme);
        if(symbolBucket->value != NULL)
          free(symbolBucket->value);

        symbolBucket = symbolBucket->nextSymbol;
      }
      free(symtabCurrent->entry[i]);
    }
  }
  symtabLevel_t *tmpSymtab = symtabCurrent->prev;
  free(symtabCurrent);
  symtabCurrent= tmpSymtab;
  if(symtabCurrent != NULL) //if the head was poped, then the current symtab can be null! (if freeSymtabs is called)
    symtabCurrent->next = NULL;
  symtabDepth--;

  return 1;
}


int symtabInsertCurrent(char* lexeme,int dim1,int dim2,int type,int constant,char *value)
{
  return symtabInsert(lexeme, symtabCurrent,dim1,dim2,type,constant,value);
}

int symtabInsertHead(char* lexeme,int dim1, int dim2,int type,int constant,char *value)
{
  return symtabInsert(lexeme, symtabHead,dim1,dim2,type,constant,value);
}


int symtabInsert(char *lexeme, symtabLevel_t *symtab,int dim1,int dim2,int type,int constant,char *value)
{
  if( symtabHead == NULL)
  {
    initializeSymtab();
    symtab = symtabCurrent;
  }
  symtabEntry_t *tokenEntry = NULL;
  int hashVal;
  hashVal = hash(lexeme);
  
  tokenEntry = malloc(sizeof(symtabEntry_t));
   tokenEntry->lexeme = lexeme;
   tokenEntry->dim1 = dim1;
   tokenEntry->dim2 = dim2;
   tokenEntry->type = type;
   tokenEntry->constant = constant;
   tokenEntry->value = value;
   tokenEntry->nextSymbol= NULL;
  symtabEntry_t *nextSymbolListStart = symtab->entry[hashVal];

  if(symtab->entry[hashVal] == NULL)
    symtab->entry[hashVal] = tokenEntry;

  //traverses list to insert a token that had same hash (bucket)      
  else
  {
    symtabEntry_t *nextSymbolListStart = symtab->entry[hashVal];
    while( symtab->entry[hashVal] != NULL)//dont really need the condition, weil break out, but just for safekeeping
    {
      //if a same named token is already in the symtab
      if( strcmp(lexeme,symtab->entry[hashVal]->lexeme) == 0)
      {
         pSemDupeSymbol(lexeme); 
         return 0;
      }
       if( symtab->entry[hashVal]->nextSymbol == NULL)
       {
         symtab->entry[hashVal]->nextSymbol = tokenEntry;
         break;
       }
       else
         symtab->entry[hashVal] = symtab->entry[hashVal]->nextSymbol;
    }
    //return the entry list to the begining
    symtab->entry[hashVal] = nextSymbolListStart;
  }
  
  return 1; 
}

symtabEntry_t* symtabLookupCurrent(char *symbol)
{
  return symtabLookup(symbol, symtabCurrent);
}

symtabEntry_t* symtabLookupHead(char *symbol)
{
  return symtabLookup(symbol, symtabHead);
}

symtabEntry_t* symtabLookup(char *lexeme, symtabLevel_t *symtab)
{
  if( symtabHead == NULL )
  {
    initializeSymtab();
    //symtab = symtabCurrent;
    return NULL;
  }
  if( symtab == NULL )
  {
    printf("DEBUG - in symtabLookUp, the symtab passed in is NULL\n");
    return NULL;
  }
  int hashVal = hash(lexeme);
  symtabLevel_t *scope  = symtab; 
  symtabEntry_t *symbol = NULL;
  
  while( scope != NULL)
  {
    symbol = scope->entry[hashVal];
    while( symbol != NULL /*symbol->lexeme != NULL*/)
    {
      if( strcmp(symbol->lexeme, lexeme) == 0)
      {
//PL0
        found_in_depth = scope->depth;
//\PL0
        return symbol;
      }
      symbol = symbol->nextSymbol; 
    }
    scope = scope->prev;
  }  

  //pSemUndecSym(lexeme);
  return NULL;//caller is expected to handle this error
}


int hash(const char *string)
{
  int value = -1;
  int i = 0;
  char c;
  while((c = string[i]) != '\0')
  {
    value += (c*i) + c; //shitty hash function
    i++;
  }  
  return (value % HASHSIZE);
}

void freeSymtabs()
{
  while(symtabCurrent != NULL)
  {
    popSymtab_real();
  }
//printf("\n scope: %i \n",symtabHead->depth);
//symtabCurrent = symtabHead;
//popSymtab_real();
}
