/* Bryan Martin Tostado
 *  creation 9 march 2014
 *
 *  Description:
 *   
 *
 *  Notes:
 *
 *
 */

#ifndef __FILES__
  #define __FILES__
  

/* TYPES */

/*buffer_t
 *  sructur that encapsulates buffer properties
 */
struct buffer_t
{
  char *buf;
  long size;
  long offset;
  long advance; //the advancement from the offset while we search for a valid token
  long lineNum; //the line number  into the buffer (sourcefile)
  long charNum; //the char number into the line
};
typedef struct buffer_t buffer_t;


/* FUNCTIONS */

/* fill_buff
 *   Description:
 *    reads fileName and copys it to buffer
 * 
 *   Parameters and type:
 *    fileName (in) : name of the input file
 *    buffer   (out): buffer to be filled in
 */        
int fill_buff(char *fileName, buffer_t **buffer);









  #endif /* __FILES__ */
