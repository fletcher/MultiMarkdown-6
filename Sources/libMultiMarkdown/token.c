/**

	Parser-Template -- Boilerplate parser example using re2c lexer and lemon parser.

	@file token.c

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

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "char.h"
#include "token.h"


#ifdef kUseObjectPool
//!< Use an object pool to allocate tokens more efficiently to improve
//!< performance.

#include "object_pool.h"

static pool * token_pool = NULL;		//!< Pointer to our object pool

/// Count number of uses of this pool to allow us know
/// when it's safe to drain the pool
static short token_pool_count = 0;

/// Intialize object pool for token allocation
void token_pool_init(void) {
	if (token_pool == NULL) {
		// No pool exists
		token_pool = pool_new(sizeof(token));
	}

	// Increment counter
	token_pool_count++;
}


/// Drain token allocator pool to prepare for another parse
void token_pool_drain(void) {
	// Decrement counter
	token_pool_count--;

	if (token_pool_count == 0)
		pool_drain(token_pool);
}


/// Free token allocator pool
void token_pool_free(void) {
	if (token_pool_count == 0) {
		pool_free(token_pool);
		token_pool = NULL;
	} else {
		fprintf(stderr, "ERROR: Attempted to drain token pool while still in use.\n");
	}
}

#endif


/// Get pointer to a new token
token * token_new(unsigned short type, size_t start, size_t len) {


#ifdef kUseObjectPool
	token * t = pool_allocate_object(token_pool);
#else
	token * t = malloc(sizeof(token));
#endif

	if (t) {
		t->type = type;
		t->start = start;
		t->len = len;

		t->next = NULL;
		t->prev = NULL;
		t->child = NULL;

		t->tail = t;

		t->can_open = true;			//!< Default to true -- we assume openers can open and closers can close
		t->can_close = true;		//!< unless specified otherwise (e.g. for ambidextrous tokens)
		t->unmatched = true;

		t->mate = NULL;
	}

	return t;
}


/// Create a parent for a chain of tokens
token * token_new_parent(token * child, unsigned short type) {
	if (child == NULL) {
		return token_new(type, 0, 0);
	}
	
	token * t = token_new(type, child->start, 0);
	t->child = child;
	child->prev = NULL;

	// Ensure that parent length correctly includes children
	if (child == NULL) {
		t->len = 0;
	} else if (child->next == NULL) {
		t->len = child->len;
	} else {
		while (child->next != NULL)
			child = child->next;

		t->len = child->start + child->len - t->start;
	}

	return t;
}


/// Add a new token to the end of a token chain.  The new token
/// may or may not also be the start of a chain
void token_chain_append(token * chain_start, token * t) {
	if ((chain_start == NULL) ||
		(t == NULL))
		return;

	// Append t
	chain_start->tail->next = t;
	t->prev = chain_start->tail;

	// Adjust tail marker
	chain_start->tail = t->tail;
}


/// Add a new token to the end of a parent's child
/// token chain.  The new token may or may not be
/// the start of a chain.
void token_append_child(token * parent, token * t) {
	if ((parent == NULL) || (t == NULL))
		return;

	if (parent->child == NULL) {
		// Parent has no children
		parent->child = t;
	} else {
		// Append to to existing child chain
		token_chain_append(parent->child, t);
	}

	// Set len on parent
	parent->len = parent->child->tail->start + parent->child->tail->len - parent->start;
}


/// Remove the first child of a token
void token_remove_first_child(token * parent) {
	if ((parent == NULL) || (parent->child == NULL))
		return;

	token * t = parent->child;
	parent->child = t->next;

	if (parent->child) {
		parent->child->prev = NULL;
		parent->child->tail = t->tail;
	}

	token_free(t);
}


/// Remove the last child of a token
void token_remove_last_child(token * parent) {
	if ((parent == NULL) || (parent->child == NULL))
		return;

	token * t = parent->child->tail;

	if (t->prev) {
		t->prev->next = NULL;
		parent->child->tail = t->prev;
	}

	token_free(t);
}


/// Remove the last token in a chain
void token_remove_tail(token * head) {
	if ((head == NULL) || (head->tail == head))
		return;

	token * t = head->tail;

	if (t->prev) {
		t->prev->next = NULL;
		head->tail = t->prev;
	}

	token_free(t);
}


/// Pop token out of it's chain, connecting head and tail of chain back together.
/// Token must be freed if it is no longer needed.
/// \todo: If t is the tail token of a chain, the tail is no longer correct on the start of chain.
void token_pop_link_from_chain(token * t) {
	if (t == NULL)
		return;

	token * prev = t->prev;
	token * next = t->next;

	t->next = NULL;
	t->prev = NULL;
	t->tail = t;

	if (prev) {
		prev->next = next;
	}

	if (next) {
		next->prev = prev;
	}
}


/// Remove one or more tokens from chain
void tokens_prune(token * first, token * last) {
	if (first == NULL || last == NULL)
		return;

	token * prev = first->prev;
	token * next = last->next;

	if (prev != NULL)
		prev->next = next;
	
	if (next != NULL)
		next->prev = prev;

    first->prev = NULL;
    last->next = NULL;
    
	token_tree_free(first);
}


/// Given a start/stop point in token chain, create a new container token.
/// Return pointer to new container token.
token * token_prune_graft(token * first, token * last, unsigned short container_type) {
	if (first == NULL || last == NULL)
		return first;

	token * prev = first->prev;
	token * next = last->next;
	
	// If we are head of chain, remember tail
	token * tail = NULL;
	if (prev == NULL)
		tail = first->tail;


	token * container = token_new(container_type, first->start, last->start + last->len - first->start);
	
	container->child = first;
	container->next = next;
	container->prev = prev;
	container->can_close = 0;
	container->can_open = 0;

	if (tail)
		container->tail = tail;

	if (prev)
		prev->next = container;

	first->prev = NULL;

	last->next = NULL;

	if (next)
		next->prev = container;

	return container;
}


/// Free token
void token_free(token * t) {
#ifdef kUseObjectPool
	return;
#else
	if (t == NULL)
		return;
	
	token_tree_free(t->child);

	free(t);
#endif
}


/// Free token chain
void token_tree_free(token * t) {
#ifdef kUseObjectPool
	return;
#else
	token * n;

	while (t != NULL) {
		n = t->next;
		token_free(t);

		t = n;
	}
#endif
}


/// Forward declaration
void print_token_tree(token * t, unsigned short depth, const char * string);


/// Print contents of the token based on specified string
void print_token(token * t, unsigned short depth, const char * string) {
	if (t != NULL) {
		for (int i = 0; i < depth; ++i)
		{
			fprintf(stderr, "\t");
		}
		if (string == NULL) {
			fprintf(stderr, "* (%d) %lu:%lu\n", t->type, t->start, t->len);
		} else {
			fprintf(stderr, "* (%d) %lu:%lu\t'%.*s'\n", t->type, t->start, t->len, (int)t->len, &string[t->start]);
		}

		if (t->child != NULL)
			print_token_tree(t->child, depth + 1, string);
	}
}


/// Print contents of the token tree based on specified string
void print_token_tree(token * t, unsigned short depth, const char * string) {
	while (t != NULL) {
		print_token(t, depth, string);

		t = t->next;
	}
}


/// Print a description of the token based on specified string
void token_describe(token * t, const char * string) {
	print_token(t, 0, string);
}


/// Print a description of the token tree based on specified string
void token_tree_describe(token * t, const char * string) {
	fprintf(stderr, "=====>\n");
	while (t != NULL) {
		print_token(t, 0, string);

		t = t->next;
	}
	fprintf(stderr, "<=====\n");
}


/// Find the child node of a given parent that contains the specified
/// offset position.
token * token_child_for_offset(
	token * parent,						//!< Pointer to parent token
	size_t offset						//!< Search position
) {
	if (parent == NULL)
		return NULL;

	if ((parent->start > offset) ||
		(parent->start + parent->len < offset))
		return NULL;

	token * walker = parent->child;

	while (walker != NULL) {
		if (walker->start <= offset) {
			if (walker->start + walker->len > offset) {
				return walker;
			}
		}
		if (walker->start > offset)
			return NULL;
		
		walker = walker->next;
	}

	return NULL;
}


/// Given two character ranges, see if they intersect (touching doesn't count)
static bool ranges_intersect(size_t start1, size_t len1, size_t start2, size_t len2) {
	return ((start1 < start2 + len2) && (start2 < start1 + len1)) ? true : false;
}

/// Find first child node of a given parent that intersects the specified
/// offset range.
token * token_first_child_in_range(
	token * parent,						//!< Pointer to parent token
	size_t start,						//!< Start search position
	size_t len							//!< Search length
) {
	if (parent == NULL)
		return NULL;

	if ((parent->start > start + len) ||
		(parent->start + parent->len < start))
		return NULL;

	token * walker = parent->child;

	while (walker != NULL) {
		if (ranges_intersect(start, len, walker->start, walker->len))
			return walker;

		if (walker->start > start)
			return NULL;

		walker = walker->next;
	}

	return NULL;
}


/// Find last child node of a given parent that intersects the specified
/// offset range.
token * token_last_child_in_range(
	token * parent,						//!< Pointer to parent token
	size_t start,						//!< Start search position
	size_t len							//!< Search length
) {
	if (parent == NULL)
		return NULL;

	if ((parent->start > start + len) ||
		(parent->start + parent->len < start))
		return NULL;

	token * walker = parent->child;
	token * last = NULL;

	while (walker != NULL) {
		if (ranges_intersect(start, len, walker->start, walker->len))
			last = walker;

		if (walker->start > start + len)
			return last;

		walker = walker->next;
	}

	return last;
}


void token_trim_leading_whitespace(token * t, const char * string) {
	while (t->len && char_is_whitespace(string[t->start])) {
		t->start++;
		t->len--;
	}
}


void token_trim_trailing_whitespace(token * t, const char * string) {
	while (t->len && char_is_whitespace(string[t->start + t->len - 1])) {
		t->len--;
	}
}


void token_trim_whitespace(token * t, const char * string) {
	token_trim_leading_whitespace(t, string);
	token_trim_trailing_whitespace(t, string);
}


/// Check whether first token in the chain matches the given type.
/// If so, return and advance the chain.
token * token_chain_accept(token ** t, short type) {
	token * result = NULL;

	if (t && *t && ((*t)->type == type)) {
		result = *t;
		*t = (*t)->next;
	}

	return result;
}


/// Allow checking for multiple token types
token * token_chain_accept_multiple(token ** t, int n, ...) {
	token * result = NULL;
	va_list valist;

	va_start(valist, n);

	for (int i = 0; i < n; ++i)
	{
		result = token_chain_accept(t, va_arg(valist, int));
		if (result)
			break;
	}

	va_end(valist);

	return result;
}


void token_skip_until_type(token ** t, short type) {
	while ((*t) && ((*t)->type != type))
		*t = (*t)->next;
}


/// Allow checking for multiple token types
void token_skip_until_type_multiple(token ** t, int n, ...) {
	va_list valist;
	int type[n];

	va_start(valist, n);

	// Load target types
	for (int i = 0; i < n; ++i)
	{
		type[i] = va_arg(valist, int);
	}

	// 
	while (*t) {
		for (int i = 0; i < n; ++i)
		{
			if ((*t)->type == type[i])
				return;
		}

		*t = (*t)->next;
	}

	va_end(valist);
}


void token_split_on_char(token * t, const char * source, const char c) {
	if (!t)
		return;

	size_t start = t->start;
	size_t pos = 0;
	size_t stop = t->len;
	token * new = NULL;

	while (pos + 1 < stop) {
		if (source[start + pos] == c){
			new = token_new(t->type, start + pos + 1, stop - (pos + 1));
			new->next = t->next;
			t->next = new;
			
			t->len = pos;
			
			t = t->next;
		}

		pos++;
	}
}


// Split a token and create new ones as needed
void token_split(token * t, size_t start, size_t len, unsigned short new_type) {
	if (!t)
		return;

	size_t stop = start + len;

	if (start < t->start)
		return;

	if (stop > t->start + t->len)
		return;

	token * A;		// This will be new token
	bool inset_start = false;
	bool inset_stop = false;

	// Will we need a leading token?
	if (start > t->start)
		inset_start = true;

	// Will we need a lagging token?
	if (stop < t->start + t->len)
		inset_stop = true;


	if (inset_start) {
		A = token_new(new_type, start, len);
		if (inset_stop) {
			// We will end up with t->A->T2

			// Create T2
			token * T2 = token_new(t->type, stop, t->start + t->len - stop);
			T2->next = t->next;
	
			if (t->next)
				t->next->prev = T2;

			A->next = T2;
			T2->prev = A;
		} else {
			// We will end up with T->A
			A->next = t->next;

			if (t->next)
				t->next->prev = A;
		}

		t->next = A;
		A->prev = t;

		t->len = start - t->start;
	} else {
		if (inset_stop) {
			// We will end up with A->T
			// But we swap the tokens to ensure we don't
			// cause difficulty pointing to this chain,
			// resulting in T->A, where T is the new type
			A = token_new(t->type, stop, t->start + t->len - stop);
			A->prev = t;
			A->next = t->next;
			t->next = A;

			if (A->next)
				A->next->prev = A;

			t->len = stop - t->start;
			t->type = new_type;
		} else {
			// We will end up with A
			t->type = new_type;
		}
	}
}

