/*============================================================================
FILE  read_ifs.c

MEMBER OF process cmpp

Copyright 1991
Georgia Tech Research Corporation
Atlanta, Georgia 30332
All Rights Reserved

PROJECT A-8503

AUTHORS

    9/12/91  Bill Kuhn and Steve Tynor

MODIFICATIONS

    <date> <person name> <nature of modifications>

SUMMARY

    This file contains top-level functions used in reading information
    from the ifspec.ifs file and building an internal data structure that
    holds the information.  Most of the work in parsing of the
    ifspec.ifs file and in building the structure is handled by
    the UNIX 'lex' and 'yacc' utilities.  This processing is begun
    at the call to yyparse() in read_ifs_table() below.  See also files:

        ifs_lex.l
        ifs_yacc.y

INTERFACES

    read_ifs_file()
    yywrap()
    yyerror()

REFERENCED FILES

    None.

NON-STANDARD FEATURES

    None.

============================================================================*/

#include <assert.h>
#include <stdio.h>
#include  "cmpp.h"

extern char *prog_name;
extern int ifs_yyparse ();
extern void ifs_yyrestart (FILE*);

void  *malloc(unsigned size);

static Status_t read_ifs_table(FILE *fp, int mode, Ifs_Table_t *ifs_table);

char *current_filename;

/* *********************************************************************** */


/*
  NOTE
  
  The following function may be called either by cmpp -ifs or cmpp -lst with
  mode set to GET_IFS_TABLE or GET_IFS_NAME respectively.
  */


/*
read_ifs_file

Function read_ifs_file() opens the Interface Specification file
(ifspec.ifs) for read access and calls read_ifs_table() with the
assigned file pointer to read and parse the file.  Upon return
from read_ifs_table(), the file is closed.
*/



Status_t  read_ifs_file(
    char        *filename,   /* File to read */
    int         mode,        /* Get names only or get everything? */
    Ifs_Table_t *ifs_table)  /* Table to put info in */
{
   
   FILE     *fp;                     /* Ifs file pointer */
   
   char     msg[MAX_PATH_LEN+257];   /* space for an error message */
   
   Status_t status;                  /* returned status from function */
   
   
   /* Open the ifs file for read access */
   
   fp = fopen(filename, "r");
   
   if(fp == NULL) {
      perror (prog_name);
      sprintf(msg, "ERROR - File not found: %s", filename);
      print_error(msg);
      return(ERROR);
   }

   current_filename = filename;
   
   /* Get the stuff from the file into the ifs_table struct */
   
   status = read_ifs_table(fp, mode, ifs_table);
   
   /* Close file and return */
   
   fclose(fp);
   
   return(status);
   
}




/* *********************************************************************** */


/*
read_ifs_table

Function read_ifs_table() calls yyparse() to read and parse the
Interface Specification file contents and place the information
into an internal data structure.  Function yyparse() is
automatically generated by UNIX lex/yacc.
*/



static Status_t read_ifs_table(
    FILE        *fp,          /* File to read from */
    int         mode,         /* Get names only or get everything? */
    Ifs_Table_t *ifs_table)   /* Table to put info in */
{

   extern FILE *ifs_yyin;
   extern Ifs_Table_t *parser_ifs_table;
   extern Boolean_t parser_just_names;
   extern int ifs_yylineno;
   
   assert (ifs_table);
   assert (fp);

   ifs_yylineno = 1;
   ifs_yyin = fp;
   parser_just_names = (mode == GET_IFS_NAME);
   parser_ifs_table = ifs_table;

   if (ifs_yyparse()) {
      print_error ("Error parsing interface specification file");
      ifs_yyrestart(NULL);
      return ERROR;
   }
   ifs_yyrestart(NULL);
   return OK;
}

/*---------------------------------------------------------------------------*/
int
ifs_yyerror (char *str)
{
   extern int ifs_yylineno;
   extern char *ifs_yytext;
   
   fprintf (stderr, "%s: Error: \"%s\": line %d (near \'%s\'):\n\t%s.\n",
	    prog_name, current_filename, ifs_yylineno, ifs_yytext, str);

   return 0;
}

