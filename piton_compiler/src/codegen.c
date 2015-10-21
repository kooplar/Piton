/* Bryan Martin Tostado
 *  created: 25/march/2014
 *
 *  Description:
 *    Generates code for a PL/0 stack machine
 *
 */

#include "./include/codegen.h"
#include "./include/errors.h"
#include <stdio.h>
#include <stdlib.h>

char *outputFileName = "code.eje";
static int label_counter = 1;
static char *codegen_ptr_list[400]; //list of pointers that were inserted to the symtab that SHOULD be freed. most
                                   // of the strings inserted into symtab are raw strings, so this is necesary
static int  codegen_ptr_list_index=0;

//symbol table
static struct codegen_symtab_t
{
  char *name;
  char class;
  char type;
  int dim1;
  int dim2;
  /*char *value; not needed, stack machine will handle this*/
}symtab[MAX_PL0_SYMTAB];
int symtab_ptr = 1;

static struct codegen_code_t code[MAX_PL0_SYMTAB];
int code_ptr=1;

char* nextLabel()
{
  char *nextLabel = malloc( (sizeof(char)) * 7); // 2 for _E, rest for number
  sprintf(nextLabel,"_E%d",label_counter++);
  return nextLabel;
}

void insLabelSymTabPL0(char *name_)
{
  insSymTabPL0(name_,'I','I',code_ptr,0);
  return;
}

void insSymTabPL0(char *name_, char class_, char type_, int dim1_, int dim2_)
{
  symtab[symtab_ptr].name = name_;
  symtab[symtab_ptr].class = class_;
  symtab[symtab_ptr].type = type_;
  symtab[symtab_ptr].dim1 = dim1_;
  symtab[symtab_ptr++].dim2 = dim2_;
  return;
}
#include <string.h>
void codeGenPL0(char *mnemo_, char *dir1_,char *dir2_)
{
  code[code_ptr].mnemo = mnemo_;
  code[code_ptr].dir1 = dir1_;
  code[code_ptr++].dir2 = dir2_;
/*printf("\n\n CODE:\n");
for(int i = 1;i < code_ptr; i++){

 printf("%s %s,%s\n",code[i].mnemo, code[i].dir1,code[i].dir2);

}
*/
  return;
}
void repeatLastCodeGenPL0(int i)
{
  if(i>=code_ptr) //there is no previous code generated
    return;
  code[code_ptr].mnemo = code[code_ptr-i].mnemo;
  code[code_ptr].dir1 = code[code_ptr-i].dir1;
  code[code_ptr].dir2 = code[code_ptr-i].dir2;
  code_ptr++;
}
struct codegen_code_t previousCodeGenPL0(int i)
{
  if(i>=code_ptr)
    return (struct codegen_code_t){0,0,0};
  return code[code_ptr-i];
}

void codeGenPL0Index(char *mnemo_, char *dir1_,char *dir2_,int i)
{
  code[i].mnemo = mnemo_;
  code[i].dir1 = dir1_;
  code[i].dir2 = dir2_;
}
void codeGenPL0Flop(int flop_index)
{
  if(flop_index < 2 || flop_index >= code_ptr-2)
    return;
  
  int last_index = code_ptr - 1; // -1 becuse code_ptr points to the next writeable index
  struct codegen_code_t tmp;
  for(;flop_index <= last_index; flop_index++,last_index--)
  {
    tmp.mnemo = code[flop_index].mnemo;
    tmp.dir1 = code[flop_index].dir1;
    tmp.dir2 = code[flop_index].dir2;
  
    code[flop_index].mnemo = code[last_index].mnemo;
    code[flop_index].dir1= code[last_index].dir1;
    code[flop_index].dir2= code[last_index].dir2;

    code[last_index].mnemo = tmp.mnemo;
    code[last_index].dir1= tmp.dir1;
    code[last_index].dir2= tmp.dir2;
  }

}
void writePL0()
{
  FILE *file = NULL;
  int i = 0; 

  if( (file = fopen(outputFileName,"w") ) == NULL)
  {
    printf("Could not open file: %s to write the PL0 code\n",outputFileName);
    cleanExit(FAILURE,"Error witing pl0 file");  
  } 
  for(i=1; i < symtab_ptr; i++)
  {
    fputs(symtab[i].name,file);
    fputc(',',file);
    fputc(symtab[i].class,file);
    fputc(',',file);
    fputc(symtab[i].type,file);
    fputc(',',file);
    fprintf(file,"%d",symtab[i].dim1);
    fputc(',',file);
    fprintf(file,"%d",symtab[i].dim2);
   
    fputs(",#,\n",file);
  }
  fputs("@\n",file);

  for(i = 1;i < code_ptr; i++)
  {
    char lineNum[5];
    sprintf(lineNum,"%d",i);
    fputs(lineNum,file);
    fputc(' ',file);

    fputs(code[i].mnemo,file);
    fputc(' ',file);
    fputs(code[i].dir1,file);
    fputs(", ",file);
    fputs(code[i].dir2,file);
    if( i+1 < code_ptr)
      fputc('\n',file);
  }
  fclose( file);
}
void  codegenPL0PtrFreeList(char *string)
{
   codegen_ptr_list[codegen_ptr_list_index++] = string;
}

void codegenFreePL0()
{
  int i;
  for(i = 0;i < codegen_ptr_list_index;i++)
  {
    free(codegen_ptr_list[i]);
  }
  for(i = 1;i < symtab_ptr;i++)
  {
    free(symtab[i].name);   
  }

}



