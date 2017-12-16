#include <stdio.h>
#include <stdlib.h>  /* for exit, malloc, realloc */
#include <ctype.h>   /* for isspace */
#include <string.h>  /* for strdup */

#include "scanner.h"

/* VERSION 1.2
 *
 * scanner.c - a collection of input routines for C
 *           - written by John C. Lusth
 *           - in general, the functions return what is implied by their names
 *
 *    readInt(FILE *) 
 *      - wrapper for fscanf(fp,"%d")
 *      - returns 0 if end of file, but feof will subsequently return true
 *      - usage example: int x = readInt(stdin);
 *    readReal(FILE *) 
 *      - wrapper for fscanf(fp,"%lf")
 *      - returns a double
 *      - returns 0.0 if read fails due to end of file,
 *        but feof will subsequently return true
 *      - usage example: double x = readReal(stdin);
 *    readChar(FILE *) 
 *      - wrapper for fscanf(fp," %c")
 *      - returns a non-whitespace character
 *      - returns 0 if end of file; feof will subsequently return true
 *      - usage example: char x = readChar(stdin);
 *    readRawChar(FILE *fp) 
 *      - wrapper for fscanf(fp,"%c")
 *      - returns a character, whether whitespace or non-whitespace
 *      - returns 0 if end of file; feof will subsequently return true
 *      - usage example: char x = readRawChar(stdin);
 *    readToken(FILE *fp) 
 *      - safe version of fscanf(fp,"%s")
 *      - returns a malloc'd string
 *        the caller should free the returned string
 *      - returns 0 if end of file; feof will subsequently return true
 *      - usage example: char *x = readToken(stdin);
 *    readString(FILE *fp) 
 *      - reads in a double quoted string
 *      - returns a malloc'd string; the quotation marks are not included
 *        the caller should free the returned string
 *      - returns 0 if end of file; feof will subsequently return true
 *      - usage example: char *x = readString(stdin);
 *    readLine(FILE *fp) 
 *      - reads in a line or remainder of a line
 *      - returns a malloc'd string; the newline is not included
 *        the caller should free the returned string
 *      - returns 0 if end of file; feof will subsequently return true
 *      - usage example: char *x = readLine(stdin);
 *    allocate(size_t size) 
 *      - wrapper for malloc that will generate an out of memory error
 *      - usage example: int *x = (int *) allocate(sizeof(int) * count);
 *    reallocate(void *items,size_t size) 
 *      - wrapper for realloc that will generate an out of memory error
 *      - usage example: x = (int *) reallocate(x,sizeof(int) * count);
 */

static void skipWhiteSpace(FILE *);
static char convertEscapedChar(int);
static void *allocateMsg(size_t,char *);
static void *reallocateMsg(void *,size_t,char *);

/********** public functions **********************/

int
readInt(FILE *fp)
    {
    int x,result;
    result = fscanf(fp,"%d",&x);
    if (result == EOF)
        {
        return 0;
        }
    if (result == 0)
        {
        fprintf(stderr,"SCAN ERROR: attempt to read an integer failed\n");
        fprintf(stderr,"offending character was <%c>\n",fgetc(fp));
        exit(1);
        }
    return x;
    }

double
readReal(FILE *fp)
    {
    int result;
    double x;
    result = fscanf(fp," %lf",&x);
    if (result == EOF)
        {
        return 0.0;
        }
    if (result == 0)
        {
        fprintf(stderr,"SCAN ERROR: attempt to read a real number failed\n");
        fprintf(stderr,"offending character was <%c>\n",fgetc(fp));
        exit(2);
        }
    return x;
    }

char
readChar(FILE *fp)
    {
    int result;
    char x;
    result = fscanf(fp," %c",&x);
    if (result == EOF)
        {
        return -1;
        }
    if (result == 0)
        {
        fprintf(stderr,"SCAN ERROR: attempt to read a non-whitespace character failed\n");
        fprintf(stderr,"offending character was <%c>\n",fgetc(fp));
        exit(2);
        }
    return x;
    }

char
readRawChar(FILE *fp)
    {
    int result;
    char x;
    result = fscanf(fp,"%c",&x);
    if (result == EOF)
        {
        return -1;
        }
    if (result == 0)
        {
        fprintf(stderr,"SCAN ERROR: attempt to read a raw character failed\n");
        fprintf(stderr,"offending character was <%c>\n",fgetc(fp));
        exit(2);
        }
    return x;
    }

char *
readString(FILE *fp)
    {
    int ch,index;
    char *buffer;
    int size = 512;

    /* advance to the double quote */

    skipWhiteSpace(fp);

    ch = fgetc(fp);
    if (ch == EOF) return 0;

    /* allocate the buffer */

    buffer = allocateMsg(size,"readString");

    if (ch != '\"')
        {
        fprintf(stderr,"SCAN ERROR: attempt to read a string failed\n");
        fprintf(stderr,"first character was <%c>\n",ch);
        exit(4);
        }

    /* toss the double quote, skip to the next character */

    ch = fgetc(fp);

    /* initialize the buffer index */

    index = 0;

    /* collect characters until the closing double quote */

    while (ch != '\"')
        {
        if (ch == EOF)
            {
            fprintf(stderr,"SCAN ERROR: attempt to read a string failed\n");
            fprintf(stderr,"no closing double quote\n");
            exit(6);
            }
        if (index > size - 2) 
            {
            ++size;
            buffer = reallocateMsg(buffer,size,"readString");
            }

        if (ch == '\\')
            {
            ch = fgetc(fp);
            if (ch == EOF)
                {
                fprintf(stderr,"SCAN ERROR: attempt to read a string failed\n");
                fprintf(stderr,"escaped character missing\n");
                exit(6);
                }
            buffer[index] = convertEscapedChar(ch);
            }
        else
            buffer[index] = ch;
        ++index;
        ch = fgetc(fp);
        }

    buffer[index] = '\0';

    return buffer;
    }

char *
readToken(FILE *fp)
    {
    int ch,index;
    char *buffer;
    int size = 80;

    skipWhiteSpace(fp);

    ch = fgetc(fp);
    if (ch == EOF) return 0;

    buffer = allocateMsg(size,"readToken");

    index = 0;
    while (!isspace(ch))
        {
        if (ch == EOF) break;
        if (index > size - 2)
            {
            ++size;
            buffer = reallocateMsg(buffer,size,"readToken");
            }
        buffer[index] = ch;
        ++index;
        ch = fgetc(fp);
        }

    /* push back the character that got us out of this loop */

    ungetc(ch,fp);

    if (index > 0)              //there is something in the buffer
        clearerr(fp);           //so force the read to be good

    buffer[index] = '\0';

    return buffer;
    }

char *
readLine(FILE *fp)
    {
    int ch,index;
    char *buffer;
    int size = 512;
    
    ch = fgetc(fp);
    if (ch == EOF) return 0;

    buffer = allocateMsg(size,"readLine");

    index = 0;
    while (ch != '\n')
        {
        if (ch == EOF) break;
        if (index > size - 2)
            {
            ++size;
            buffer = reallocateMsg(buffer,size,"readLine");
            }
        buffer[index] = ch;
        ++index;
        ch = fgetc(fp);
        }


    if (index > 0)              //there is something in the buffer
        clearerr(fp);           //so force the read to be good

    buffer[index] = '\0';

    return buffer;
    }

/********** private functions **********************/

static void
skipWhiteSpace(FILE *fp)
    {
    int ch;

    /* read chars until a non-whitespace character is encountered */

    while ((ch = fgetc(fp)) != EOF && isspace(ch))
        continue;

    /* a non-space character got us out of the loop, so push it back */

    ungetc(ch,fp);
    }

static char
convertEscapedChar(int ch)
    {
    switch (ch)
        {
        case 'n':  return '\n';
        case 't':  return '\t';
        case '"':  return '\"';
        case '\\': return '\\';
        }
    return ch;
    }

void *
allocate(size_t size)
    {
    char *s;
    s = malloc(size);
    if (s == 0)
        {
        fprintf(stderr,"could not allocate string, out of memory\n");
        exit(3);
        }

    return s;
    }

void *
reallocate(void *s,size_t size)
    {
    char *t;
    t = realloc(s,size);
    if (t == 0)
        {
        fprintf(stderr,"could not reallocate string, out of memory\n");
        exit(3);
        }

    return t;
    }


void *
allocateMsg(size_t size,char *where)
    {
    char *s;
    s = malloc(size);
    if (s == 0)
        {
        fprintf(stderr,"%s: could not allocate string, out of memory\n",
            where);
        exit(3);
        }

    return s;
    }

static void *
reallocateMsg(void *s,size_t size,char *where)
    {
    char *t;
    t = realloc(s,size);
    if (t == 0)
        {
        fprintf(stderr,"%s: could not reallocate string, out of memory\n",
            where);
        exit(3);
        }

    return t;
    }

