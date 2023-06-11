/**

	Parser-Template -- Boilerplate parser example using re2c lexer and lemon parser.

	@file token.h

	@brief Structure and functions to manage tokens representing portions of a
	text string.


	@author	Fletcher T. Penney

	@bug

**/

/*

	Copyright © 2016 - 2017 Fletcher T. Penney.


	The `MultiMarkdown 6` project is released under the MIT License..

	GLibFacade.c and GLibFacade.h are from the MultiMarkdown v4 project:

		https://github.com/fletcher/MultiMarkdown-4/

	MMD 4 is released under both the MIT License and GPL.


	CuTest is released under the zlib/libpng license. See CuTest.c for the text
	of the license.


	## The MIT License ##

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.

*/

#ifndef TOKEN_PARSER_TEMPLATE_H
#define TOKEN_PARSER_TEMPLATE_H


#ifdef DISABLE_OBJECT_POOL
	#undef kUseObjectPool
#else
	#define kUseObjectPool 1
#endif
//!< Use an object pool to allocate tokens to improve
//!< performance in memory allocation. Frees all
//!< tokens at once, however, at end of parsing.

/// Should call init() once per thread/use, and drain() once per thread/use.
/// This allows us to know when the pool is no longer being used and it is safe
/// to free.

/// This is easy with a command line utility, but complex in a multithreaded
/// application.  Unless you *really* know what you're doing, fully understand
/// threads and C memory management, you should probably disable object pools
/// when creating a long-running GUI application.  (I disable them in
/// MultiMarkdown Composer, for example.)

#ifdef kUseObjectPool
	void token_pool_init(void);			//!< Initialize object pool for allocating tokens
	void token_pool_drain(void);		//!< Drain pool to free memory when parse complete
	void token_pool_free(void);			//!< Free the token object pool
#endif


/// Definition for token node struct.  This can be used to match an
/// abstract syntax tree with the appropriate spans in the original
/// source string.
struct token {
	unsigned short		type;			//!< Type for the token
	short				can_open;		//!< Can token open a matched pair?
	short				can_close;		//!< Can token close a matched pair?
	short				unmatched;		//!< Has token been matched yet?

	size_t				start;			//!< Starting offset in the source string
	size_t				len;			//!< Length of the token in the source string

	size_t				out_start;
	size_t				out_len;

	struct token 	*	next;			//!< Pointer to next token in the chain
	struct token 	*	prev;			//!< Pointer to previous marker in the chain
	struct token 	*	child;			//!< Pointer to child chain

	struct token 	*	tail;			//!< Pointer to last token in the chain

	struct token 	*	mate;			//!< Pointer to other token in matched pair
};

typedef struct token token;


/// Get pointer to a new token
token * token_new(
	unsigned short type,				//!< Type for new token
	size_t start,						//!< Starting offset for token
	size_t len							//!< Len of token
);

/// Duplicate an existing token
token * token_copy(
	token * original					//!< Token to be copied
);

/// Create a parent for a chain of tokens
token * token_new_parent(
	token * child,						//!< Pointer to child token chain
	unsigned short type					//!< Type for new token
);

/// Add a new token to the end of a token chain.  The new token
/// may or may not also be the start of a chain
void token_chain_append(
	token * chain_start,				//!< Pointer to start of token chain
	token * t							//!< Pointer to token to append
);

/// Add a new token to the end of a parent's child
/// token chain.  The new token may or may not be
/// the start of a chain.
void token_append_child(
	token * parent,						//!< Pointer to parent node
	token * t							//!< Pointer to token to append
);

/// Remove the first child of a token
void token_remove_first_child(
	token * parent						//!< Pointer to parent node
);

/// Remove the last child of a token
void token_remove_last_child(
	token * parent						//!< Pointer to parent node
);

/// Remove the last token in a chain
void token_remove_tail(token * head);

/// Pop token out of it's chain, connecting head and tail of chain back together.
/// Token must be freed if it is no longer needed.
void token_pop_link_from_chain(
	token * t							//!< Pointer to token to remove
);

/// Remove one or more tokens from chain
void tokens_prune(
	token * first,						//!< Pointer to first node to be removed
	token * last						//!< Pointer to last node to be removed
);

/// Given a start/stop point in token chain, create a new parent token.
/// Reinsert the new parent in place of the removed segment.
/// Return pointer to new container token.
token * token_prune_graft(
	token * first,						//!< Pointer to first node to be removed
	token * last,						//!< Pointer to last node to be removed
	unsigned short container_type		//!< Type for new parent node for removed section
);

/// Free token
void token_free(
	token * t							//!< Pointer to token to be freed
);

/// Free token tree
void token_tree_free(
	token * t							//!< Pointer to token to be freed
);

/// Print a description of the token based on specified string
void token_describe(
	token * t,							//!< Pointer to token to described
	const char * string					//!< Source string
);

/// Print a description of the token tree based on specified string
void token_tree_describe(
	token * t,							//!< Pointer to token to described
	const char * string					//!< Source string
);

/// Find the child node of a given parent that contains the specified
/// offset position.
token * token_child_for_offset(
	token * parent,						//!< Pointer to parent token
	size_t offset						//!< Search position
);

/// Find first child node of a given parent that intersects the specified
/// offset range.
token * token_first_child_in_range(
	token * parent,						//!< Pointer to parent token
	size_t start,						//!< Start search position
	size_t len							//!< Search length
);

/// Find last child node of a given parent that intersects the specified
/// offset range.
token * token_last_child_in_range(
	token * parent,						//!< Pointer to parent token
	size_t start,						//!< Start search position
	size_t len							//!< Search length
);

void token_trim_leading_whitespace(token * t, const char * string);

void token_trim_trailing_whitespace(token * t, const char * string);

void token_trim_whitespace(token * t, const char * string);


///
token * token_chain_accept(token ** t, unsigned short type);

token * token_chain_accept_multiple(token ** t, int n, ...);

void token_skip_until_type(token ** t, unsigned short type);

void token_skip_until_type_multiple(token ** t, int n, ...);

void token_split_on_char(token * t, const char * source, const char c);

void token_split(token * t, size_t start, size_t len, unsigned short new_type);

#endif

