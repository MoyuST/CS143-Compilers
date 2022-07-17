/*
 *  The scanner definition for COOL.
 */

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */
%{
#include <cool-parse.h>
#include <stringtab.h>
#include <utilities.h>

/* The compiler assumes these identifiers. */
#define yylval cool_yylval
#define yylex  cool_yylex

/* Max size of string constants */
#define MAX_STR_CONST 1025
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;

extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE cool_yylval;

/*
 *  Add Your own definitions here
 */

%}



%option stack
%x CLASS_EXPECTING_TYPEID
%x CLASS_TYPEID_DEFINED
%x CLASS_EXPECTING_PARENT_TYPEID
%x CLASS_PARENT_TYPEID_DEFINED
%x FEATURE_EXPECTING_LEFT_BRACE
%x FEATURE_LEFT_BRACE
%x FEATURE_EXPECTING_COLON
%x FEATURE_EXPECTING_RETRUN_TYPEID
%x FEATURE_DEFINED
%x FEATURE_METHOD_EXPECTING_LEFT_BRACE
%x FORMAL_EXPECTING_COLON
%x FORMAL_EXPECTING_TYPEID
%x FORMAL_EXPECTING_COMMA
%x FORMAL_DEFINED
%x FORMAL_EXPECTING_NEXT_ONE



/*
 * Define names for regular expressions here.
 */

DARROW          =>
CLASSTYPEID     [A-Z][a-zA-Z0-9_]*
FEATURETYPEID   [a-z][a-zA-Z0-9_]*
FORMALID        [a-z]([[a-z]]|"_")*
LE              <=
ASSIGN          <-
WHITESPACE      [" "\n\f\r\t\v]+




%%

<INITIAL>"class"  {
                    cool_yylex = CLASS;
                    BEGIN(CLASS_EXPECTING_TYPEID);
                  }

<CLASS_EXPECTING_TYPEID, CLASS_EXPECTING_PARENT_TYPEID, FORMAL_EXPECTING_TYPEID, FEATURE_EXPECTING_RETRUN_TYPEID>{CLASSTYPEID}
                          {
                            cool_yylex = TYPEID;
                            cool_yylval.symbol->str = yytext;
                            cool_yylval.symbol->len = yyleng;

                            if(YY_START==CLASS_EXPECTING_TYPEID){
                              BEGIN(CLASS_TYPEID_DEFINED);
                            }
                            else if(YY_START==FORMAL_EXPECTING_TYPEID){
                              BEGIN(FORMAL_DEFINED);
                            }
                            else if(YY_START==FEATURE_EXPECTING_RETRUN_TYPEID){
                              if(yy_top_state()==FEATURE_LEFT_BRACE){
                                yy_pop_state();
                                BEGIN(FEATURE_METHOD_EXPECTING_LEFT_BRACE);
                              }
                              else{

                              }
                              BEGIN(FEATURE_DEFINED);
                            }
                            else{
                              BEGIN(CLASS_PARENT_TYPEID_DEFINED);
                            }

                            return cool_yylex;
                          }

<CLASS_TYPEID_DEFINED>"inherits"  {
                                cool_yylex = INHERITS;
                                BEGIN(CLASS_EXPECTING_PARENT_TYPEID);
                                return cool_yylex;
                              }

<CLASS_TYPEID_DEFINED, CLASS_PARENT_TYPEID_DEFINED>"{"
                                  {
                                    cool_yylex = "{";
                                    yy_push_state(CLASS_BODY_LEFT_BRACE);
                                    return cool_yylex;
                                  }

<CLASS_BODY_LEFT_BRACE>{FEATURETYPEID} 
                                  {
                                    cool_yylex = TYPEID;
                                    cool_yylval.symbol->str = yytext;
                                    cool_yylval.symbol->len = yyleng;
                                    BEGIN(FEATURE_EXPECTING_LEFT_BRACE);
                                    return cool_yylex;
                                  }

<FEATURE_EXPECTING_LEFT_BRACE>"("
                    {
                      cool_yylex = "(";
                      yy_push_state(FEATURE_LEFT_BRACE);
                      return cool_yylex;
                    }

<FEATURE_LEFT_BRACE, FORMAL_EXPECTING_NEXT_ONE>{FORMALID}
                    {
                      cool_yylex = OBJECTID;
                      cool_yylval.symbol->str = yytext;
                      cool_yylval.symbol->len = yyleng;
                      BEGIN(FORMAL_EXPECTING_COLON);
                      return cool_yylex;
                    }

<FORMAL_EXPECTING_COLON>":"
                    {
                      cool_yylex = ":";
                      BEGIN(FORMAL_EXPECTING_TYPEID);
                      return cool_yylex;
                    }

<FORMAL_DEFINED>","
                    {
                      cool_yylex = ",";
                      BEGIN(FORMAL_EXPECTING_NEXT_ONE);
                      return cool_yylex;
                    }

<FEATURE_LEFT_BRACE, FORMAL_DEFINED>")"
                    {
                      cool_yylex = ")";
                      BEGIN(FEATURE_EXPECTING_COLON);
                      return cool_yylex;
                    }

<FEATURE_EXPECTING_COLON>":"
                    {
                      cool_yylex = ":";
                      BEGIN(FEATURE_EXPECTING_RETRUN_TYPEID);
                      return cool_yylex;
                    }

<FEATURE_METHOD_EXPECTING_LEFT_BRACE>"{"
                    {
                      cool_yylex = "{";
                      
                      BEGIN(FEATURE_EXPECTING_RETRUN_TYPEID);
                      return cool_yylex;
                    }



<*>{WHITESPACE}   /*skipping all whitespace*/

 /*
  *  Nested comments
  */


 /*
  *  The multiple-character operators.
  */
{DARROW}		{ return (DARROW); }

 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */


 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */


%%









