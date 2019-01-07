/**

	C-Template -- Boilerplate c project with cmake support, CuTest unit testing, and more.

	@file aho-corasick.h

	@brief


	@author	Fletcher T. Penney
	@bug

**/

/*

	Copyright © 2015-2017 Fletcher T. Penney.


	The `c-template` project is released under the MIT License.

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


#ifndef AC_TEMPLATE_H
#define AC_TEMPLATE_H

#include <string.h>

struct trie_node {
	char				c;					// Character for this node
	unsigned short		match_type;			// 0 = no match, otherwise what have we matched?
	unsigned short		len;				// Length of string matched
	size_t				child[256];			// Where should we go next?
	size_t				ac_fail;			// Where should we go if we fail?
};

typedef struct trie_node trie_node;


struct trie {
	size_t				size;				// How many nodes are in use?
	size_t				capacity;			// How many nodes can we hold

	trie_node 	*		node;				// Pointer to stack of nodes
};

typedef struct trie trie;


struct match {
	size_t				start;				// Starting offset for this match
	size_t				len;				// Length for this match
	unsigned short		match_type;			// Match type
	struct match 	*	next;				// Pointer to next match in list
	struct match 	*	prev;				// Pointer to previous match in list
};

typedef struct match match;


trie * trie_new(size_t startingSize);

bool trie_insert(trie * a, const char * key, unsigned short match_type);

void ac_trie_prepare(trie * a);

match * ac_trie_search(trie * a, const char * source, size_t start, size_t len);

match * ac_trie_leftmost_longest_search(trie * a, const char * source, size_t start, size_t len);

void trie_free(trie * a);

void match_set_describe(match * m, const char * source);

void match_set_filter_leftmost_longest(match * header);

void match_free(match * m);


void trie_to_graphviz(trie * a);


#ifdef TEST
	#include "CuTest.h"
#endif


#endif
