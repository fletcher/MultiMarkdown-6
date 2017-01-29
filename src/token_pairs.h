/**

	MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file token_pairs.h

	@brief  Allow for pairing certain tokens together (e.g. '[' and ']') to create
	more meaningful token trees.


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


#ifndef TOKEN_PAIRS_MULTIMARKDOWN_H
#define TOKEN_PAIRS_MULTIMARKDOWN_H

#include "stack.h"
#include "token.h"


#ifdef TEST
#include "CuTest.h"
#endif

#define kMaxTokenTypes	200			// This needs to be larger than the largest token type being used
#define kLargeStackThreshold 1000	// Avoid unnecessary searches of large stacks


/// Store information about which tokens can be paired, and what actions to take when 
/// pairing them.
struct token_pair_engine {
	unsigned short		can_open_pair[kMaxTokenTypes];				//!< Can token type open a pair?
	unsigned short		can_close_pair[kMaxTokenTypes];				//!< Can token type close a pair?

	unsigned short		pair_type[kMaxTokenTypes][kMaxTokenTypes];	//!< Which pair are we forming?

	unsigned short		empty_allowed[kMaxTokenTypes];				//!< Is this pair type allowed to be empty?
	unsigned short		match_len[kMaxTokenTypes];					//!< Does this pair type require matched lengths of openers/closers?
	unsigned short		should_prune[kMaxTokenTypes];				//!< Does this pair type need to be pruned to a child token chain?
};

typedef struct token_pair_engine token_pair_engine;


/// Flags for token pair options
enum pairings_options {
	PAIRING_ALLOW_EMPTY		= 1 << 0,		//!< Allow consecutive tokens to match with each other
	PAIRING_MATCH_LENGTH	= 1 << 1,		//!< Require that opening/closing tokens be same length
	PAIRING_PRUNE_MATCH		= 1 << 2,		//!< Move the matched sub-chain into a child chain
};


/// Create a new token pair engine
token_pair_engine * token_pair_engine_new(void);

/// Free existing token pair engine
void token_pair_engine_free(
	token_pair_engine * e					//!< Token pair engine to be freed
);

/// Add a new pairing configuration to a token pair engine
void token_pair_engine_add_pairing(
	token_pair_engine * e,					//!< Token pair engine to add to
	unsigned short open_type,				//!< Token type for opener
	unsigned short close_type,				//!< Token type for closer
	unsigned short pair_type,				//!< Token type for pairing
	int options								//!< Token pair options to use
);

/// Search a token's childen for matching pairs
void token_pairs_match_pairs_inside_token(
	token * parent,							//!< Which tokens should we search for pairs
	token_pair_engine * e,					//!< Token pair engine to be used for matching
	stack * s								//!< Pointer to a stack to use for pairing tokens
);


#endif
