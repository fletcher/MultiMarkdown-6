/**

	MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file token_pairs.c

	@brief	Allow for pairing certain tokens together (e.g. '[' and ']') to create
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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stack.h"
#include "token.h"
#include "token_pairs.h"


/// Create a new token pair engine
token_pair_engine * token_pair_engine_new(void) {
	token_pair_engine * e = malloc(sizeof(token_pair_engine));

	if (e) {
		unsigned short empty[kMaxTokenTypes] = {0};
		unsigned short empty2[kMaxTokenTypes][kMaxTokenTypes] = {{0}};

		memcpy(e->can_open_pair, empty, sizeof(unsigned short) * kMaxTokenTypes);
		memcpy(e->can_close_pair, empty, sizeof(unsigned short) * kMaxTokenTypes);

		memcpy(e->pair_type, empty2, sizeof(unsigned short) * kMaxTokenTypes * kMaxTokenTypes);

		memcpy(e->empty_allowed, empty, sizeof(unsigned short) * kMaxTokenTypes);
		memcpy(e->match_len, empty, sizeof(unsigned short) * kMaxTokenTypes);
		memcpy(e->should_prune, empty, sizeof(unsigned short) * kMaxTokenTypes);
	}

	return e;
}


/// Free existing token pair engine
void token_pair_engine_free(token_pair_engine * e) {
	if (e == NULL) {
		return;
	}

	free(e);
}


/// Add a new pairing configuration to a token pair engine
void token_pair_engine_add_pairing(token_pair_engine * e, unsigned short open_type, unsigned short close_type,
								   unsigned short pair_type, int options) {
	// \todo: This needs to be more sophisticated
	e->can_open_pair[open_type] = 1;
	e->can_close_pair[close_type] = 1;
	(e->pair_type)[open_type][close_type] = pair_type;

	if (options & PAIRING_ALLOW_EMPTY) {
		e->empty_allowed[pair_type] = true;
	}

	if (options & PAIRING_MATCH_LENGTH) {
		e->match_len[pair_type] = true;
	}

	if (options & PAIRING_PRUNE_MATCH) {
		e->should_prune[pair_type] = true;
	}

}


/// Mate opener and closer together
void token_pair_mate(token * a, token * b) {
	if (a == NULL | b  == NULL) {
		return;
	}

	a->mate = b;
	a->unmatched = false;

	b->mate = a;
	b->unmatched = false;
}


/// Search a token's childen for matching pairs
void token_pairs_match_pairs_inside_token(token * parent, token_pair_engine * e, stack * s, unsigned short depth) {

	// Avoid stack overflow in "pathologic" input
	if (depth == kMaxPairRecursiveDepth) {
		return;
	}

	// Walk the child chain
	token * walker = parent->child;

	// Counter
	size_t start_counter = s->size;
	size_t i;				// We're sharing one stack, so any opener earlier than this belongs to a parent

	token * peek;
	unsigned short pair_type;

	unsigned int opener_count[kMaxTokenTypes] = {0};	// Keep track of which token types are on the stack

	while (walker != NULL) {

		if (walker->child) {
			token_pairs_match_pairs_inside_token(walker, e, s, depth + 1);
		}

		// Is this a closer?
		if (walker->can_close && e->can_close_pair[walker->type] && walker->unmatched ) {
			i = s->size;

			// Do we even have a valid opener in the stack?
			// It's only worth checking if the stack is beyond a certain size
			if (i > start_counter + kLargeStackThreshold) {
				for (int j = 0; j < kMaxTokenTypes; ++j) {
					if (opener_count[j]) {
						if (e->pair_type[j][walker->type]) {
							goto close;
						}
					}
				}

				// No opener available for this as closer
				goto open;
			}

close:

			// Find matching opener for this closer
			while (i > start_counter) {
				peek = stack_peek_index(s, i - 1);

				pair_type = e->pair_type[peek->type][walker->type];

				if (pair_type) {
					if (!e->empty_allowed[pair_type]) {
						// Make sure they aren't consecutive tokens
						if ((peek->next == walker) &&
								(peek->start + peek->len == walker->start)) {
							// i--;
							i = start_counter;	// In this situation, we can't use this token as a closer
							continue;
						}
					}

					if (e->match_len[pair_type]) {
						// Lengths must match
						if (peek->len != walker->len) {
							i--;
							continue;
						}
					}

					token_pair_mate(peek, walker);


					// Clear portion of stack between opener and closer as they are now unavailable for mating
					while (s->size > (i - 1)) {
						peek = stack_pop(s);
						opener_count[peek->type]--;
					}

					#ifndef NDEBUG
					fprintf(stderr, "stack now sized %lu\n", s->size);
					#endif
					// Prune matched section

					if (e->should_prune[pair_type]) {
						if (peek->prev == NULL) {
							walker = token_prune_graft(peek, walker, e->pair_type[peek->type][walker->type]);
							parent->child = walker;
						} else {
							walker = token_prune_graft(peek, walker, e->pair_type[peek->type][walker->type]);
						}
					}

					break;
				}

				#ifndef NDEBUG
				else {
					fprintf(stderr, "token type %d failed to match stack element\n", walker->type);
				}

				#endif
				i--;
			}
		}

open:

		// Is this an opener?
		if (walker->can_open && e->can_open_pair[walker->type] && walker->unmatched) {
			stack_push(s, walker);
			opener_count[walker->type]++;
			#ifndef NDEBUG
			fprintf(stderr, "push token type %d to stack (%lu elements)\n", walker->type, s->size);
			#endif
		}

		walker = walker->next;
	}

	#ifndef NDEBUG
	fprintf(stderr, "token stack has %lu elements (of %lu)\n", s->size, s->capacity);
	#endif

	// Remove unused tokens from stack and return to parent
	s->size = start_counter;
}
