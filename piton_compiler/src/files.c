/*
 * Bryan Martin Tostado
 * created 9 march 2014
 *
 * Descripticion:
 *  handles files
 *
 *
 * Notes:
 *
 */

#include "./include/files.h"
#include <stdio.h>
#include <stdlib.h>


int fill_buff(char *fileName, buffer_t **buffer)
{
  if(*buffer == NULL)
  {
    if((*buffer = (buffer_t*)malloc(sizeof(buffer_t))) == NULL)
    {
      return 0;
    } 
  }
  FILE *file;
  if((file = fopen(fileName,"r")) == NULL)
  {
    perror("error opening file");
    return 0;  
  }
  
  if(fseek(file, 0L, SEEK_END) != 0)
  {
    fprintf(stderr,"error seeking end of file");
    return 0;
  }
  long size = ftell(file)-1;
  if(((*buffer)->buf = (char*)malloc(sizeof(char) * (size+/*1*/2))) == NULL)
  {
    perror("asigning buffer:");
    return 0;
  }
  (*buffer)->size = size;
  if(fseek(file, 0L, SEEK_SET) != 0)
  {
    fprintf(stderr,"error seeking begining of file:");
    return 0;
  }
  if(fread((*buffer)->buf,sizeof(char),(*buffer)->size,file) <= 0)
  {
    fprintf(stderr,"Error: file is empty\n");
    return 0;
  }
  (*buffer)->buf[++((*buffer)->size)] = '\0';
  (*buffer)->offset = 0;
  (*buffer)->advance = 0;
  (*buffer)->lineNum = 1;
  (*buffer)->charNum = 1;
  fclose(file);
  //SIGSEGV WHILE CLOSING?? WTF?
  return 1;
}


