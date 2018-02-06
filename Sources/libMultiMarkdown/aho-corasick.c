/**

	C-Template -- Boilerplate c project with cmake support, CuTest unit testing, and more.

	@file aho-corasick.c

	@brief C implementation of the Aho-Corasick algorithm for searching text
	for multiple strings simultaneously in a single pass without backtracking.

	<https://en.wikipedia.org/wiki/Aho%E2%80%93Corasick_algorithm>


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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "aho-corasick.h"

#define kTrieStartingSize 256

void trie_to_graphviz(trie * a);


trie * trie_new(size_t startingSize) {
	trie * a = malloc(sizeof(trie));

	if (a) {
		if (startingSize <= 1) {
			startingSize = kTrieStartingSize;
		}

		a->node = malloc(sizeof(trie_node) * startingSize);

		if (!a->node) {
			free(a);
			return NULL;
		}

		// Clear memory
		memset(a->node, 0, sizeof(trie_node) * startingSize);

		// All tries have a root node
		a->size = 1;
		a->capacity = startingSize;
	}

	return a;
}


void trie_free(trie * a) {
	free(a->node);
	free(a);
}


bool trie_node_insert(trie * a, size_t s, const unsigned char * key, unsigned short match_type, unsigned short depth) {
	// Get node for state s
	trie_node * n = &a->node[s];

	size_t i;

	if (key[0] == '\0') {
		// We've hit end of key
		n->match_type = match_type;
		n->len = depth;
		return true;		// Success
	}

	if (n->child[key[0]] != 0) {
		// First character already in trie, advance forward
		return trie_node_insert(a, n->child[key[0]], key + 1, match_type, ++depth);
	} else {
		// Create new node

		// Ensure capacity
		if (a->size == a->capacity) {
			a->capacity *= 2;
			a->node = realloc(a->node, a->capacity * sizeof(trie_node));

			// Set n to new location
			n = &(a->node[s]);
		}

		// Current node points to next node
		i = a->size;
		n->child[key[0]] = i;

		// Initialize new node to 0
		n = &a->node[i];
		memset(n, 0, sizeof(trie_node));

		// Set char for new node
		n->c = key[0];

		// Incremement size
		a->size++;

		// Advance forward
		return trie_node_insert(a, i, key + 1, match_type, ++depth);
	}
}


bool trie_insert(trie * a, const char * key, unsigned short match_type) {
	if (a && key && (key[0] != '\0')) {
		return trie_node_insert(a, 0, (const unsigned char *)key, match_type, 0);
	}

	return false;
}


#ifdef TEST
void Test_trie_insert(CuTest* tc) {
	trie * a = trie_new(0);

	CuAssertIntEquals(tc, kTrieStartingSize, a->capacity);
	CuAssertIntEquals(tc, 1, a->size);

	trie_insert(a, "foo", 42);

	trie_node * n = &a->node[0];
	CuAssertIntEquals(tc, 0, n->match_type);
	CuAssertIntEquals(tc, 1, n->child['f']);
	CuAssertIntEquals(tc, '\0', n->c);

	n = &a->node[1];
	CuAssertIntEquals(tc, 0, n->match_type);
	CuAssertIntEquals(tc, 2, n->child['o']);
	CuAssertIntEquals(tc, 'f', n->c);

	n = &a->node[2];
	CuAssertIntEquals(tc, 0, n->match_type);
	CuAssertIntEquals(tc, 3, n->child['o']);
	CuAssertIntEquals(tc, 'o', n->c);

	n = &a->node[3];
	CuAssertIntEquals(tc, 42, n->match_type);
	CuAssertIntEquals(tc, 3, n->len);
	CuAssertIntEquals(tc, 'o', n->c);

	trie_free(a);
}
#endif


size_t trie_node_search(trie * a, size_t s, const char * query) {
	if (query[0] == '\0') {
		// Found matching state
		return s;
	}

	if (a->node[s].child[(unsigned char)query[0]] == 0) {
		// Failed to match
		return -1;
	}

	// Partial match, keep going
	return trie_node_search(a, a->node[s].child[(unsigned char)query[0]], query + 1);
}


size_t trie_search(trie * a, const char * query) {
	if (a && query) {
		return trie_node_search(a, 0, query);
	}

	return 0;
}


unsigned short trie_search_match_type(trie * a, const char * query) {
	size_t s = trie_search(a, query);

	if (s == -1) {
		return -1;
	}

	return a->node[s].match_type;
}


#ifdef TEST
void Test_trie_search(CuTest* tc) {
	trie * a = trie_new(0);

	trie_insert(a, "foo", 42);
	trie_insert(a, "bar", 41);
	trie_insert(a, "food", 40);

	CuAssertIntEquals(tc, 3, trie_search(a, "foo"));
	CuAssertIntEquals(tc, 42, trie_search_match_type(a, "foo"));

	CuAssertIntEquals(tc, 6, trie_search(a, "bar"));
	CuAssertIntEquals(tc, 41, trie_search_match_type(a, "bar"));

	CuAssertIntEquals(tc, 7, trie_search(a, "food"));
	CuAssertIntEquals(tc, 40, trie_search_match_type(a, "food"));

	CuAssertIntEquals(tc, -1, trie_search(a, "foot"));
	CuAssertIntEquals(tc, (unsigned short) - 1, trie_search_match_type(a, "foot"));

	trie_free(a);
}
#endif


void ac_trie_node_prepare(trie * a, size_t s, char * buffer, unsigned short depth, size_t last_match_state) {

	buffer[depth] = '\0';
	buffer[depth + 1] = '\0';

	// Current node
	trie_node * n = &(a->node[s]);

	char * suffix = buffer;

	// Longest match seen so far??
	suffix += 1;

	// Find valid suffixes for failure path
	while ((suffix[0] != '\0') && (n->ac_fail == 0)) {
		n->ac_fail = trie_search(a, suffix);

		if (n->ac_fail == -1) {
			n->ac_fail = 0;
		}

		if (n->ac_fail == s) {
			// Something went wrong
			fprintf(stderr, "Recursive trie fallback detected at state %lu('%c') - suffix:'%s'!\n", s, n->c, suffix);
			n->ac_fail = 0;
		}

		suffix++;
	}


	// Prepare children
	for (int i = 0; i < 256; ++i) {
		if ((n->child[i] != 0) &&
				(n->child[i] != s)) {
			buffer[depth] = i;

			ac_trie_node_prepare(a, n->child[i], buffer, depth + 1, last_match_state);
		}
	}
}

/// Prepare trie for Aho-Corasick search algorithm by mapping failure connections
void ac_trie_prepare(trie * a) {
	// Clear old pointers
	for (size_t i = 0; i < a->size; ++i) {
		a->node[i].ac_fail = 0;
	}

	// Create a buffer to use
	char buffer[a->capacity];

	ac_trie_node_prepare(a, 0, buffer, 0, 0);
}



#ifdef TEST
void Test_trie_prepare(CuTest* tc) {
	trie * a = trie_new(0);

	trie_insert(a, "a", 1);
	trie_insert(a, "aa", 2);
	trie_insert(a, "aaa", 3);
	trie_insert(a, "aaaa", 4);

	ac_trie_prepare(a);

	trie_free(a);
}
#endif


match * match_new(size_t start, size_t len, unsigned short match_type) {
	match * m = malloc(sizeof(match));

	if (m) {
		m->start = start;
		m->len = len;
		m->match_type = match_type;
		m->next = NULL;
		m->prev = NULL;
	}

	return m;
}


void match_free(match * m) {
	if (m) {
		if (m->next) {
			match_free(m->next);
		}

		free(m);
	}
}


match * match_add(match * last, size_t start, size_t len, unsigned short match_type) {
	if (last) {
		last->next = match_new(start, len, match_type);
		last->next->prev = last;
		return last->next;
	} else {
		return match_new(start, len, match_type);
	}

	return NULL;
}


match * ac_trie_search(trie * a, const char * source, size_t start, size_t len) {

	// Store results in a linked list
//	match * result = match_new(0, 0, 0);
	match * result = NULL;
	match * m = result;

	// Keep track of our state
	size_t state = 0;
	size_t temp_state;

	// Character being compared
	unsigned char test_value;
	size_t counter = start;
	size_t stop = start + len;

	while ((counter < stop) && (source[counter] != '\0')) {
		// Read next character
		test_value = (unsigned char)source[counter++];

		// Check for path that allows us to match next character
		while (state != 0 && a->node[state].child[test_value] == 0) {
			state = a->node[state].ac_fail;
		}

		// Advance state for the next character
		state = a->node[state].child[test_value];

		// Check for partial matches
		temp_state = state;

		while (temp_state != 0) {
			if (a->node[temp_state].match_type) {
				// This is a match
				if (!m) {
					result = match_new(0, 0, 0);
					m = result;
				}

				m = match_add(m, counter - a->node[temp_state].len,
							  a->node[temp_state].len, a->node[temp_state].match_type);
			}

			// Iterate to find shorter matches
			temp_state = a->node[temp_state].ac_fail;
		}
	}

	return result;
}


void match_excise(match * m) {
	if (m) {
		if (m->prev) {
			m->prev->next = m->next;
		}

		if (m->next) {
			m->next->prev = m->prev;
		}

		free(m);
	}
}


int match_count(match * m) {
	int result = 0;
	m = m->next;	// Skip header

	while (m) {
		result++;
		m = m->next;
	}

	return result;
}


void match_describe(match * m, const char * source) {
	fprintf(stderr, "'%.*s'(%d) at %lu:%lu\n", (int)m->len, &source[m->start],
			m->match_type, m->start, m->start + m->len);
}


void match_set_describe(match * m, const char * source) {
	m = m->next;	// Skip header

	while (m) {
		match_describe(m, source);
		m = m->next;
	}
}


void match_set_filter_leftmost_longest(match * header) {
	// Filter results to include only leftmost/longest results
	match * m = header->next; 	// Skip header
	match * n;

	while (m) {
		if (m->next) {
			if (m->start == m->next->start) {
				// The next match is longer than this one
				n = m;
				m = m->next;
				match_excise(n);
				continue;
			}

			while (m->next &&
					m->next->start > m->start &&
					m->next->start < m->start + m->len) {
				// This match is "lefter" than next
				#ifndef __clang_analyzer__
				match_excise(m->next);
				#endif
			}

			while (m->next &&
					m->next->start < m->start) {
				// Next match is "lefter" than us
				n = m;
				m = m->prev;
				match_excise(n);
			}
		}

		while (m->prev &&
				m->prev->len &&
				m->prev->start >= m->start) {
			// We are "lefter" than previous
			n = m->prev;
			#ifndef __clang_analyzer__
			match_excise(n);
			#endif
		}

		m = m->next;
	}
}


match * ac_trie_leftmost_longest_search(trie * a, const char * source, size_t start, size_t len) {
	match * result = ac_trie_search(a, source, start, len);

	if (result) {
		match_set_filter_leftmost_longest(result);
	}

	return result;
}


#ifdef TEST
void Test_aho_trie_search(CuTest* tc) {
	trie * a = trie_new(0);

	trie_insert(a, "foo", 42);
	trie_insert(a, "bar", 41);
	trie_insert(a, "food", 40);

	ac_trie_prepare(a);

	match * m = ac_trie_search(a, "this is a bar that serves food.", 0, 31);

	match_free(m);
	trie_free(a);


	a = trie_new(0);

	trie_insert(a, "A", 1);
	trie_insert(a, "AB", 2);
	trie_insert(a, "ABC", 3);
	trie_insert(a, "BC", 4);
	trie_insert(a, "BCD", 5);
	trie_insert(a, "E", 6);
	trie_insert(a, "EFGHIJ", 7);
	trie_insert(a, "F", 8);
	trie_insert(a, "ZABCABCZ", 9);
	trie_insert(a, "ZAB", 10);

	ac_trie_prepare(a);

	m = ac_trie_search(a, "ABCDEFGGGAZABCABCDZABCABCZ", 0, 26);
	fprintf(stderr, "Finish with %d matches\n", match_count(m));
	match_set_describe(m, "ABCDEFGGGAZABCABCDZABCABCZ");
	match_free(m);

	m = ac_trie_leftmost_longest_search(a, "ABCDEFGGGAZABCABCDZABCABCZ", 0, 26);
	fprintf(stderr, "Finish with %d matches\n", match_count(m));
	match_set_describe(m, "ABCDEFGGGAZABCABCDZABCABCZ");
	match_free(m);

	// trie_to_graphviz(a);

	trie_free(a);
}
#endif


void trie_node_to_graphviz(trie * a, size_t s) {
	trie_node * n = &a->node[s];

	if (n->match_type) {
		fprintf(stderr, "\"%lu\" [shape=doublecircle]\n", s);
	}

	for (int i = 0; i < 256; ++i) {
		if (n->child[i]) {
			switch (i) {
				default:
					fprintf(stderr, "\"%lu\" -> \"%lu\" [label=\"%c\"]\n", s, n->child[i], (char)i);
			}
		}
	}

	if (n->ac_fail) {
		fprintf(stderr, "\"%lu\" -> \"%lu\" [label=\"fail\"]\n", s, n->ac_fail);
	}
}


void trie_to_graphviz(trie * a) {
	fprintf(stderr, "digraph dfa {\n");

	for (int i = 0; i <  a->size; ++i) {
		trie_node_to_graphviz(a, i);
	}

	fprintf(stderr, "}\n");
}

