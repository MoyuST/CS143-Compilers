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
#include <string>

int _string_char_cnt{0};
int _lineno_increase{0};

%}


%x  PROCESSING_STRING
%x  PROCESSING_STRING_ERROR
%x  LINE_COMMENTS
%x  NESTED_COMMENTS
/*
 * Define names for regular expressions here.
 */

DIGITS          [0-9]+
BOOL_TRUE       t[Rr][Uu][Ee]
BOOL_FALSE      f[Aa][Ll][Ss][Ee]
DARROW          =>
LE              <=
ASSIGN          <-
CLASS           [Cc][Ll][Aa][Ss][Ss]
ELSE            [Ee][Ll][Ss][Ee]
FI              [Ff][Ii]
IF              [Ii][Ff]
IN              [Ii][Nn]
INHERITS        [Ii][Nn][Hh][Ee][Rr][Ii][Tt][Ss]
LET             [Ll][Ee][Tt]
LOOP            [Ll][Oo][Oo][Pp]
POOL            [Pp][Oo][Oo][Ll]
THEN            [Tt][Hh][Ee][Nn]
WHILE           [Ww][Hh][Ii][Ll][Ee]
CASE            [Cc][Aa][Ss][Ee]
ESAC            [Ee][Ss][Aa][Cc]
OF              [Oo][Ff]
NEW             [Nn][Ee][Ww]
ISVOID          [Ii][Ss][Vv][Oo][Ii][Dd]
NOT             [Nn][Oo][Tt]
CLASS_TYPEID    [A-Z][a-zA-Z0-9_]*
OBJECT_ID       [a-z][a-zA-Z0-9_]*
SINGLE_CHAR_OPERATOR        [+-/\*=:\;\{\}\(\)\@\?\.\<~]
WHITESPACE_EXCEPT_NEWLINE   [ \f\r\t\v]

%%

<*>.* {
        curr_lineno+=_lineno_increase;
        _lineno_increase=0;
        REJECT;
      }

 /*
  * Line comments
  */ 
<INITIAL>-- {BEGIN(LINE_COMMENTS);}
<LINE_COMMENTS>\n {BEGIN(INITIAL); _lineno_increase++;}
<LINE_COMMENTS><<EOF>> {BEGIN(INITIAL);curr_lineno+=_lineno_increase;_lineno_increase=0;}
<LINE_COMMENTS>.  /* eating up characters in this line */

 /*
  *  Nested comments
  */
<INITIAL>\(\* {BEGIN(NESTED_COMMENTS);}
<NESTED_COMMENTS>\n {_lineno_increase++;}
<NESTED_COMMENTS><<EOF>> {
                  cool_yylval.error_msg=stringtable.add_string("EOF in comment")->get_string();
                  BEGIN(PROCESSING_STRING_ERROR);
                  curr_lineno+=_lineno_increase;_lineno_increase=0;
                  return ERROR;
                }
<NESTED_COMMENTS>\*\) {BEGIN(INITIAL);}
<NESTED_COMMENTS>. /* eating up characters in nested comments */
<INITIAL>\*\) {
                cool_yylval.error_msg=stringtable.add_string("Unmacthed *)")->get_string();
                BEGIN(PROCESSING_STRING_ERROR);
                return ERROR;
              }
 
 /*
  *  The multiple-character operators.
  */
<INITIAL>{DARROW}		{ return (DARROW); }
<INITIAL>{LE}       { return (LE); }
<INITIAL>{ASSIGN}   { return (ASSIGN); }

 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */

<INITIAL>{BOOL_TRUE}    { cool_yylval.boolean=true;return BOOL_CONST;}
<INITIAL>{BOOL_FALSE}   { cool_yylval.boolean=false;return BOOL_CONST;}
<INITIAL>{CLASS}        { return CLASS;}
<INITIAL>{ELSE}         { return ELSE;}
<INITIAL>{IF}           { return IF;}
<INITIAL>{FI}           { return FI;}
<INITIAL>{IN}           { return IN;}
<INITIAL>{INHERITS}     { return INHERITS;}
<INITIAL>{LET}          { return LET;}
<INITIAL>{LOOP}         { return LOOP;}
<INITIAL>{POOL}         { return POOL;}
<INITIAL>{THEN}         { return THEN;}
<INITIAL>{WHILE}        { return WHILE;}
<INITIAL>{CASE}         { return CASE;}
<INITIAL>{ESAC}         { return ESAC;}
<INITIAL>{OF}           { return OF;}
<INITIAL>{NEW}          { return NEW;}
<INITIAL>{ISVOID}       { return ISVOID;}
<INITIAL>{NOT}          { return NOT;}

 /*
  *  The single-character operators.
  */
<INITIAL>{SINGLE_CHAR_OPERATOR} {return yytext[0];}

<INITIAL>{DIGITS} {
        cool_yylval.symbol = inttable.add_string(yytext, yyleng);
        return INT_CONST;
      }

<INITIAL>{CLASS_TYPEID} {
        cool_yylval.symbol = inttable.add_string(yytext, yyleng);
        return TYPEID;
      }

<INITIAL>{OBJECT_ID} {
        cool_yylval.symbol = inttable.add_string(yytext, yyleng);
        return OBJECTID;
      }

 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */

<INITIAL>\" {
              BEGIN(PROCESSING_STRING);
              string_buf_ptr=string_buf;
              _string_char_cnt=0;
            }

<PROCESSING_STRING><<EOF>> {
              BEGIN(PROCESSING_STRING_ERROR);
              cool_yylval.error_msg=stringtable.add_string("String contains EOF")->get_string();
              curr_lineno+=_lineno_increase;_lineno_increase=0;
              return ERROR;
            }

<PROCESSING_STRING>[\0\n] {
              if(yytext[0]=='\0'){
                BEGIN(PROCESSING_STRING_ERROR);
                cool_yylval.error_msg=stringtable.add_string("String contains null character")->get_string();
              }
              else if(yytext[0]=='\n'){
                BEGIN(INITIAL);
                cool_yylval.error_msg=stringtable.add_string("Unterminated string constant")->get_string();
                if(yytext[0]=='\n'){_lineno_increase++;}
              }
              else{
                BEGIN(PROCESSING_STRING_ERROR);
                cool_yylval.error_msg=stringtable.add_string("String contains EOF")->get_string();
                curr_lineno+=_lineno_increase;_lineno_increase=0;
              }
              return ERROR;
            }

<PROCESSING_STRING>\" {
                        BEGIN(INITIAL);
                        *string_buf_ptr='\0';
                        cool_yylval.symbol = inttable.add_string((char*) string_buf, _string_char_cnt);
                        return STR_CONST;
                      }

<PROCESSING_STRING>\\[btnf\n] {
                    _string_char_cnt++;
                    if(_string_char_cnt>=MAX_STR_CONST){
                      cool_yylval.error_msg=stringtable.add_string("String constant too long")->get_string();
                      BEGIN(PROCESSING_STRING_ERROR);
                      return ERROR;
                    }
                    if(yytext[1]=='b'){
                      *string_buf_ptr++ = '\b';
                    }
                    else if(yytext[1]=='t'){
                      *string_buf_ptr++ = '\t';
                    }
                    else if(yytext[1]=='n'){
                      *string_buf_ptr++ = '\n';
                    }
                    else if(yytext[1]=='f'){
                      *string_buf_ptr++ = '\f';
                    }
                    else{
                      *string_buf_ptr++ = '\n';
                      _lineno_increase++;
                    }
                  }

<PROCESSING_STRING>\\[a-zA-Z]  {*string_buf_ptr++ = yytext[1];}
<PROCESSING_STRING>.  {_string_char_cnt++;*string_buf_ptr++ = yytext[0];}

<PROCESSING_STRING_ERROR>[\n\"] {BEGIN(INITIAL);_lineno_increase++;}
<PROCESSING_STRING_ERROR>.  /* eating up remainning characters in remainning string */

 /* 
 * Skipping all whitespace
 */

<INITIAL>\n {_lineno_increase++;}
<INITIAL>{WHITESPACE_EXCEPT_NEWLINE}+ /* eating up all whitespace */
<INITIAL>.  {
              cool_yylval.error_msg=stringtable.add_string(yytext, yyleng)->get_string();
              BEGIN(INITIAL);
              return ERROR;
            }

%%
