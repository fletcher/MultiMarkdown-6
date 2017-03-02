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
#define YYNOCODE 83
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
#define YYNSTATE             41
#define YYNRULE              131
#define YY_MAX_SHIFT         40
#define YY_MIN_SHIFTREDUCE   133
#define YY_MAX_SHIFTREDUCE   263
#define YY_MIN_REDUCE        264
#define YY_MAX_REDUCE        394
#define YY_ERROR_ACTION      395
#define YY_ACCEPT_ACTION     396
#define YY_NO_ACTION         397
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
#define YY_ACTTAB_COUNT (258)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   396,    1,  135,   30,  145,  146,  147,  148,  149,   40,
 /*    10 */   151,   29,   27,   37,   35,   28,   13,  158,  159,  160,
 /*    20 */    39,  211,   12,   12,   27,  233,  220,  221,  262,   32,
 /*    30 */    32,   25,  258,   24,   23,  217,   37,   35,  262,    7,
 /*    40 */   186,   38,  258,   14,   14,  264,  142,  175,  182,   15,
 /*    50 */   262,  212,  220,  221,  258,  247,  136,  137,  138,  139,
 /*    60 */   140,  141,  225,    6,    5,    3,    2,   16,   34,   34,
 /*    70 */   143,  231,    4,  233,  235,  238,  241,  236,  239,  242,
 /*    80 */   253,  142,  220,  221,   15,  262,  179,  220,  221,  258,
 /*    90 */   247,  136,  137,  138,  139,  140,  141,  225,    6,    5,
 /*   100 */     3,    2,   16,  193,   21,  143,  231,    4,  233,  235,
 /*   110 */   238,  241,  236,  239,  242,  253,  134,   30,  145,  146,
 /*   120 */   147,  148,  149,   40,  151,   29,   27,   37,   35,   28,
 /*   130 */    13,  158,  159,  160,  217,  245,  246,   39,   27,  243,
 /*   140 */   214,   31,   31,   32,   32,   25,  192,   24,   23,  204,
 /*   150 */    37,   35,  167,    7,  191,   38,  233,   14,   14,   33,
 /*   160 */    33,  244,  187,  189,  185,  188,  190,  245,  246,   26,
 /*   170 */    26,  243,    8,   39,    8,   12,   12,   36,   36,  161,
 /*   180 */     9,    9,   17,   17,  162,  248,  169,  169,  227,  249,
 /*   190 */    26,  255,  172,  244,  183,  180,  181,  184,   26,  226,
 /*   200 */     4,    9,    9,   17,   17,    5,  199,  168,  168,    9,
 /*   210 */     9,   17,   17,  245,  246,  173,  173,  243,   26,    6,
 /*   220 */   254,  209,  194,   26,  210,  210,  206,  207,  175,   10,
 /*   230 */    10,   19,   19,   26,  200,  161,   18,   18,  266,  244,
 /*   240 */   176,  177,  178,   26,   11,   11,   20,   20,  266,  195,
 /*   250 */   266,    4,  266,  266,  266,  161,   22,   22,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    37,   38,   39,   40,   41,   42,   43,   44,   45,   46,
 /*    10 */    47,   48,   49,   50,   51,   52,   53,   54,   55,   56,
 /*    20 */    78,   79,   80,   81,   61,   28,    7,    8,    5,   66,
 /*    30 */    67,   68,    9,   70,   71,    6,   73,   74,    5,   76,
 /*    40 */    69,   78,    9,   80,   81,    0,    1,   28,   69,    4,
 /*    50 */     5,   28,    7,    8,    9,   10,   11,   12,   13,   14,
 /*    60 */    15,   16,   17,   18,   19,   20,   21,   22,   66,   67,
 /*    70 */    25,   26,   27,   28,   29,   30,   31,   32,   33,   34,
 /*    80 */    35,    1,    7,    8,    4,    5,   69,    7,    8,    9,
 /*    90 */    10,   11,   12,   13,   14,   15,   16,   17,   18,   19,
 /*   100 */    20,   21,   22,   28,   61,   25,   26,   27,   28,   29,
 /*   110 */    30,   31,   32,   33,   34,   35,   39,   40,   41,   42,
 /*   120 */    43,   44,   45,   46,   47,   48,   49,   50,   51,   52,
 /*   130 */    53,   54,   55,   56,    6,    2,    3,   78,   61,    6,
 /*   140 */    81,   57,   58,   66,   67,   68,   61,   70,   71,   75,
 /*   150 */    73,   74,   65,   76,   72,   78,   28,   80,   81,   57,
 /*   160 */    58,   28,   29,   30,   31,   32,   33,    2,    3,   46,
 /*   170 */    46,    6,   77,   78,   79,   80,   81,   57,   58,   58,
 /*   180 */    57,   58,   59,   60,   60,    6,   63,   64,    6,   10,
 /*   190 */    46,    6,   67,   28,   29,   30,   31,   32,   46,   17,
 /*   200 */    27,   57,   58,   59,   60,   19,   74,   63,   64,   57,
 /*   210 */    58,   59,   60,    2,    3,   63,   64,    6,   46,   18,
 /*   220 */    35,    5,   73,   46,    9,    9,    2,    3,   28,   57,
 /*   230 */    58,   59,   60,   46,   62,   58,   59,   60,   82,   28,
 /*   240 */    29,   30,   31,   46,   57,   58,   59,   60,   82,   62,
 /*   250 */    82,   27,   82,   82,   82,   58,   59,   60,
};
#define YY_SHIFT_USE_DFLT (258)
#define YY_SHIFT_COUNT    (40)
#define YY_SHIFT_MIN      (-3)
#define YY_SHIFT_MAX      (224)
static const short yy_shift_ofst[] = {
 /*     0 */    80,   45,  128,  128,  128,  128,  128,   33,   33,  128,
 /*    10 */   128,  128,   23,  224,   33,   29,   29,   -3,   -3,   -3,
 /*    20 */    -3,   29,   -3,  133,  165,  211,   19,   75,  185,  179,
 /*    30 */   182,   29,  173,   29,  173,  186,   29,  201,  216,  215,
 /*    40 */   200,
};
#define YY_REDUCE_USE_DFLT (-59)
#define YY_REDUCE_COUNT (37)
#define YY_REDUCE_MIN   (-58)
#define YY_REDUCE_MAX   (197)
static const short yy_reduce_ofst[] = {
 /*     0 */   -37,   77,  123,  144,  152,  172,  187,   95,  -58,  177,
 /*    10 */   197,  197,   59,    2,   59,   84,  102,  124,  124,  124,
 /*    20 */   124,  120,  124,  -29,  -21,   17,   43,   85,   74,   82,
 /*    30 */    87,  121,  125,  121,  125,  132,  121,  149,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   395,  395,  360,  359,  305,  334,  329,  388,  339,  353,
 /*    10 */   332,  327,  344,  288,  346,  387,  361,  355,  297,  333,
 /*    20 */   328,  295,  296,  371,  368,  365,  350,  284,  287,  283,
 /*    30 */   275,  336,  394,  301,  302,  286,  294,  285,  392,  392,
 /*    40 */   281,
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
   10,  /*    LINE_HR => LINE_HTML */
    1,  /* LINE_SETEXT_1 => LINE_HR */
    1,  /* LINE_SETEXT_2 => LINE_HR */
    6,  /* LINE_PLAIN => LINE_CONTINUATION */
    4,  /* LINE_TABLE_SEPARATOR => LINE_PLAIN */
    0,  /* LINE_CONTINUATION => nothing */
    6,  /* LINE_INDENTED_TAB => LINE_CONTINUATION */
    6,  /* LINE_INDENTED_SPACE => LINE_CONTINUATION */
    6,  /* LINE_TABLE => LINE_CONTINUATION */
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
   10,  /* LINE_DEF_CITATION => LINE_HTML */
   10,  /* LINE_DEF_FOOTNOTE => LINE_HTML */
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
  "LINE_PLAIN",    "LINE_TABLE_SEPARATOR",  "LINE_CONTINUATION",  "LINE_INDENTED_TAB",
  "LINE_INDENTED_SPACE",  "LINE_TABLE",    "LINE_HTML",     "LINE_ATX_1",  
  "LINE_ATX_2",    "LINE_ATX_3",    "LINE_ATX_4",    "LINE_ATX_5",  
  "LINE_ATX_6",    "LINE_BLOCKQUOTE",  "LINE_LIST_BULLETED",  "LINE_LIST_ENUMERATED",
  "LINE_DEF_CITATION",  "LINE_DEF_FOOTNOTE",  "LINE_DEF_LINK",  "LINE_FENCE_BACKTICK",
  "LINE_FENCE_BACKTICK_START",  "LINE_TOC",      "LINE_DEF_ABBREVIATION",  "LINE_DEFINITION",
  "LINE_EMPTY",    "LINE_FENCE_BACKTICK_3",  "LINE_FENCE_BACKTICK_4",  "LINE_FENCE_BACKTICK_5",
  "LINE_FENCE_BACKTICK_START_3",  "LINE_FENCE_BACKTICK_START_4",  "LINE_FENCE_BACKTICK_START_5",  "LINE_META",   
  "error",         "doc",           "blocks",        "block",       
  "blockquote",    "def_abbreviation",  "def_citation",  "def_footnote",
  "def_link",      "definition_block",  "empty",         "fenced_block",
  "html_block",    "indented_code",  "list_bullet",   "list_enum",   
  "meta_block",    "para",          "setext_1",      "setext_2",    
  "table",         "chunk",         "chunk_line",    "nested_chunks",
  "nested_chunk",  "indented_line",  "ext_chunk",     "opt_ext_chunk",
  "tail",          "quote_line",    "defs",          "def",         
  "fenced_3",      "fenced_line",   "fenced_4",      "fenced_5",    
  "html_line",     "item_bullet",   "item_enum",     "meta_line",   
  "table_header",  "table_body",    "header_rows",   "table_section",
  "all_rows",      "row",         
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
 /*  15 */ "block ::= def_link",
 /*  16 */ "block ::= definition_block",
 /*  17 */ "block ::= empty",
 /*  18 */ "block ::= fenced_block",
 /*  19 */ "block ::= html_block",
 /*  20 */ "block ::= indented_code",
 /*  21 */ "block ::= list_bullet",
 /*  22 */ "block ::= list_enum",
 /*  23 */ "block ::= meta_block",
 /*  24 */ "block ::= para",
 /*  25 */ "block ::= setext_1",
 /*  26 */ "block ::= setext_2",
 /*  27 */ "block ::= table",
 /*  28 */ "chunk ::= chunk chunk_line",
 /*  29 */ "nested_chunks ::= nested_chunks nested_chunk",
 /*  30 */ "nested_chunk ::= empty indented_line chunk",
 /*  31 */ "nested_chunk ::= empty indented_line",
 /*  32 */ "ext_chunk ::= chunk nested_chunks",
 /*  33 */ "opt_ext_chunk ::= chunk nested_chunks",
 /*  34 */ "blockquote ::= blockquote quote_line",
 /*  35 */ "def_citation ::= LINE_DEF_CITATION tail",
 /*  36 */ "def_footnote ::= LINE_DEF_FOOTNOTE tail",
 /*  37 */ "def_link ::= LINE_DEF_LINK chunk",
 /*  38 */ "definition_block ::= para defs",
 /*  39 */ "defs ::= defs def",
 /*  40 */ "def ::= LINE_DEFINITION tail",
 /*  41 */ "def ::= LINE_DEFINITION",
 /*  42 */ "empty ::= empty LINE_EMPTY",
 /*  43 */ "fenced_block ::= fenced_3 LINE_FENCE_BACKTICK_3",
 /*  44 */ "fenced_block ::= fenced_3 LINE_FENCE_BACKTICK_4",
 /*  45 */ "fenced_block ::= fenced_3 LINE_FENCE_BACKTICK_5",
 /*  46 */ "fenced_3 ::= fenced_3 fenced_line",
 /*  47 */ "fenced_block ::= fenced_4 LINE_FENCE_BACKTICK_4",
 /*  48 */ "fenced_block ::= fenced_4 LINE_FENCE_BACKTICK_5",
 /*  49 */ "fenced_4 ::= fenced_4 fenced_line",
 /*  50 */ "fenced_4 ::= fenced_4 LINE_FENCE_BACKTICK_3",
 /*  51 */ "fenced_4 ::= fenced_4 LINE_FENCE_BACKTICK_START_3",
 /*  52 */ "fenced_block ::= fenced_5 LINE_FENCE_BACKTICK_5",
 /*  53 */ "fenced_5 ::= fenced_5 fenced_line",
 /*  54 */ "fenced_5 ::= fenced_5 LINE_FENCE_BACKTICK_3",
 /*  55 */ "fenced_5 ::= fenced_5 LINE_FENCE_BACKTICK_START_3",
 /*  56 */ "fenced_5 ::= fenced_5 LINE_FENCE_BACKTICK_4",
 /*  57 */ "fenced_5 ::= fenced_5 LINE_FENCE_BACKTICK_START_4",
 /*  58 */ "html_block ::= html_block html_line",
 /*  59 */ "indented_code ::= indented_code indented_line",
 /*  60 */ "indented_code ::= indented_code LINE_EMPTY",
 /*  61 */ "list_bullet ::= list_bullet item_bullet",
 /*  62 */ "item_bullet ::= LINE_LIST_BULLETED ext_chunk",
 /*  63 */ "item_bullet ::= LINE_LIST_BULLETED chunk",
 /*  64 */ "item_bullet ::= LINE_LIST_BULLETED nested_chunks",
 /*  65 */ "item_bullet ::= LINE_LIST_BULLETED",
 /*  66 */ "list_enum ::= list_enum item_enum",
 /*  67 */ "item_enum ::= LINE_LIST_ENUMERATED ext_chunk",
 /*  68 */ "item_enum ::= LINE_LIST_ENUMERATED chunk",
 /*  69 */ "item_enum ::= LINE_LIST_ENUMERATED nested_chunks",
 /*  70 */ "item_enum ::= LINE_LIST_ENUMERATED",
 /*  71 */ "meta_block ::= meta_block meta_line",
 /*  72 */ "para ::= LINE_PLAIN chunk",
 /*  73 */ "setext_1 ::= para LINE_SETEXT_1",
 /*  74 */ "setext_2 ::= para LINE_SETEXT_2",
 /*  75 */ "table ::= table_header table_body",
 /*  76 */ "table_header ::= header_rows LINE_TABLE_SEPARATOR",
 /*  77 */ "header_rows ::= header_rows LINE_TABLE",
 /*  78 */ "table_body ::= table_body table_section",
 /*  79 */ "table_section ::= all_rows LINE_EMPTY",
 /*  80 */ "table_section ::= all_rows",
 /*  81 */ "all_rows ::= all_rows row",
 /*  82 */ "para ::= all_rows",
 /*  83 */ "chunk ::= chunk_line",
 /*  84 */ "chunk_line ::= LINE_CONTINUATION",
 /*  85 */ "nested_chunks ::= nested_chunk",
 /*  86 */ "nested_chunk ::= empty",
 /*  87 */ "indented_line ::= LINE_INDENTED_TAB",
 /*  88 */ "indented_line ::= LINE_INDENTED_SPACE",
 /*  89 */ "opt_ext_chunk ::= chunk",
 /*  90 */ "tail ::= opt_ext_chunk",
 /*  91 */ "tail ::= nested_chunks",
 /*  92 */ "blockquote ::= LINE_BLOCKQUOTE",
 /*  93 */ "quote_line ::= LINE_BLOCKQUOTE",
 /*  94 */ "quote_line ::= LINE_CONTINUATION",
 /*  95 */ "def_citation ::= LINE_DEF_CITATION",
 /*  96 */ "def_footnote ::= LINE_DEF_FOOTNOTE",
 /*  97 */ "def_link ::= LINE_DEF_LINK",
 /*  98 */ "def_abbreviation ::= LINE_DEF_ABBREVIATION",
 /*  99 */ "defs ::= def",
 /* 100 */ "empty ::= LINE_EMPTY",
 /* 101 */ "fenced_block ::= fenced_3",
 /* 102 */ "fenced_3 ::= LINE_FENCE_BACKTICK_3",
 /* 103 */ "fenced_3 ::= LINE_FENCE_BACKTICK_START_3",
 /* 104 */ "fenced_block ::= fenced_4",
 /* 105 */ "fenced_4 ::= LINE_FENCE_BACKTICK_4",
 /* 106 */ "fenced_4 ::= LINE_FENCE_BACKTICK_START_4",
 /* 107 */ "fenced_block ::= fenced_5",
 /* 108 */ "fenced_5 ::= LINE_FENCE_BACKTICK_5",
 /* 109 */ "fenced_5 ::= LINE_FENCE_BACKTICK_START_5",
 /* 110 */ "fenced_line ::= LINE_CONTINUATION",
 /* 111 */ "fenced_line ::= LINE_EMPTY",
 /* 112 */ "fenced_line ::= LINE_SETEXT_1",
 /* 113 */ "fenced_line ::= LINE_SETEXT_2",
 /* 114 */ "html_block ::= LINE_HTML",
 /* 115 */ "html_line ::= LINE_CONTINUATION",
 /* 116 */ "html_line ::= LINE_HTML",
 /* 117 */ "indented_code ::= indented_line",
 /* 118 */ "list_bullet ::= item_bullet",
 /* 119 */ "list_enum ::= item_enum",
 /* 120 */ "meta_block ::= LINE_META",
 /* 121 */ "meta_line ::= LINE_META",
 /* 122 */ "meta_line ::= LINE_CONTINUATION",
 /* 123 */ "para ::= LINE_PLAIN",
 /* 124 */ "table ::= table_header",
 /* 125 */ "header_rows ::= LINE_TABLE",
 /* 126 */ "table_body ::= table_section",
 /* 127 */ "all_rows ::= row",
 /* 128 */ "row ::= header_rows",
 /* 129 */ "row ::= LINE_TABLE_SEPARATOR",
 /* 130 */ "para ::= defs",
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
  { 37, 1 },
  { 38, 2 },
  { 38, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 39, 1 },
  { 57, 2 },
  { 59, 2 },
  { 60, 3 },
  { 60, 2 },
  { 62, 2 },
  { 63, 2 },
  { 40, 2 },
  { 42, 2 },
  { 43, 2 },
  { 44, 2 },
  { 45, 2 },
  { 66, 2 },
  { 67, 2 },
  { 67, 1 },
  { 46, 2 },
  { 47, 2 },
  { 47, 2 },
  { 47, 2 },
  { 68, 2 },
  { 47, 2 },
  { 47, 2 },
  { 70, 2 },
  { 70, 2 },
  { 70, 2 },
  { 47, 2 },
  { 71, 2 },
  { 71, 2 },
  { 71, 2 },
  { 71, 2 },
  { 71, 2 },
  { 48, 2 },
  { 49, 2 },
  { 49, 2 },
  { 50, 2 },
  { 73, 2 },
  { 73, 2 },
  { 73, 2 },
  { 73, 1 },
  { 51, 2 },
  { 74, 2 },
  { 74, 2 },
  { 74, 2 },
  { 74, 1 },
  { 52, 2 },
  { 53, 2 },
  { 54, 2 },
  { 55, 2 },
  { 56, 2 },
  { 76, 2 },
  { 78, 2 },
  { 77, 2 },
  { 79, 2 },
  { 79, 1 },
  { 80, 2 },
  { 53, 1 },
  { 57, 1 },
  { 58, 1 },
  { 59, 1 },
  { 60, 1 },
  { 61, 1 },
  { 61, 1 },
  { 63, 1 },
  { 64, 1 },
  { 64, 1 },
  { 40, 1 },
  { 65, 1 },
  { 65, 1 },
  { 42, 1 },
  { 43, 1 },
  { 44, 1 },
  { 41, 1 },
  { 66, 1 },
  { 46, 1 },
  { 47, 1 },
  { 68, 1 },
  { 68, 1 },
  { 47, 1 },
  { 70, 1 },
  { 70, 1 },
  { 47, 1 },
  { 71, 1 },
  { 71, 1 },
  { 69, 1 },
  { 69, 1 },
  { 69, 1 },
  { 69, 1 },
  { 48, 1 },
  { 72, 1 },
  { 72, 1 },
  { 49, 1 },
  { 50, 1 },
  { 51, 1 },
  { 52, 1 },
  { 75, 1 },
  { 75, 1 },
  { 53, 1 },
  { 56, 1 },
  { 78, 1 },
  { 77, 1 },
  { 80, 1 },
  { 81, 1 },
  { 81, 1 },
  { 53, 1 },
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
      case 15: /* block ::= def_link */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_DEF_LINK); stack_push(engine->definition_stack, yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 16: /* block ::= definition_block */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_DEFLIST); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 17: /* block ::= empty */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_EMPTY); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 18: /* block ::= fenced_block */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_CODE_FENCED); yymsp[0].minor.yy0->child->type = CODE_FENCE; }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 19: /* block ::= html_block */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_HTML); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 20: /* block ::= indented_code */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_CODE_INDENTED); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 21: /* block ::= list_bullet */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_LIST_BULLETED); is_list_loose(yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 22: /* block ::= list_enum */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_LIST_ENUMERATED); is_list_loose(yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 23: /* block ::= meta_block */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_META); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 24: /* block ::= para */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_PARA); is_para_html(engine, yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 25: /* block ::= setext_1 */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_SETEXT_1); if (!(engine->extensions & EXT_NO_LABELS)) stack_push(engine->header_stack, yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 26: /* block ::= setext_2 */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_SETEXT_2); if (!(engine->extensions & EXT_NO_LABELS)) stack_push(engine->header_stack, yylhsminor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 27: /* block ::= table */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_TABLE); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 28: /* chunk ::= chunk chunk_line */
      case 29: /* nested_chunks ::= nested_chunks nested_chunk */ yytestcase(yyruleno==29);
      case 32: /* ext_chunk ::= chunk nested_chunks */ yytestcase(yyruleno==32);
      case 33: /* opt_ext_chunk ::= chunk nested_chunks */ yytestcase(yyruleno==33);
      case 34: /* blockquote ::= blockquote quote_line */ yytestcase(yyruleno==34);
      case 35: /* def_citation ::= LINE_DEF_CITATION tail */ yytestcase(yyruleno==35);
      case 36: /* def_footnote ::= LINE_DEF_FOOTNOTE tail */ yytestcase(yyruleno==36);
      case 37: /* def_link ::= LINE_DEF_LINK chunk */ yytestcase(yyruleno==37);
      case 39: /* defs ::= defs def */ yytestcase(yyruleno==39);
      case 42: /* empty ::= empty LINE_EMPTY */ yytestcase(yyruleno==42);
      case 46: /* fenced_3 ::= fenced_3 fenced_line */ yytestcase(yyruleno==46);
      case 49: /* fenced_4 ::= fenced_4 fenced_line */ yytestcase(yyruleno==49);
      case 50: /* fenced_4 ::= fenced_4 LINE_FENCE_BACKTICK_3 */ yytestcase(yyruleno==50);
      case 51: /* fenced_4 ::= fenced_4 LINE_FENCE_BACKTICK_START_3 */ yytestcase(yyruleno==51);
      case 53: /* fenced_5 ::= fenced_5 fenced_line */ yytestcase(yyruleno==53);
      case 54: /* fenced_5 ::= fenced_5 LINE_FENCE_BACKTICK_3 */ yytestcase(yyruleno==54);
      case 55: /* fenced_5 ::= fenced_5 LINE_FENCE_BACKTICK_START_3 */ yytestcase(yyruleno==55);
      case 56: /* fenced_5 ::= fenced_5 LINE_FENCE_BACKTICK_4 */ yytestcase(yyruleno==56);
      case 57: /* fenced_5 ::= fenced_5 LINE_FENCE_BACKTICK_START_4 */ yytestcase(yyruleno==57);
      case 58: /* html_block ::= html_block html_line */ yytestcase(yyruleno==58);
      case 59: /* indented_code ::= indented_code indented_line */ yytestcase(yyruleno==59);
      case 60: /* indented_code ::= indented_code LINE_EMPTY */ yytestcase(yyruleno==60);
      case 61: /* list_bullet ::= list_bullet item_bullet */ yytestcase(yyruleno==61);
      case 66: /* list_enum ::= list_enum item_enum */ yytestcase(yyruleno==66);
      case 71: /* meta_block ::= meta_block meta_line */ yytestcase(yyruleno==71);
      case 72: /* para ::= LINE_PLAIN chunk */ yytestcase(yyruleno==72);
      case 73: /* setext_1 ::= para LINE_SETEXT_1 */ yytestcase(yyruleno==73);
      case 74: /* setext_2 ::= para LINE_SETEXT_2 */ yytestcase(yyruleno==74);
      case 75: /* table ::= table_header table_body */ yytestcase(yyruleno==75);
      case 77: /* header_rows ::= header_rows LINE_TABLE */ yytestcase(yyruleno==77);
      case 78: /* table_body ::= table_body table_section */ yytestcase(yyruleno==78);
      case 81: /* all_rows ::= all_rows row */ yytestcase(yyruleno==81);
{ yylhsminor.yy0 = yymsp[-1].minor.yy0; token_chain_append(yymsp[-1].minor.yy0, yymsp[0].minor.yy0); }
  yymsp[-1].minor.yy0 = yylhsminor.yy0;
        break;
      case 30: /* nested_chunk ::= empty indented_line chunk */
{ yylhsminor.yy0 = yymsp[-2].minor.yy0; token_chain_append(yymsp[-2].minor.yy0, yymsp[-1].minor.yy0); token_chain_append(yymsp[-2].minor.yy0, yymsp[0].minor.yy0); yymsp[-1].minor.yy0->type = LINE_CONTINUATION; }
  yymsp[-2].minor.yy0 = yylhsminor.yy0;
        break;
      case 31: /* nested_chunk ::= empty indented_line */
{ yylhsminor.yy0 = yymsp[-1].minor.yy0; token_chain_append(yymsp[-1].minor.yy0, yymsp[0].minor.yy0); yymsp[0].minor.yy0->type = LINE_CONTINUATION; }
  yymsp[-1].minor.yy0 = yylhsminor.yy0;
        break;
      case 38: /* definition_block ::= para defs */
{ yylhsminor.yy0 = yymsp[-1].minor.yy0; token_chain_append(yymsp[-1].minor.yy0, yymsp[0].minor.yy0); yymsp[-1].minor.yy0->type = BLOCK_TERM; }
  yymsp[-1].minor.yy0 = yylhsminor.yy0;
        break;
      case 40: /* def ::= LINE_DEFINITION tail */
{ yylhsminor.yy0 = token_new_parent(yymsp[-1].minor.yy0, BLOCK_DEFINITION); token_chain_append(yymsp[-1].minor.yy0, yymsp[0].minor.yy0); recursive_parse_indent(engine, yylhsminor.yy0); }
  yymsp[-1].minor.yy0 = yylhsminor.yy0;
        break;
      case 41: /* def ::= LINE_DEFINITION */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_DEFINITION);  }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 43: /* fenced_block ::= fenced_3 LINE_FENCE_BACKTICK_3 */
      case 44: /* fenced_block ::= fenced_3 LINE_FENCE_BACKTICK_4 */ yytestcase(yyruleno==44);
      case 45: /* fenced_block ::= fenced_3 LINE_FENCE_BACKTICK_5 */ yytestcase(yyruleno==45);
      case 47: /* fenced_block ::= fenced_4 LINE_FENCE_BACKTICK_4 */ yytestcase(yyruleno==47);
      case 48: /* fenced_block ::= fenced_4 LINE_FENCE_BACKTICK_5 */ yytestcase(yyruleno==48);
      case 52: /* fenced_block ::= fenced_5 LINE_FENCE_BACKTICK_5 */ yytestcase(yyruleno==52);
{ yylhsminor.yy0 = yymsp[-1].minor.yy0; token_chain_append(yymsp[-1].minor.yy0, yymsp[0].minor.yy0); yymsp[0].minor.yy0->child->type = CODE_FENCE; }
  yymsp[-1].minor.yy0 = yylhsminor.yy0;
        break;
      case 62: /* item_bullet ::= LINE_LIST_BULLETED ext_chunk */
      case 64: /* item_bullet ::= LINE_LIST_BULLETED nested_chunks */ yytestcase(yyruleno==64);
      case 67: /* item_enum ::= LINE_LIST_ENUMERATED ext_chunk */ yytestcase(yyruleno==67);
      case 69: /* item_enum ::= LINE_LIST_ENUMERATED nested_chunks */ yytestcase(yyruleno==69);
{ yylhsminor.yy0 = token_new_parent(yymsp[-1].minor.yy0, BLOCK_LIST_ITEM); token_chain_append(yymsp[-1].minor.yy0, yymsp[0].minor.yy0); recursive_parse_list_item(engine, yylhsminor.yy0); }
  yymsp[-1].minor.yy0 = yylhsminor.yy0;
        break;
      case 63: /* item_bullet ::= LINE_LIST_BULLETED chunk */
      case 68: /* item_enum ::= LINE_LIST_ENUMERATED chunk */ yytestcase(yyruleno==68);
{ yylhsminor.yy0 = token_new_parent(yymsp[-1].minor.yy0, BLOCK_LIST_ITEM_TIGHT); token_chain_append(yymsp[-1].minor.yy0, yymsp[0].minor.yy0); recursive_parse_list_item(engine, yylhsminor.yy0); }
  yymsp[-1].minor.yy0 = yylhsminor.yy0;
        break;
      case 65: /* item_bullet ::= LINE_LIST_BULLETED */
      case 70: /* item_enum ::= LINE_LIST_ENUMERATED */ yytestcase(yyruleno==70);
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_LIST_ITEM_TIGHT); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 76: /* table_header ::= header_rows LINE_TABLE_SEPARATOR */
{ yylhsminor.yy0 = token_new_parent(yymsp[-1].minor.yy0, BLOCK_TABLE_HEADER); token_chain_append(yymsp[-1].minor.yy0, yymsp[0].minor.yy0); }
  yymsp[-1].minor.yy0 = yylhsminor.yy0;
        break;
      case 79: /* table_section ::= all_rows LINE_EMPTY */
{ yylhsminor.yy0 = token_new_parent(yymsp[-1].minor.yy0, BLOCK_TABLE_SECTION); token_chain_append(yymsp[-1].minor.yy0, yymsp[0].minor.yy0); }
  yymsp[-1].minor.yy0 = yylhsminor.yy0;
        break;
      case 80: /* table_section ::= all_rows */
{ yylhsminor.yy0 = token_new_parent(yymsp[0].minor.yy0, BLOCK_TABLE_SECTION); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 82: /* para ::= all_rows */
{ yylhsminor.yy0 = yymsp[0].minor.yy0; }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      default:
      /* (83) chunk ::= chunk_line (OPTIMIZED OUT) */ assert(yyruleno!=83);
      /* (84) chunk_line ::= LINE_CONTINUATION */ yytestcase(yyruleno==84);
      /* (85) nested_chunks ::= nested_chunk (OPTIMIZED OUT) */ assert(yyruleno!=85);
      /* (86) nested_chunk ::= empty */ yytestcase(yyruleno==86);
      /* (87) indented_line ::= LINE_INDENTED_TAB */ yytestcase(yyruleno==87);
      /* (88) indented_line ::= LINE_INDENTED_SPACE */ yytestcase(yyruleno==88);
      /* (89) opt_ext_chunk ::= chunk */ yytestcase(yyruleno==89);
      /* (90) tail ::= opt_ext_chunk (OPTIMIZED OUT) */ assert(yyruleno!=90);
      /* (91) tail ::= nested_chunks */ yytestcase(yyruleno==91);
      /* (92) blockquote ::= LINE_BLOCKQUOTE */ yytestcase(yyruleno==92);
      /* (93) quote_line ::= LINE_BLOCKQUOTE */ yytestcase(yyruleno==93);
      /* (94) quote_line ::= LINE_CONTINUATION */ yytestcase(yyruleno==94);
      /* (95) def_citation ::= LINE_DEF_CITATION */ yytestcase(yyruleno==95);
      /* (96) def_footnote ::= LINE_DEF_FOOTNOTE */ yytestcase(yyruleno==96);
      /* (97) def_link ::= LINE_DEF_LINK */ yytestcase(yyruleno==97);
      /* (98) def_abbreviation ::= LINE_DEF_ABBREVIATION */ yytestcase(yyruleno==98);
      /* (99) defs ::= def (OPTIMIZED OUT) */ assert(yyruleno!=99);
      /* (100) empty ::= LINE_EMPTY */ yytestcase(yyruleno==100);
      /* (101) fenced_block ::= fenced_3 */ yytestcase(yyruleno==101);
      /* (102) fenced_3 ::= LINE_FENCE_BACKTICK_3 */ yytestcase(yyruleno==102);
      /* (103) fenced_3 ::= LINE_FENCE_BACKTICK_START_3 */ yytestcase(yyruleno==103);
      /* (104) fenced_block ::= fenced_4 */ yytestcase(yyruleno==104);
      /* (105) fenced_4 ::= LINE_FENCE_BACKTICK_4 */ yytestcase(yyruleno==105);
      /* (106) fenced_4 ::= LINE_FENCE_BACKTICK_START_4 */ yytestcase(yyruleno==106);
      /* (107) fenced_block ::= fenced_5 */ yytestcase(yyruleno==107);
      /* (108) fenced_5 ::= LINE_FENCE_BACKTICK_5 */ yytestcase(yyruleno==108);
      /* (109) fenced_5 ::= LINE_FENCE_BACKTICK_START_5 */ yytestcase(yyruleno==109);
      /* (110) fenced_line ::= LINE_CONTINUATION */ yytestcase(yyruleno==110);
      /* (111) fenced_line ::= LINE_EMPTY */ yytestcase(yyruleno==111);
      /* (112) fenced_line ::= LINE_SETEXT_1 */ yytestcase(yyruleno==112);
      /* (113) fenced_line ::= LINE_SETEXT_2 */ yytestcase(yyruleno==113);
      /* (114) html_block ::= LINE_HTML */ yytestcase(yyruleno==114);
      /* (115) html_line ::= LINE_CONTINUATION */ yytestcase(yyruleno==115);
      /* (116) html_line ::= LINE_HTML */ yytestcase(yyruleno==116);
      /* (117) indented_code ::= indented_line (OPTIMIZED OUT) */ assert(yyruleno!=117);
      /* (118) list_bullet ::= item_bullet (OPTIMIZED OUT) */ assert(yyruleno!=118);
      /* (119) list_enum ::= item_enum (OPTIMIZED OUT) */ assert(yyruleno!=119);
      /* (120) meta_block ::= LINE_META */ yytestcase(yyruleno==120);
      /* (121) meta_line ::= LINE_META */ yytestcase(yyruleno==121);
      /* (122) meta_line ::= LINE_CONTINUATION */ yytestcase(yyruleno==122);
      /* (123) para ::= LINE_PLAIN */ yytestcase(yyruleno==123);
      /* (124) table ::= table_header */ yytestcase(yyruleno==124);
      /* (125) header_rows ::= LINE_TABLE */ yytestcase(yyruleno==125);
      /* (126) table_body ::= table_section (OPTIMIZED OUT) */ assert(yyruleno!=126);
      /* (127) all_rows ::= row (OPTIMIZED OUT) */ assert(yyruleno!=127);
      /* (128) row ::= header_rows */ yytestcase(yyruleno==128);
      /* (129) row ::= LINE_TABLE_SEPARATOR */ yytestcase(yyruleno==129);
      /* (130) para ::= defs */ yytestcase(yyruleno==130);
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
