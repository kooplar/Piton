/*Bryan Martin Tostado
 *
 * creation: 9 march 2014
 *
 * Description:
 *   entry point for compiler
 *
 * Notes:
 *
 *
 */

#include "include/lexer.h"
#include "./include/files.h"
#include "./include/syntax.h"
#include "./include/codegen.h"
#include "./include/errors.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  if(argc < 2)
  {
    printf("missing file\n");
    return 0;
  }

  buffer_t *buffer;
  if(fill_buff(argv[1],&buffer) == 0)
  {
    exit(1);
  }
  registerBuff(buffer);
/*
  token_t tok;
  do{
  tok = nexTok();
  }while(tok.lexeme != NULL);
*/
  if( syntax() == 0)
  {
    writePL0();
    cleanExit(SUCCESS,"");
  }

  else
    cleanExit(FAILURE,"Exited with errors");
  return 0;
}






