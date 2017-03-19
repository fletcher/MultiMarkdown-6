/*
** 2000-05-29
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Driver template for the LEMON parser generator.
**
** The "lemon" program processes an LALR(1) input grammar file, then uses
** this template to construct a parser.  The "lemon" program inserts text
** at each "%%" line.  Also, any "P-a-r-s-e" identifer prefix (without the
** interstitial "-" characters) contained in this template is changed into
** the value of the %name directive from the grammar.  Otherwise, the content
** of this template is copied straight through into the generate parser
** source file.
**
** The following is the concatenation of all %include directives from the
** input grammar file:
*/
#include <stdio.h>
/************ Begin %include sections from the grammar ************************/

	#include <assert.h>
	#include <stdio.h>
	#include <stdlib.h>

	#include "libMultiMarkdown.h"
	#include "mmd.h"
	#include "parser.h"
	#include "token.h"
/**************** End of %include directives **********************************/
/* These constants specify the various numeric values for terminal symbols
** in a format understandable to "makeheaders".  This section is blank unless
** "lemon" is run with the "-m" command-line option.
***************** Begin makeheaders token definitions *************************/
/**************** End makeheaders token definitions ***************************/

/* The next sections is a series of control #defines.
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used to store the integer codes
**                       that represent terminal and non-terminal symbols.
**                       "unsigned char" is used if there are fewer than
**                       256 symbols.  Larger types otherwise.
**    YYNOCODE           is a number of type YYCODETYPE that is not used for
**                       any terminal or nonterminal symbol.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       (also known as: "terminal symbols") have fall-back
**                       values which should be used if the original symbol
**                       would not parse.  This permits keywords to sometimes
**                       be used as identifiers, for example.
**    YYACTIONTYPE       is the data type used for "action codes" - numbers
**                       that indicate what to do in response to the next
**                       token.
**    ParseTOKENTYPE     is the data type used for minor type for terminal
**                       symbols.  Background: A "minor type" is a semantic
**                       value associated with a terminal or non-terminal
**                       symbols.  For example, for an "ID" terminal symbol,
**                       the minor type might be the name of the identifier.
**                       Each non-terminal can have a different minor type.
**                       Terminal symbols all have the same minor type, though.
**                       This macros defines the minor type for terminal 
**                       symbols.
**    YYMINORTYPE        is the data type used for all minor types.
**                       This is typically a union of many types, one of
**                       which is ParseTOKENTYPE.  The entry in the union
**                       for terminal symbols is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    ParseARG_SDECL     A static variable declaration for the %extra_argument
**    ParseARG_PDECL     A parameter declaration for the %extra_argument
**    ParseARG_STORE     Code to store %extra_argument into yypParser
**    ParseARG_FETCH     Code to extract %extra_argument from yypParser
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YY_MAX_SHIFT       Maximum value for shift actions
**    YY_MIN_SHIFTREDUCE Minimum value for shift-reduce actions
**    YY_MAX_SHIFTREDUCE Maximum value for shift-reduce actions
**    YY_MIN_REDUCE      Maximum value for reduce actions
**    YY_ERROR_ACTION    The yy_action[] code for syntax error
**    YY_ACCEPT_ACTION   The yy_action[] code for accept
**    YY_NO_ACTION       The yy_action[] code for no-op
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/************* Begin control #defines *****************************************/
#define YYCODETYPE unsigned char
#define YYNOCODE 85
#define YYACTIONTYPE unsigned short int
#define ParseTOKENTYPE  token * 
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL  mmd_engine * engine ;
#define ParseARG_PDECL , mmd_engine * engine 
#define ParseARG_FETCH  mmd_engine * engine  = yypParser->engine 
#define ParseARG_STORE yypParser->engine  = engine 
#define YYFALLBACK 1
#define YYNSTATE             44
#define YYNRULE              136
#define YY_MAX_SHIFT         43
#define YY_MIN_SHIFTREDUCE   138
#define YY_MAX_SHIFTREDUCE   273
#define YY_MIN_REDUCE        274
#define YY_MAX_REDUCE        409
#define YY_ERROR_ACTION      410
#define YY_ACCEPT_ACTION     411
#define YY_NO_ACTION         412
/************* End control #defines *******************************************/

/* Define the yytestcase() macro to be a no-op if is not already defined
** otherwise.
**
** Applications can choose to define yytestcase() in the %include section
** to a macro that can assist in verifying code coverage.  For production
** code the yytestcase() macro should be turned off.  But it is useful
** for testing.
*/
#ifndef yytestcase
# define yytestcase(X)
#endif


/* Next are the tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N <= YY_MAX_SHIFT             Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   N between YY_MIN_SHIFTREDUCE       Shift to an arbitrary state then
**     and YY_MAX_SHIFTREDUCE           reduce by rule N-YY_MIN_SHIFTREDUCE.
**
**   N between YY_MIN_REDUCE            Reduce by rule N-YY_MIN_REDUCE
**     and YY_MAX_REDUCE
**
**   N == YY_ERROR_ACTION               A syntax error has occurred.
**
**   N == YY_ACCEPT_ACTION              The parser accepts its input.
**
**   N == YY_NO_ACTION                  No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as either:
**
**    (A)   N = yy_action[ yy_shift_ofst[S] + X ]
**    (B)   N = yy_default[S]
**
** The (A) formula is preferred.  The B formula is used instead if:
**    (1)  The yy_shift_ofst[S]+X value is out of range, or
**    (2)  yy_lookahead[yy_shift_ofst[S]+X] is not equal to X, or
**    (3)  yy_shift_ofst[S] equal YY_SHIFT_USE_DFLT.
** (Implementation note: YY_SHIFT_USE_DFLT is chosen so that
** YY_SHIFT_USE_DFLT+X will be out of range for all possible lookaheads X.
** Hence only tests (1) and (2) need to be evaluated.)
**
** The formulas above are for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
**
*********** Begin parsing tables **********************************************/
#define YY_ACTTAB_COUNT (266)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   411,    1,  140,   32,  150,  151,  152,  153,  154,  155,
 /*    10 */    43,  157,   30,   29,   40,   38,   31,   14,  164,  165,
 /*    20 */   166,   42,  219,   13,   13,   29,  268,  272,   37,   37,
 /*    30 */    34,   34,   27,  225,   26,   25,   28,   40,   38,  265,
 /*    40 */     8,  242,   41,  194,   15,   15,  274,  147,  167,   20,
 /*    50 */    20,   16,  228,  229,  268,  272,  256,  141,  142,  143,
 /*    60 */   144,  145,  146,  233,    7,    6,   17,    4,    3,    2,
 /*    70 */    18,  264,   28,  148,    5,  242,  244,  247,  250,  245,
 /*    80 */   248,  251,  263,  147,  167,   24,   24,   16,  228,  229,
 /*    90 */   268,  272,  256,  141,  142,  143,  144,  145,  146,  233,
 /*   100 */     7,    6,   17,    4,    3,    2,   18,   33,   33,  148,
 /*   110 */     5,  242,  244,  247,  250,  245,  248,  251,  263,  139,
 /*   120 */    32,  150,  151,  152,  153,  154,  155,   43,  157,   30,
 /*   130 */    29,   40,   38,   31,   14,  164,  165,  166,  254,  255,
 /*   140 */   252,   23,   29,  268,  272,   35,   35,   34,   34,   27,
 /*   150 */    42,   26,   25,  222,   40,   38,  190,    8,  187,   41,
 /*   160 */   200,   15,   15,   28,  220,  253,  195,  197,  193,  196,
 /*   170 */   198,  254,  255,  252,   10,   10,   19,   19,   28,  199,
 /*   180 */   176,  176,    9,   42,    9,   13,   13,  214,  215,   10,
 /*   190 */    10,   19,   19,  228,  229,  175,  175,   28,  253,  191,
 /*   200 */   188,  189,  192,  254,  255,  252,   28,  173,   10,   10,
 /*   210 */    19,   19,  167,    5,  174,  174,  183,   10,   10,   19,
 /*   220 */    19,  228,  229,  181,  181,   28,  259,   36,   36,  257,
 /*   230 */   253,  184,  185,  186,  225,  258,   11,   11,   21,   21,
 /*   240 */    28,  208,  235,   28,  201,   39,   39,  218,  217,  212,
 /*   250 */     5,   12,   12,   22,   22,  234,  203,  168,  180,  242,
 /*   260 */     6,    7,  218,  183,  207,  202,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    38,   39,   40,   41,   42,   43,   44,   45,   46,   47,
 /*    10 */    48,   49,   50,   51,   52,   53,   54,   55,   56,   57,
 /*    20 */    58,   80,   81,   82,   83,   63,    8,    9,   68,   69,
 /*    30 */    68,   69,   70,    4,   72,   73,   48,   75,   76,    4,
 /*    40 */    78,   29,   80,   71,   82,   83,    0,    1,   60,   61,
 /*    50 */    62,    5,    6,    7,    8,    9,   10,   11,   12,   13,
 /*    60 */    14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /*    70 */    24,   36,   48,   27,   28,   29,   30,   31,   32,   33,
 /*    80 */    34,   35,   36,    1,   60,   61,   62,    5,    6,    7,
 /*    90 */     8,    9,   10,   11,   12,   13,   14,   15,   16,   17,
 /*   100 */    18,   19,   20,   21,   22,   23,   24,   59,   60,   27,
 /*   110 */    28,   29,   30,   31,   32,   33,   34,   35,   36,   40,
 /*   120 */    41,   42,   43,   44,   45,   46,   47,   48,   49,   50,
 /*   130 */    51,   52,   53,   54,   55,   56,   57,   58,    2,    3,
 /*   140 */     4,   63,   63,    8,    9,   59,   60,   68,   69,   70,
 /*   150 */    80,   72,   73,   83,   75,   76,   71,   78,   71,   80,
 /*   160 */    63,   82,   83,   48,   29,   29,   30,   31,   32,   33,
 /*   170 */    34,    2,    3,    4,   59,   60,   61,   62,   48,   74,
 /*   180 */    65,   66,   79,   80,   81,   82,   83,    2,    3,   59,
 /*   190 */    60,   61,   62,    6,    7,   65,   66,   48,   29,   30,
 /*   200 */    31,   32,   33,    2,    3,    4,   48,   67,   59,   60,
 /*   210 */    61,   62,   60,   28,   65,   66,   29,   59,   60,   61,
 /*   220 */    62,    6,    7,   65,   66,   48,    1,   59,   60,    4,
 /*   230 */    29,   30,   31,   32,    4,   10,   59,   60,   61,   62,
 /*   240 */    48,   64,    4,   48,   29,   59,   60,    8,    9,   77,
 /*   250 */    28,   59,   60,   61,   62,   17,   64,   62,   69,   29,
 /*   260 */    19,   18,    8,   29,   76,   75,
};
#define YY_SHIFT_USE_DFLT (266)
#define YY_SHIFT_COUNT    (43)
#define YY_SHIFT_MIN      (0)
#define YY_SHIFT_MAX      (254)
static const unsigned short int yy_shift_ofst[] = {
 /*     0 */    82,   46,  230,  230,  230,  230,  230,  230,   18,   18,
 /*    10 */   230,  230,  230,  135,  185,   18,   29,   29,   29,   12,
 /*    20 */    12,   12,   12,   29,   12,  136,  169,  201,  187,  215,
 /*    30 */   225,   35,  238,   29,  222,   29,   29,  222,  241,   29,
 /*    40 */   243,  239,  254,  234,
};
#define YY_REDUCE_USE_DFLT (-60)
#define YY_REDUCE_COUNT (40)
#define YY_REDUCE_MIN   (-59)
#define YY_REDUCE_MAX   (195)
static const short yy_reduce_ofst[] = {
 /*     0 */   -38,   79,  115,  130,  149,  158,  177,  192,  103,  -59,
 /*    10 */   -12,   24,   24,   70,  -40,   70,   48,   86,  168,  195,
 /*    20 */   195,  195,  195,  186,  195,  -28,   85,   87,   78,   97,
 /*    30 */   105,  172,  140,  152,  189,  152,  152,  189,  188,  152,
 /*    40 */   190,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   410,  410,  374,  373,  372,  318,  347,  342,  403,  352,
 /*    10 */   366,  345,  340,  357,  299,  359,  402,  376,  375,  368,
 /*    20 */   308,  346,  341,  306,  307,  385,  382,  379,  363,  295,
 /*    30 */   294,  298,  285,  349,  409,  314,  313,  315,  297,  305,
 /*    40 */   296,  407,  407,  292,
};
/********** End of lemon-generated parsing tables *****************************/

/* The next table maps tokens (terminal symbols) into fallback tokens.  
** If a construct like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
**
** This feature can be used, for example, to cause some keywords in a language
** to revert to identifiers if they keyword does not apply in the context where
** it appears.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
    0,  /*          $ => nothing */
    0,  /*    LINE_HR => nothing */
    1,  /* LINE_SETEXT_1 => LINE_HR */
    1,  /* LINE_SETEXT_2 => LINE_HR */
    0,  /* LINE_CONTINUATION => nothing */
    4,  /* LINE_PLAIN => LINE_CONTINUATION */
    4,  /* LINE_INDENTED_TAB => LINE_CONTINUATION */
    4,  /* LINE_INDENTED_SPACE => LINE_CONTINUATION */
    4,  /* LINE_TABLE => LINE_CONTINUATION */
    4,  /* LINE_TABLE_SEPARATOR => LINE_CONTINUATION */
    0,  /*  LINE_HTML => nothing */
   10,  /* LINE_ATX_1 => LINE_HTML */
   10,  /* LINE_ATX_2 => LINE_HTML */
   10,  /* LINE_ATX_3 => LINE_HTML */
   10,  /* LINE_ATX_4 => LINE_HTML */
   10,  /* LINE_ATX_5 => LINE_HTML */
   10,  /* LINE_ATX_6 => LINE_HTML */
   10,  /* LINE_BLOCKQUOTE => LINE_HTML */
   10,  /* LINE_LIST_BULLETED => LINE_HTML */
   10,  /* LINE_LIST_ENUMERATED => LINE_HTML */
   10,  /* LINE_DEF_ABBREVIATION => LINE_HTML */
   10,  /* LINE_DEF_CITATION => LINE_HTML */
   10,  /* LINE_DEF_FOOTNOTE => LINE_HTML */
   10,  /* LINE_DEF_GLOSSARY => LINE_HTML */
   10,  /* LINE_DEF_LINK => LINE_HTML */
   10,  /* LINE_FENCE_BACKTICK => LINE_HTML */
   10,  /* LINE_FENCE_BACKTICK_START => LINE_HTML */
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
**
** After the "shift" half of a SHIFTREDUCE action, the stateno field
** actually contains the reduce action for the second half of the
** SHIFTREDUCE.
*/
struct yyStackEntry {
  YYACTIONTYPE stateno;  /* The state-number, or reduce action in SHIFTREDUCE */
  YYCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  YYMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  yyStackEntry *yytos;          /* Pointer to top element of the stack */
#ifdef YYTRACKMAXSTACKDEPTH
  int yyhwm;                    /* High-water mark of the stack */
#endif
#ifndef YYNOERRORRECOVERY
  int yyerrcnt;                 /* Shifts left before out of the error */
#endif
  ParseARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
  yyStackEntry yystk0;          /* First stack entry */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/* 
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL 
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void ParseTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = { 
  "$",             "LINE_HR",       "LINE_SETEXT_1",  "LINE_SETEXT_2",
  "LINE_CONTINUATION",  "LINE_PLAIN",    "LINE_INDENTED_TAB",  "LINE_INDENTED_SPACE",
  "LINE_TABLE",    "LINE_TABLE_SEPARATOR",  "LINE_HTML",     "LINE_ATX_1",  
  "LINE_ATX_2",    "LINE_ATX_3",    "LINE_ATX_4",    "LINE_ATX_5",  
  "LINE_ATX_6",    "LINE_BLOCKQUOTE",  "LINE_LIST_BULLETED",  "LINE_LIST_ENUMERATED",
  "LINE_DEF_ABBREVIATION",  "LINE_DEF_CITATION",  "LINE_DEF_FOOTNOTE",  "LINE_DEF_GLOSSARY",
  "LINE_DEF_LINK",  "LINE_FENCE_BACKTICK",  "LINE_FENCE_BACKTICK_START",  "LINE_TOC",    
  "LINE_DEFINITION",  "LINE_EMPTY",    "LINE_FENCE_BACKTICK_3",  "LINE_FENCE_BACKTICK_4",
  "LINE_FENCE_BACKTICK_5",  "LINE_FENCE_BACKTICK_START_3",  "LINE_FENCE_BACKTICK_START_4",  "LINE_FENCE_BACKTICK_START_5",
  "LINE_META",     "error",         "doc",           "blocks",      
  "block",         "blockquote",    "def_abbreviation",  "def_citation",
  "def_footnote",  "def_glossary",  "def_link",      "definition_block",
  "empty",         "fenced_block",  "html_block",    "indented_code",
  "list_bullet",   "list_enum",     "meta_block",    "para",        
  "setext_1",      "setext_2",      "table",         "chunk",       
  "chunk_line",    "nested_chunks",  "nested_chunk",  "indented_line",
  "ext_chunk",     "opt_ext_chunk",  "tail",          "quote_line",  
  "defs",          "def",           "fenced_3",      "fenced_line", 
  "fenced_4",      "fenced_5",      "html_line",     "item_bullet", 
  "item_enum",     "meta_line",     "table_header",  "table_body",  
  "header_rows",   "table_section",  "all_rows",      "row",         
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "doc ::= blocks",
 /*   1 */ "blocks ::= blocks block",
 /*   2 */ "blocks ::= block",
 /*   3 */ "block ::= LINE_ATX_1",
 /*   4 */ "block ::= LINE_ATX_2",
 /*   5 */ "block ::= LINE_ATX_3",
 /*   6 */ "block ::= LINE_ATX_4",
 /*   7 */ "block ::= LINE_ATX_5",
 /*   8 */ "block ::= LINE_ATX_6",
 /*   9 */ "block ::= LINE_HR",
 /*  10 */ "block ::= LINE_TOC",
 /*  11 */ "block ::= blockquote",
 /*  12 */ "block ::= def_abbreviation",
 /*  13 */ "block ::= def_citation",
 /*  14 */ "block ::= def_footnote",
 /*  15 */ "block ::= def_glossary",
 /*  16 */ "block ::= def_link",
 /*  17 */ "block ::= definition_block",
 /*  18 */ "block ::= empty",
 /*  19 */ "block ::= fenced_block",
 /*  20 */ "block ::= html_block",
 /*  21 */ "block ::= indented_code",
 /*  22 */ "block ::= list_bullet",
 /*  23 */ "block ::= list_enum",
 /*  24 */ "block ::= meta_block",
 /*  25 */ "block ::= para",
 /*  26 */ "block ::= setext_1",
 /*  27 */ "block ::= setext_2",
 /*  28 */ "block ::= table",
 /*  29 */ "chunk ::= chunk chunk_line",
 /*  30 */ "nested_chunks ::= nested_chunks nested_chunk",
 /*  31 */ "nested_chunk ::= empty indented_line chunk",
 /*  32 */ "nested_chunk ::= empty indented_line",
 /*  33 */ "ext_chunk ::= chunk nested_chunks",
 /*  34 */ "opt_ext_chunk ::= chunk nested_chunks",
 /*  35 */ "blockquote ::= blockquote quote_line",
 /*  36 */ "def_citation ::= LINE_DEF_CITATION tail",
 /*  37 */ "def_footnote ::= LINE_DEF_FOOTNOTE tail",
 /*  38 */ "def_glossary ::= LINE_DEF_GLOSSARY tail",
 /*  39 */ "def_link ::= LINE_DEF_LINK chunk",
 /*  40 */ "def_abbreviation ::= LINE_DEF_ABBREVIATION chunk",
 /*  41 */ "definition_block ::= para defs",
 /*  42 */ "defs ::= defs def",
 /*  43 */ "def ::= LINE_DEFINITION tail",
 /*  44 */ "def ::= LINE_DEFINITION",
 /*  45 */ "empty ::= empty LINE_EMPTY",
 /*  46 */ "fenced_block ::= fenced_3 LINE_FENCE_BACKTICK_3",
 /*  47 */ "fenced_block ::= fenced_3 LINE_FENCE_BACKTICK_4",
 /*  48 */ "fenced_block ::= fenced_3 LINE_FENCE_BACKTICK_5",
 /*  49 */ "fenced_3 ::= fenced_3 fenced_line",
 /*  50 */ "fenced_block ::= fenced_4 LINE_FENCE_BACKTICK_4",
 /*  51 */ "fenced_block ::= fenced_4 LINE_FENCE_BACKTICK_5",
 /*  52 */ "fenced_4 ::= fenced_4 fenced_line",
 /*  53 */ "fenced_4 ::= fenced_4 LINE_FENCE_BACKTICK_3",
 /*  54 */ "fenced_4 ::= fenced_4 LINE_FENCE_BACKTICK_START_3",
 /*  55 */ "fenced_block ::= fenced_5 LINE_FENCE_BACKTICK_5",
 /*  56 */ "fenced_5 ::= fenced_5 fenced_line",
 /*  57 */ "fenced_5 ::= fenced_5 LINE_FENCE_BACKTICK_3",
 /*  58 */ "fenced_5 ::= fenced_5 LINE_FENCE_BACKTICK_START_3",
 /*  59 */ "fenced_5 ::= fenced_5 LINE_FENCE_BACKTICK_4",
 /*  60 */ "fenced_5 ::= fenced_5 LINE_FENCE_BACKTICK_START_4",
 /*  61 */ "html_block ::= html_block html_line",
 /*  62 */ "indented_code ::= indented_code indented_line",
 /*  63 */ "indented_code ::= indented_code LINE_EMPTY",
 /*  64 */ "list_bullet ::= list_bullet item_bullet",
 /*  65 */ "item_bullet ::= LINE_LIST_BULLETED ext_chunk",
 /*  66 */ "item_bullet ::= LINE_LIST_BULLETED chunk",
 /*  67 */ "item_bullet ::= LINE_LIST_BULLETED nested_chunks",
 /*  68 */ "item_bullet ::= LINE_LIST_BULLETED",
 /*  69 */ "list_enum ::= list_enum item_enum",
 /*  70 */ "item_enum ::= LINE_LIST_ENUMERATED ext_chunk",
 /*  71 */ "item_enum ::= LINE_LIST_ENUMERATED chunk",
 /*  72 */ "item_enum ::= LINE_LIST_ENUMERATED nested_chunks",
 /*  73 */ "item_enum ::= LINE_LIST_ENUMERATED",
 /*  74 */ "meta_block ::= meta_block meta_line",
 /*  75 */ "para ::= LINE_PLAIN chunk",
 /*  76 */ "setext_1 ::= para LINE_SETEXT_1",
 /*  77 */ "setext_2 ::= para LINE_SETEXT_2",
 /*  78 */ "table ::= table_header table_body",
 /*  79 */ "table_header ::= header_rows LINE_TABLE_SEPARATOR",
 /*  80 */ "header_rows ::= header_rows LINE_TABLE",
 /*  81 */ "table_body ::= table_body table_section",
 /*  82 */ "table_section ::= all_rows LINE_EMPTY",
 /*  83 */ "table_section ::= all_rows",
 /*  84 */ "all_rows ::= all_rows row",
 /*  85 */ "para ::= all_rows",
 /*  86 */ "chunk ::= chunk_line",
 /*  87 */ "chunk_line ::= LINE_CONTINUATION",
 /*  88 */ "nested_chunks ::= nested_chunk",
 /*  89 */ "nested_chunk ::= empty",
 /*  90 */ "indented_line ::= LINE_INDENTED_TAB",
 /*  91 */ "indented_line ::= LINE_INDENTED_SPACE",
 /*  92 */ "opt_ext_chunk ::= chunk",
 /*  93 */ "tail ::= opt_ext_chunk",
 /*  94 */ "tail ::= nested_chunks",
 /*  95 */ "blockquote ::= LINE_BLOCKQUOTE",
 /*  96 */ "quote_line ::= LINE_BLOCKQUOTE",
 /*  97 */ "quote_line ::= LINE_CONTINUATION",
 /*  98 */ "def_citation ::= LINE_DEF_CITATION",
 /*  99 */ "def_footnote ::= LINE_DEF_FOOTNOTE",
 /* 100 */ "def_glossary ::= LINE_DEF_GLOSSARY",
 /* 101 */ "def_link ::= LINE_DEF_LINK",
 /* 102 */ "def_abbreviation ::= LINE_DEF_ABBREVIATION",
 /* 103 */ "defs ::= def",
 /* 104 */ "empty ::= LINE_EMPTY",
 /* 105 */ "fenced_block ::= fenced_3",
 /* 106 */ "fenced_3 ::= LINE_FENCE_BACKTICK_3",
 /* 107 */ "fenced_3 ::= LINE_FENCE_BACKTICK_START_3",
 /* 108 */ "fenced_block ::= fenced_4",
 /* 109 */ "fenced_4 ::= LINE_FENCE_BACKTICK_4",
 /* 110 */ "fenced_4 ::= LINE_FENCE_BACKTICK_START_4",
 /* 111 */ "fenced_block ::= fenced_5",
 /* 112 */ "fenced_5 ::= LINE_FENCE_BACKTICK_5",
 /* 113 */ "fenced_5 ::= LINE_FENCE_BACKTICK_START_5",
 /* 114 */ "fenced_line ::= LINE_CONTINUATION",
 /* 115 */ "fenced_line ::= LINE_EMPTY",
 /* 116 */ "fenced_line ::= LINE_SETEXT_1",
 /* 117 */ "fenced_line ::= LINE_SETEXT_2",
 /* 118 */ "html_block ::= LINE_HTML",
 /* 119 */ "html_line ::= LINE_CONTINUATION",
 /* 120 */ "html_line ::= LINE_HTML",
 /* 121 */ "html_line ::= LINE_HR",
 /* 122 */ "indented_code ::= indented_line",
 /* 123 */ "list_bullet ::= item_bullet",
 /* 124 */ "list_enum ::= item_enum",
 /* 125 */ "meta_block ::= LINE_META",
 /* 126 */ "meta_line ::= LINE_META",
 /* 127 */ "meta_line ::= LINE_CONTINUATION",
 /* 128 */ "para ::= LINE_PLAIN",
 /* 129 */ "table ::= table_header",
 /* 130 */ "header_rows ::= LINE_TABLE",
 /* 131 */ "table_body ::= table_section",
 /* 132 */ "all_rows ::= row",
 /* 133 */ "row ::= header_rows",
 /* 134 */ "row ::= LINE_TABLE_SEPARATOR",
 /* 135 */ "para ::= defs",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.  Return the number
** of errors.  Return 0 on success.
*/
static int yyGrowStack(yyParser *p){
  int newSize;
  int idx;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  idx = p->yytos ? (int)(p->yytos - p->yystack) : 0;
  if( p->yystack==&p->yystk0 ){
    pNew = malloc(newSize*sizeof(pNew[0]));
    if( pNew ) pNew[0] = p->yystk0;
  }else{
    pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  }
  if( pNew ){
    p->yystack = pNew;
    p->yytos = &p->yystack[idx];
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows from %d to %d entries.\n",
              yyTracePrompt, p->yystksz, newSize);
    }
#endif
    p->yystksz = newSize;
  }
  return pNew==0; 
}
#endif

/* Datatype of the argument to the memory allocated passed as the
** second argument to ParseAlloc() below.  This can be changed by
** putting an appropriate #define in the %include section of the input
** grammar.
*/
#ifndef YYMALLOCARGTYPE
# define YYMALLOCARGTYPE size_t
#endif

/* 
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to Parse and ParseFree.
*/
void *ParseAlloc(void *(*mallocProc)(YYMALLOCARGTYPE)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (YYMALLOCARGTYPE)sizeof(yyParser) );
  if( pParser ){
#ifdef YYTRACKMAXSTACKDEPTH
    pParser->yyhwm = 0;
#endif
#if YYSTACKDEPTH<=0
    pParser->yytos = NULL;
    pParser->yystack = NULL;
    pParser->yystksz = 0;
    if( yyGrowStack(pParser) ){
      pParser->yystack = &pParser->yystk0;
      pParser->yystksz = 1;
    }
#endif
#ifndef YYNOERRORRECOVERY
    pParser->yyerrcnt = -1;
#endif
    pParser->yytos = pParser->yystack;
    pParser->yystack[0].stateno = 0;
    pParser->yystack[0].major = 0;
  }
  return pParser;
}

/* The following function deletes the "minor type" or semantic value
** associated with a symbol.  The symbol can be either a terminal
** or nonterminal. "yymajor" is the symbol code, and "yypminor" is
** a pointer to the value to be deleted.  The code used to do the 
** deletions is derived from the %destructor and/or %token_destructor
** directives of the input grammar.
*/
static void yy_destructor(
  yyParser *yypParser,    /* The parser */
  YYCODETYPE yymajor,     /* Type code for object to destroy */
  YYMINORTYPE *yypminor   /* The object to be destroyed */
){
  ParseARG_FETCH;
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are *not* used
    ** inside the C code.
    */
/********* Begin destructor definitions ***************************************/
/********* End destructor definitions *****************************************/
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
*/
static void yy_pop_parser_stack(yyParser *pParser){
  yyStackEntry *yytos;
  assert( pParser->yytos!=0 );
  assert( pParser->yytos > pParser->yystack );
  yytos = pParser->yytos--;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yy_destructor(pParser, yytos->major, &yytos->minor);
}

/* 
** Deallocate and destroy a parser.  Destructors are called for
** all stack elements before shutting the parser down.
**
** If the YYPARSEFREENEVERNULL macro exists (for example because it
** is defined in a %include section of the input grammar) then it is
** assumed that the input pointer is never NULL.
*/
void ParseFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
#ifndef YYPARSEFREENEVERNULL
  if( pParser==0 ) return;
#endif
  while( pParser->yytos>pParser->yystack ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  if( pParser->yystack!=&pParser->yystk0 ) free(pParser->yystack);
#endif
  (*freeProc)((void*)pParser);
}

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int ParseStackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyhwm;
}
#endif

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
*/
static unsigned int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->yytos->stateno;
 
  if( stateno>=YY_MIN_REDUCE ) return stateno;
  assert( stateno <= YY_SHIFT_COUNT );
  do{
    i = yy_shift_ofst[stateno];
    assert( iLookAhead!=YYNOCODE );
    i += iLookAhead;
    if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
#ifdef YYFALLBACK
      YYCODETYPE iFallback;            /* Fallback token */
      if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
             && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
             yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
        }
#endif
        assert( yyFallback[iFallback]==0 ); /* Fallback loop must terminate */
        iLookAhead = iFallback;
        continue;
      }
#endif
#ifdef YYWILDCARD
      {
        int j = i - iLookAhead + YYWILDCARD;
        if( 
#if YY_SHIFT_MIN+YYWILDCARD<0
          j>=0 &&
#endif
#if YY_SHIFT_MAX+YYWILDCARD>=YY_ACTTAB_COUNT
          j<YY_ACTTAB_COUNT &&
#endif
          yy_lookahead[j]==YYWILDCARD && iLookAhead>0
        ){
#ifndef NDEBUG
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead],
               yyTokenName[YYWILDCARD]);
          }
#endif /* NDEBUG */
          return yy_action[j];
        }
      }
#endif /* YYWILDCARD */
      return yy_default[stateno];
    }else{
      return yy_action[i];
    }
  }while(1);
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
*/
static int yy_find_reduce_action(
  int stateno,              /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
#ifdef YYERRORSYMBOL
  if( stateno>YY_REDUCE_COUNT ){
    return yy_default[stateno];
  }
#else
  assert( stateno<=YY_REDUCE_COUNT );
#endif
  i = yy_reduce_ofst[stateno];
  assert( i!=YY_REDUCE_USE_DFLT );
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
#ifdef YYERRORSYMBOL
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }
#else
  assert( i>=0 && i<YY_ACTTAB_COUNT );
  assert( yy_lookahead[i]==iLookAhead );
#endif
  return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser){
   ParseARG_FETCH;
   yypParser->yytos--;
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yytos>yypParser->yystack ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
/******** Begin %stack_overflow code ******************************************/
/******** End %stack_overflow code ********************************************/
   ParseARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Print tracing information for a SHIFT action
*/
#ifndef NDEBUG
static void yyTraceShift(yyParser *yypParser, int yyNewState){
  if( yyTraceFILE ){
    if( yyNewState<YYNSTATE ){
      fprintf(yyTraceFILE,"%sShift '%s', go to state %d\n",
         yyTracePrompt,yyTokenName[yypParser->yytos->major],
         yyNewState);
    }else{
      fprintf(yyTraceFILE,"%sShift '%s'\n",
         yyTracePrompt,yyTokenName[yypParser->yytos->major]);
    }
  }
}
#else
# define yyTraceShift(X,Y)
#endif

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  ParseTOKENTYPE yyMinor        /* The minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yytos++;
#ifdef YYTRACKMAXSTACKDEPTH
  if( (int)(yypParser->yytos - yypParser->yystack)>yypParser->yyhwm ){
    yypParser->yyhwm++;
    assert( yypParser->yyhwm == (int)(yypParser->yytos - yypParser->yystack) );
  }
#endif
#if YYSTACKDEPTH>0 
  if( yypParser->yytos>=&yypParser->yystack[YYSTACKDEPTH] ){
    yyStackOverflow(yypParser);
    return;
  }
#else
  if( yypParser->yytos>=&yypParser->yystack[yypParser->yystksz] ){
    if( yyGrowStack(yypParser) ){
      yyStackOverflow(yypParser);
      return;
    }
  }
#endif
  if( yyNewState > YY_MAX_SHIFT ){
    yyNewState += YY_MIN_REDUCE - YY_MIN_SHIFTREDUCE;
  }
  yytos = yypParser->yytos;
  yytos->stateno = (YYACTIONTYPE)yyNewState;
  yytos->major = (YYCODETYPE)yyMajor;
  yytos->minor.yy0 = yyMinor;
  yyTraceShift(yypParser, yyNewState);
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 38, 1 },
  { 39, 2 },
  { 39, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 59, 2 },
  { 61, 2 },
  { 62, 3 },
  { 62, 2 },
  { 64, 2 },
  { 65, 2 },
  { 41, 2 },
  { 43, 2 },
  { 44, 2 },
  { 45, 2 },
  { 46, 2 },
  { 42, 2 },
  { 47, 2 },
  { 68, 2 },
  { 69, 2 },
  { 69, 1 },
  { 48, 2 },
  { 49, 2 },
  { 49, 2 },
  { 49, 2 },
  { 70, 2 },
  { 49, 2 },
  { 49, 2 },
  { 72, 2 },
  { 72, 2 },
  { 72, 2 },
  { 49, 2 },
  { 73, 2 },
  { 73, 2 },
  { 73, 2 },
  { 73, 2 },
  { 73, 2 },
  { 50, 2 },
  { 51, 2 },
  { 51, 2 },
  { 52, 2 },
  { 75, 2 },
  { 75, 2 },
  { 75, 2 },
  { 75, 1 },
  { 53, 2 },
  { 76, 2 },
  { 76, 2 },
  { 76, 2 },
  { 76, 1 },
  { 54, 2 },
  { 55, 2 },
  { 56, 2 },
  { 57, 2 },
  { 58, 2 },
  { 78, 2 },
  { 80, 2 },
  { 79, 2 },
  { 81, 2 },
  { 81, 1 },
  { 82, 2 },
  { 55, 1 },
  { 59, 1 },
  { 60, 1 },
  { 61, 1 },
  { 62, 1 },
  { 63, 1 },
  { 63, 1 },
  { 65, 1 },
  { 66, 1 },
  { 66, 1 },
  { 41, 1 },
  { 67, 1 },
  { 67, 1 },
  { 43, 1 },
  { 44, 1 },
  { 45, 1 },
  { 46, 1 },
  { 42, 1 },
  { 68, 1 },
  { 48, 1 },
  { 49, 1 },
  { 70, 1 },
  { 70, 1 },
  { 49, 1 },
  { 72, 1 },
  { 72, 1 },
  { 49, 1 },
  { 73, 1 },
  { 73, 1 },
  { 71, 1 },
  { 71, 1 },
  { 71, 1 },
  { 71, 1 },
  { 50, 1 },
  { 74, 1 },
  { 74, 1 },
  { 74, 1 },
  { 51, 1 },
  { 52, 1 },
  { 53, 1 },
  { 54, 1 },
  { 77, 1 },
  { 77, 1 },
  { 55, 1 },
  { 58, 1 },
  { 80, 1 },
  { 79, 1 },
  { 82, 1 },
  { 83, 1 },
  { 83, 1 },
  { 55, 1 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  unsigned int yyruleno        /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  ParseARG_FETCH;
  yymsp = yypParser->yytos;
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    yysize = yyRuleInfo[yyruleno].nrhs;
    fprintf(yyTraceFILE, "%sReduce [%s], go to state %d.\n", yyTracePrompt,
      yyRuleName[yyruleno], yymsp[-yysize].stateno);
  }
#endif /* NDEBUG */

  /* Check that the stack is large enough to grow by a single entry
  ** if the RHS of the rule is empty.  This ensures that there is room
  ** enough on the stack to push the LHS value */
  if( yyRuleInfo[yyruleno].nrhs==0 ){
#ifdef YYTRACKMAXSTACKDEPTH
    if( (int)(yypParser->yytos - yypParser->yystack)>yypParser->yyhwm ){
      yypParser->yyhwm++;
      assert( yypParser->yyhwm == (int)(yypParser->yytos - yypParser->yystack));
    }
#endif
#if YYSTACKDEPTH>0 
    if( yypParser->yytos>=&yypParser->yystack[YYSTACKDEPTH-1] ){
      yyStackOverflow(yypParser);
      return;
    }
#else
    if( yypParser->yytos>=&yypParser->yystack[yypParser->yystksz-1] ){
      if( yyGrowStack(yypParser) ){
        yyStackOverflow(yypParser);
        return;
      }
      yymsp = yypParser->yytos;
    }
#endif
  }

  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
/********** Begin reduce actions **********************************************/
        YYMINORTYPE yylhsminor;
      case 0: /* doc ::= blocks */
{ engine->root = yymsp[0].minor.yy0; }
        break;
      case 1: /* blocks ::= blocks block */
{
		strip_line_tokens_from_block(engine, yymsp[0].minor.yy0);
		if (yymsp[-1].minor.yy0 == NULL) { yymsp[-1].minor.yy0 = yymsp[0].minor.yy0; yymsp[0].minor.yy0 = NULL;}
		yylhsminor.yy0 = yymsp[-1].minor.yy0;
		token_chain_append(yylhsminor.yy0, yymsp[0].minor.yy0);
		#ifndef NDEBUG
		fprintf(stderr, "Next block %d\n", yylhsminor.yy0->tail->type);
		#endif
	}
  yymsp[-1].minor.yy0 = yylhsminor.yy0;
        break;
      case 2: /* blocks ::= block */
{
		engine->root = yymsp[0].minor.yy0;	// In case the first block is metadata and we just want to know if it exists
		strip_line_tokens_from_block(engine, yymsp[0].minor.yy0);
		#ifndef NDEBUG
		fprintf(stderr, "First block %d\n", yymsp[0].minor.yy0->type);
		#endif
		yylhsminor.yy0 = yymsp[0].minor.yy0;
	}
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 3: /* block ::= LINE_ATX_1 */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_H1); if (!(engine->extensions & EXT_NO_LABELS)) stack_push(engine->header_stack, yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 4: /* block ::= LINE_ATX_2 */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_H2); if (!(engine->extensions & EXT_NO_LABELS)) stack_push(engine->header_stack, yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 5: /* block ::= LINE_ATX_3 */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_H3); if (!(engine->extensions & EXT_NO_LABELS)) stack_push(engine->header_stack, yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 6: /* block ::= LINE_ATX_4 */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_H4); if (!(engine->extensions & EXT_NO_LABELS)) stack_push(engine->header_stack, yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 7: /* block ::= LINE_ATX_5 */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_H5); if (!(engine->extensions & EXT_NO_LABELS)) stack_push(engine->header_stack, yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 8: /* block ::= LINE_ATX_6 */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_H6); if (!(engine->extensions & EXT_NO_LABELS)) stack_push(engine->header_stack, yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 9: /* block ::= LINE_HR */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_HR); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 10: /* block ::= LINE_TOC */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_TOC); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 11: /* block ::= blockquote */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_BLOCKQUOTE); recursive_parse_blockquote(engine, yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 12: /* block ::= def_abbreviation */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_DEF_ABBREVIATION); stack_push(engine->definition_stack, yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 13: /* block ::= def_citation */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_DEF_CITATION); stack_push(engine->definition_stack, yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 14: /* block ::= def_footnote */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_DEF_FOOTNOTE); stack_push(engine->definition_stack, yylhsminor.yy0); recursive_parse_indent(engine, yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 15: /* block ::= def_glossary */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_DEF_GLOSSARY); stack_push(engine->definition_stack, yylhsminor.yy0); recursive_parse_indent(engine, yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 16: /* block ::= def_link */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_DEF_LINK); stack_push(engine->definition_stack, yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 17: /* block ::= definition_block */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_DEFLIST); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 18: /* block ::= empty */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_EMPTY); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 19: /* block ::= fenced_block */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_CODE_FENCED); yymsp[0].minor.yy0->child->type = CODE_FENCE; }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 20: /* block ::= html_block */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_HTML); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 21: /* block ::= indented_code */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_CODE_INDENTED); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 22: /* block ::= list_bullet */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_LIST_BULLETED); is_list_loose(yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 23: /* block ::= list_enum */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_LIST_ENUMERATED); is_list_loose(yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 24: /* block ::= meta_block */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_META); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 25: /* block ::= para */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_PARA); is_para_html(engine, yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 26: /* block ::= setext_1 */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_SETEXT_1); if (!(engine->extensions & EXT_NO_LABELS)) stack_push(engine->header_stack, yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 27: /* block ::= setext_2 */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_SETEXT_2); if (!(engine->extensions & EXT_NO_LABELS)) stack_push(engine->header_stack, yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 28: /* block ::= table */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_TABLE); stack_push(engine->table_stack, yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 29: /* chunk ::= chunk chunk_line */
      case 30: /* nested_chunks ::= nested_chunks nested_chunk */ yytestcase(yyruleno==30);
      case 33: /* ext_chunk ::= chunk nested_chunks */ yytestcase(yyruleno==33);
      case 34: /* opt_ext_chunk ::= chunk nested_chunks */ yytestcase(yyruleno==34);
      case 35: /* blockquote ::= blockquote quote_line */ yytestcase(yyruleno==35);
      case 36: /* def_citation ::= LINE_DEF_CITATION tail */ yytestcase(yyruleno==36);
      case 37: /* def_footnote ::= LINE_DEF_FOOTNOTE tail */ yytestcase(yyruleno==37);
      case 38: /* def_glossary ::= LINE_DEF_GLOSSARY tail */ yytestcase(yyruleno==38);
      case 39: /* def_link ::= LINE_DEF_LINK chunk */ yytestcase(yyruleno==39);
      case 40: /* def_abbreviation ::= LINE_DEF_ABBREVIATION chunk */ yytestcase(yyruleno==40);
      case 42: /* defs ::= defs def */ yytestcase(yyruleno==42);
      case 45: /* empty ::= empty LINE_EMPTY */ yytestcase(yyruleno==45);
      case 49: /* fenced_3 ::= fenced_3 fenced_line */ yytestcase(yyruleno==49);
      case 52: /* fenced_4 ::= fenced_4 fenced_line */ yytestcase(yyruleno==52);
      case 53: /* fenced_4 ::= fenced_4 LINE_FENCE_BACKTICK_3 */ yytestcase(yyruleno==53);
      case 54: /* fenced_4 ::= fenced_4 LINE_FENCE_BACKTICK_START_3 */ yytestcase(yyruleno==54);
      case 56: /* fenced_5 ::= fenced_5 fenced_line */ yytestcase(yyruleno==56);
      case 57: /* fenced_5 ::= fenced_5 LINE_FENCE_BACKTICK_3 */ yytestcase(yyruleno==57);
      case 58: /* fenced_5 ::= fenced_5 LINE_FENCE_BACKTICK_START_3 */ yytestcase(yyruleno==58);
      case 59: /* fenced_5 ::= fenced_5 LINE_FENCE_BACKTICK_4 */ yytestcase(yyruleno==59);
      case 60: /* fenced_5 ::= fenced_5 LINE_FENCE_BACKTICK_START_4 */ yytestcase(yyruleno==60);
      case 61: /* html_block ::= html_block html_line */ yytestcase(yyruleno==61);
      case 62: /* indented_code ::= indented_code indented_line */ yytestcase(yyruleno==62);
      case 63: /* indented_code ::= indented_code LINE_EMPTY */ yytestcase(yyruleno==63);
      case 64: /* list_bullet ::= list_bullet item_bullet */ yytestcase(yyruleno==64);
      case 69: /* list_enum ::= list_enum item_enum */ yytestcase(yyruleno==69);
      case 74: /* meta_block ::= meta_block meta_line */ yytestcase(yyruleno==74);
      case 75: /* para ::= LINE_PLAIN chunk */ yytestcase(yyruleno==75);
      case 76: /* setext_1 ::= para LINE_SETEXT_1 */ yytestcase(yyruleno==76);
      case 77: /* setext_2 ::= para LINE_SETEXT_2 */ yytestcase(yyruleno==77);
      case 78: /* table ::= table_header table_body */ yytestcase(yyruleno==78);
      case 80: /* header_rows ::= header_rows LINE_TABLE */ yytestcase(yyruleno==80);
      case 81: /* table_body ::= table_body table_section */ yytestcase(yyruleno==81);
      case 84: /* all_rows ::= all_rows row */ yytestcase(yyruleno==84);
{ yylhsminor.yy0 = yymsp[-1].minor.yy0; token_chain_append(yymsp[-1].minor.yy0, yymsp[0].minor.yy0); }
  yymsp[-1].minor.yy0 = yylhsminor.yy0;
        break;
      case 31: /* nested_chunk ::= empty indented_line chunk */
{ yylhsminor.yy0 = yymsp[-2].minor.yy0; token_chain_append(yymsp[-2].minor.yy0, yymsp[-1].minor.yy0); token_chain_append(yymsp[-2].minor.yy0, yymsp[0].minor.yy0); yymsp[-1].minor.yy0->type = LINE_CONTINUATION; }
  yymsp[-2].minor.yy0 = yylhsminor.yy0;
        break;
      case 32: /* nested_chunk ::= empty indented_line */
{ yylhsminor.yy0 = yymsp[-1].minor.yy0; token_chain_append(yymsp[-1].minor.yy0, yymsp[0].minor.yy0); yymsp[0].minor.yy0->type = LINE_CONTINUATION; }
  yymsp[-1].minor.yy0 = yylhsminor.yy0;
        break;
      case 41: /* definition_block ::= para defs */
{ yylhsminor.yy0 = yymsp[-1].minor.yy0; token_chain_append(yymsp[-1].minor.yy0, yymsp[0].minor.yy0); yymsp[-1].minor.yy0->type = BLOCK_TERM; }
  yymsp[-1].minor.yy0 = yylhsminor.yy0;
        break;
      case 43: /* def ::= LINE_DEFINITION tail */
{ yylhsminor.yy0 = token_new_parent(yymsp[-1].minor.yy0, BLOCK_DEFINITION); token_chain_append(yymsp[-1].minor.yy0, yymsp[0].minor.yy0); recursive_parse_indent(engine, yylhsminor.yy0); }
  yymsp[-1].minor.yy0 = yylhsminor.yy0;
        break;
      case 44: /* def ::= LINE_DEFINITION */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_DEFINITION);  }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 46: /* fenced_block ::= fenced_3 LINE_FENCE_BACKTICK_3 */
      case 47: /* fenced_block ::= fenced_3 LINE_FENCE_BACKTICK_4 */ yytestcase(yyruleno==47);
      case 48: /* fenced_block ::= fenced_3 LINE_FENCE_BACKTICK_5 */ yytestcase(yyruleno==48);
      case 50: /* fenced_block ::= fenced_4 LINE_FENCE_BACKTICK_4 */ yytestcase(yyruleno==50);
      case 51: /* fenced_block ::= fenced_4 LINE_FENCE_BACKTICK_5 */ yytestcase(yyruleno==51);
      case 55: /* fenced_block ::= fenced_5 LINE_FENCE_BACKTICK_5 */ yytestcase(yyruleno==55);
{ yylhsminor.yy0 = yymsp[-1].minor.yy0; token_chain_append(yymsp[-1].minor.yy0, yymsp[0].minor.yy0); yymsp[0].minor.yy0->child->type = CODE_FENCE; }
  yymsp[-1].minor.yy0 = yylhsminor.yy0;
        break;
      case 65: /* item_bullet ::= LINE_LIST_BULLETED ext_chunk */
      case 67: /* item_bullet ::= LINE_LIST_BULLETED nested_chunks */ yytestcase(yyruleno==67);
      case 70: /* item_enum ::= LINE_LIST_ENUMERATED ext_chunk */ yytestcase(yyruleno==70);
      case 72: /* item_enum ::= LINE_LIST_ENUMERATED nested_chunks */ yytestcase(yyruleno==72);
{ yylhsminor.yy0 = token_new_parent(yymsp[-1].minor.yy0, BLOCK_LIST_ITEM); token_chain_append(yymsp[-1].minor.yy0, yymsp[0].minor.yy0); recursive_parse_list_item(engine, yylhsminor.yy0); }
  yymsp[-1].minor.yy0 = yylhsminor.yy0;
        break;
      case 66: /* item_bullet ::= LINE_LIST_BULLETED chunk */
      case 71: /* item_enum ::= LINE_LIST_ENUMERATED chunk */ yytestcase(yyruleno==71);
{ yylhsminor.yy0 = token_new_parent(yymsp[-1].minor.yy0, BLOCK_LIST_ITEM_TIGHT); token_chain_append(yymsp[-1].minor.yy0, yymsp[0].minor.yy0); recursive_parse_list_item(engine, yylhsminor.yy0); }
  yymsp[-1].minor.yy0 = yylhsminor.yy0;
        break;
      case 68: /* item_bullet ::= LINE_LIST_BULLETED */
      case 73: /* item_enum ::= LINE_LIST_ENUMERATED */ yytestcase(yyruleno==73);
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_LIST_ITEM_TIGHT); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 79: /* table_header ::= header_rows LINE_TABLE_SEPARATOR */
{ yylhsminor.yy0 = token_new_parent(yymsp[-1].minor.yy0, BLOCK_TABLE_HEADER); token_chain_append(yymsp[-1].minor.yy0, yymsp[0].minor.yy0); }
  yymsp[-1].minor.yy0 = yylhsminor.yy0;
        break;
      case 82: /* table_section ::= all_rows LINE_EMPTY */
{ yylhsminor.yy0 = token_new_parent(yymsp[-1].minor.yy0, BLOCK_TABLE_SECTION); token_chain_append(yymsp[-1].minor.yy0, yymsp[0].minor.yy0); }
  yymsp[-1].minor.yy0 = yylhsminor.yy0;
        break;
      case 83: /* table_section ::= all_rows */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_TABLE_SECTION); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 85: /* para ::= all_rows */
{ yylhsminor.yy0 = yymsp[0].minor.yy0; }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      default:
      /* (86) chunk ::= chunk_line (OPTIMIZED OUT) */ assert(yyruleno!=86);
      /* (87) chunk_line ::= LINE_CONTINUATION */ yytestcase(yyruleno==87);
      /* (88) nested_chunks ::= nested_chunk (OPTIMIZED OUT) */ assert(yyruleno!=88);
      /* (89) nested_chunk ::= empty */ yytestcase(yyruleno==89);
      /* (90) indented_line ::= LINE_INDENTED_TAB */ yytestcase(yyruleno==90);
      /* (91) indented_line ::= LINE_INDENTED_SPACE */ yytestcase(yyruleno==91);
      /* (92) opt_ext_chunk ::= chunk */ yytestcase(yyruleno==92);
      /* (93) tail ::= opt_ext_chunk (OPTIMIZED OUT) */ assert(yyruleno!=93);
      /* (94) tail ::= nested_chunks */ yytestcase(yyruleno==94);
      /* (95) blockquote ::= LINE_BLOCKQUOTE */ yytestcase(yyruleno==95);
      /* (96) quote_line ::= LINE_BLOCKQUOTE */ yytestcase(yyruleno==96);
      /* (97) quote_line ::= LINE_CONTINUATION */ yytestcase(yyruleno==97);
      /* (98) def_citation ::= LINE_DEF_CITATION */ yytestcase(yyruleno==98);
      /* (99) def_footnote ::= LINE_DEF_FOOTNOTE */ yytestcase(yyruleno==99);
      /* (100) def_glossary ::= LINE_DEF_GLOSSARY */ yytestcase(yyruleno==100);
      /* (101) def_link ::= LINE_DEF_LINK */ yytestcase(yyruleno==101);
      /* (102) def_abbreviation ::= LINE_DEF_ABBREVIATION */ yytestcase(yyruleno==102);
      /* (103) defs ::= def (OPTIMIZED OUT) */ assert(yyruleno!=103);
      /* (104) empty ::= LINE_EMPTY */ yytestcase(yyruleno==104);
      /* (105) fenced_block ::= fenced_3 */ yytestcase(yyruleno==105);
      /* (106) fenced_3 ::= LINE_FENCE_BACKTICK_3 */ yytestcase(yyruleno==106);
      /* (107) fenced_3 ::= LINE_FENCE_BACKTICK_START_3 */ yytestcase(yyruleno==107);
      /* (108) fenced_block ::= fenced_4 */ yytestcase(yyruleno==108);
      /* (109) fenced_4 ::= LINE_FENCE_BACKTICK_4 */ yytestcase(yyruleno==109);
      /* (110) fenced_4 ::= LINE_FENCE_BACKTICK_START_4 */ yytestcase(yyruleno==110);
      /* (111) fenced_block ::= fenced_5 */ yytestcase(yyruleno==111);
      /* (112) fenced_5 ::= LINE_FENCE_BACKTICK_5 */ yytestcase(yyruleno==112);
      /* (113) fenced_5 ::= LINE_FENCE_BACKTICK_START_5 */ yytestcase(yyruleno==113);
      /* (114) fenced_line ::= LINE_CONTINUATION */ yytestcase(yyruleno==114);
      /* (115) fenced_line ::= LINE_EMPTY */ yytestcase(yyruleno==115);
      /* (116) fenced_line ::= LINE_SETEXT_1 */ yytestcase(yyruleno==116);
      /* (117) fenced_line ::= LINE_SETEXT_2 */ yytestcase(yyruleno==117);
      /* (118) html_block ::= LINE_HTML */ yytestcase(yyruleno==118);
      /* (119) html_line ::= LINE_CONTINUATION */ yytestcase(yyruleno==119);
      /* (120) html_line ::= LINE_HTML */ yytestcase(yyruleno==120);
      /* (121) html_line ::= LINE_HR */ yytestcase(yyruleno==121);
      /* (122) indented_code ::= indented_line (OPTIMIZED OUT) */ assert(yyruleno!=122);
      /* (123) list_bullet ::= item_bullet (OPTIMIZED OUT) */ assert(yyruleno!=123);
      /* (124) list_enum ::= item_enum (OPTIMIZED OUT) */ assert(yyruleno!=124);
      /* (125) meta_block ::= LINE_META */ yytestcase(yyruleno==125);
      /* (126) meta_line ::= LINE_META */ yytestcase(yyruleno==126);
      /* (127) meta_line ::= LINE_CONTINUATION */ yytestcase(yyruleno==127);
      /* (128) para ::= LINE_PLAIN */ yytestcase(yyruleno==128);
      /* (129) table ::= table_header */ yytestcase(yyruleno==129);
      /* (130) header_rows ::= LINE_TABLE */ yytestcase(yyruleno==130);
      /* (131) table_body ::= table_section (OPTIMIZED OUT) */ assert(yyruleno!=131);
      /* (132) all_rows ::= row (OPTIMIZED OUT) */ assert(yyruleno!=132);
      /* (133) row ::= header_rows */ yytestcase(yyruleno==133);
      /* (134) row ::= LINE_TABLE_SEPARATOR */ yytestcase(yyruleno==134);
      /* (135) para ::= defs */ yytestcase(yyruleno==135);
        break;
/********** End reduce actions ************************************************/
  };
  assert( yyruleno<sizeof(yyRuleInfo)/sizeof(yyRuleInfo[0]) );
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,(YYCODETYPE)yygoto);
  if( yyact <= YY_MAX_SHIFTREDUCE ){
    if( yyact>YY_MAX_SHIFT ){
      yyact += YY_MIN_REDUCE - YY_MIN_SHIFTREDUCE;
    }
    yymsp -= yysize-1;
    yypParser->yytos = yymsp;
    yymsp->stateno = (YYACTIONTYPE)yyact;
    yymsp->major = (YYCODETYPE)yygoto;
    yyTraceShift(yypParser, yyact);
  }else{
    assert( yyact == YY_ACCEPT_ACTION );
    yypParser->yytos -= yysize;
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
#ifndef YYNOERRORRECOVERY
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  ParseARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yytos>yypParser->yystack ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
/************ Begin %parse_failure code ***************************************/

	fprintf(stderr, "Parser failed to successfully parse.\n");
/************ End %parse_failure code *****************************************/
  ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}
#endif /* YYNOERRORRECOVERY */

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  ParseTOKENTYPE yyminor         /* The minor type of the error token */
){
  ParseARG_FETCH;
#define TOKEN yyminor
/************ Begin %syntax_error code ****************************************/

#ifndef NDEBUG
	fprintf(stderr,"Parser syntax error.\n");
	int n = sizeof(yyTokenName) / sizeof(yyTokenName[0]);
	for (int i = 0; i < n; ++i) {
		int a = yy_find_shift_action(yypParser, (YYCODETYPE)i);
		if (a < YYNSTATE + YYNRULE) {
			fprintf(stderr,"expected token: %s\n", yyTokenName[i]);
		}
	}
#endif
/************ End %syntax_error code ******************************************/
  ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  ParseARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
#ifndef YYNOERRORRECOVERY
  yypParser->yyerrcnt = -1;
#endif
  assert( yypParser->yytos==yypParser->yystack );
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
/*********** Begin %parse_accept code *****************************************/
/*********** End %parse_accept code *******************************************/
  ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "ParseAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void Parse(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  ParseTOKENTYPE yyminor       /* The value for the token */
  ParseARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  unsigned int yyact;   /* The parser action. */
#if !defined(YYERRORSYMBOL) && !defined(YYNOERRORRECOVERY)
  int yyendofinput;     /* True if we are at the end of input */
#endif
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser;  /* The parser */

  yypParser = (yyParser*)yyp;
  assert( yypParser->yytos!=0 );
#if !defined(YYERRORSYMBOL) && !defined(YYNOERRORRECOVERY)
  yyendofinput = (yymajor==0);
#endif
  ParseARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput '%s'\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,(YYCODETYPE)yymajor);
    if( yyact <= YY_MAX_SHIFTREDUCE ){
      yy_shift(yypParser,yyact,yymajor,yyminor);
#ifndef YYNOERRORRECOVERY
      yypParser->yyerrcnt--;
#endif
      yymajor = YYNOCODE;
    }else if( yyact <= YY_MAX_REDUCE ){
      yy_reduce(yypParser,yyact-YY_MIN_REDUCE);
    }else{
      assert( yyact == YY_ERROR_ACTION );
      yyminorunion.yy0 = yyminor;
#ifdef YYERRORSYMBOL
      int yymx;
#endif
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".  
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminor);
      }
      yymx = yypParser->yytos->major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yypParser, (YYCODETYPE)yymajor, &yyminorunion);
        yymajor = YYNOCODE;
      }else{
        while( yypParser->yytos >= yypParser->yystack
            && yymx != YYERRORSYMBOL
            && (yyact = yy_find_reduce_action(
                        yypParser->yytos->stateno,
                        YYERRORSYMBOL)) >= YY_MIN_REDUCE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yytos < yypParser->yystack || yymajor==0 ){
          yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
#ifndef YYNOERRORRECOVERY
          yypParser->yyerrcnt = -1;
#endif
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          yy_shift(yypParser,yyact,YYERRORSYMBOL,yyminor);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#elif defined(YYNOERRORRECOVERY)
      /* If the YYNOERRORRECOVERY macro is defined, then do not attempt to
      ** do any kind of error recovery.  Instead, simply invoke the syntax
      ** error routine and continue going as if nothing had happened.
      **
      ** Applications can set this macro (for example inside %include) if
      ** they intend to abandon the parse upon the first syntax error seen.
      */
      yy_syntax_error(yypParser,yymajor, yyminor);
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      yymajor = YYNOCODE;
      
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor, yyminor);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
#ifndef YYNOERRORRECOVERY
        yypParser->yyerrcnt = -1;
#endif
      }
      yymajor = YYNOCODE;
#endif
    }
  }while( yymajor!=YYNOCODE && yypParser->yytos>yypParser->yystack );
#ifndef NDEBUG
  if( yyTraceFILE ){
    yyStackEntry *i;
    char cDiv = '[';
    fprintf(yyTraceFILE,"%sReturn. Stack=",yyTracePrompt);
    for(i=&yypParser->yystack[1]; i<=yypParser->yytos; i++){
      fprintf(yyTraceFILE,"%c%s", cDiv, yyTokenName[i->major]);
      cDiv = ' ';
    }
    fprintf(yyTraceFILE,"]\n");
  }
#endif
  return;
}
