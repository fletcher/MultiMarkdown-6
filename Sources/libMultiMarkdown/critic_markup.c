/**

	MultiMarkdown -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file critic_markup.c

	@brief


	@author	Fletcher T. Penney
	@bug

**/

/*

	Copyright © 2016 - 2017 Fletcher T. Penney.


	The `MultiMarkdown 6` project is released under the MIT License..

	GLibFacade.c and GLibFacade.h are from the MultiMarkdown v4 project:

		https://github.com/fletcher/MultiMarkdown-4/

	MMD 4 is released under both the MIT License and GPL.


	CuTest is released under the zlib/libpng license. See CuTest.c for the
	text of the license.


	## The MIT License ##

	Permission is hereby granted, free of charge, to any person obtaining
	a copy of this software and associated documentation files (the
	"Software"), to deal in the Software without restriction, including
	without limitation the rights to use, copy, modify, merge, publish,
	distribute, sublicense, and/or sell copies of the Software, and to
	permit persons to whom the Software is furnished to do so, subject to
	the following conditions:

	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
	CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
	TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


*/

#include <stdbool.h>
#include <string.h>


#include "aho-corasick.h"
#include "critic_markup.h"
#include "stack.h"
#include "token_pairs.h"


token * mmd_critic_tokenize_string(const char * source, size_t start, size_t len) {
	trie * ac = trie_new(0);

	trie_insert(ac, "{++", CM_ADD_OPEN);
	trie_insert(ac, "++}", CM_ADD_CLOSE);

	trie_insert(ac, "{--", CM_DEL_OPEN);
	trie_insert(ac, "--}", CM_DEL_CLOSE);

	trie_insert(ac, "{~~", CM_SUB_OPEN);
	trie_insert(ac, "~>", CM_SUB_DIV);
	trie_insert(ac, "~~}", CM_SUB_CLOSE);

	trie_insert(ac, "{==", CM_HI_OPEN);
	trie_insert(ac, "==}", CM_HI_CLOSE);

	trie_insert(ac, "{>>", CM_COM_OPEN);
	trie_insert(ac, "<<}", CM_COM_CLOSE);

	trie_insert(ac, "\\{", CM_PLAIN_TEXT);
	trie_insert(ac, "\\}", CM_PLAIN_TEXT);
	trie_insert(ac, "\\{", CM_PLAIN_TEXT);
	trie_insert(ac, "\\+", CM_PLAIN_TEXT);
	trie_insert(ac, "\\-", CM_PLAIN_TEXT);
	trie_insert(ac, "\\~", CM_PLAIN_TEXT);
	trie_insert(ac, "\\>", CM_PLAIN_TEXT);
	trie_insert(ac, "\\=", CM_PLAIN_TEXT);


	ac_trie_prepare(ac);

	match * m = ac_trie_leftmost_longest_search(ac, source, start, len);

	token * root = NULL;

	if (m) {
		match * walker = m->next;

		root = token_new(0, 0, 0);

		size_t last = start;

		while (walker) {
			if (walker->start > last) {
				token_append_child(root, token_new(CM_PLAIN_TEXT, last, walker->start - last));
				last = walker->start;
			}

			if (walker->start == last) {
				token_append_child(root, token_new(walker->match_type, walker->start, walker->len));
				last = walker->start + walker->len;
			}

			walker = walker->next;
		}

		if (last < start + len) {
			token_append_child(root, token_new(CM_PLAIN_TEXT, last, start + len));
		}

		match_free(m);
		trie_free(ac);
	}

	return root;
}



token * critic_parse_substring(const char * source, size_t start, size_t len) {
	token * chain = mmd_critic_tokenize_string(source, start, len);

	if (chain) {
		token_pair_engine * e = token_pair_engine_new();

		token_pair_engine_add_pairing(e, CM_ADD_OPEN, CM_ADD_CLOSE, CM_ADD_PAIR, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
		token_pair_engine_add_pairing(e, CM_DEL_OPEN, CM_DEL_CLOSE, CM_DEL_PAIR, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
		token_pair_engine_add_pairing(e, CM_SUB_OPEN, CM_SUB_CLOSE, CM_SUB_PAIR, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
		token_pair_engine_add_pairing(e, CM_HI_OPEN,  CM_HI_CLOSE,  CM_HI_PAIR,  PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
		token_pair_engine_add_pairing(e, CM_COM_OPEN, CM_COM_CLOSE, CM_COM_PAIR, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);

		stack * s = stack_new(0);

		token_pairs_match_pairs_inside_token(chain, e, s, 0);

		stack_free(s);
		token_pair_engine_free(e);
	}

	return chain;
}


void accept_token_tree(DString * d, token * t);
void accept_token(DString * d, token * t);


void accept_token_tree_sub(DString * d, token * t) {
	while (t) {
		if (t->type == CM_SUB_DIV) {
			while (t) {
				d_string_erase(d, t->start, t->len);
				t = t->prev;
			}

			return;
		}

		accept_token(d, t);

		t = t->prev;
	}
}


void accept_token(DString * d, token * t) {
	switch (t->type) {
		case CM_SUB_CLOSE:
			if (t->mate) {
				d_string_erase(d, t->start, t->len);
			}

			break;

		case CM_SUB_OPEN:
		case CM_ADD_OPEN:
		case CM_ADD_CLOSE:
		case CM_HI_OPEN:
		case CM_HI_CLOSE:
			if (!t->mate) {
				break;
			}

		case CM_SUB_DIV:
		case CM_DEL_PAIR:
		case CM_COM_PAIR:
			// Erase these
			d_string_erase(d, t->start, t->len);
			break;

		case CM_SUB_PAIR:

			// Erase old version and markers
			if (t->child) {
				accept_token_tree_sub(d, t->child->mate);
			}

			break;

		case CM_ADD_PAIR:
		case CM_HI_PAIR:

			// Check children
			if (t->child) {
				accept_token_tree(d, t->child->mate);
			}

			break;
	}
}


void accept_token_tree(DString * d, token * t) {
	while (t) {
		accept_token(d, t);

		// Iterate backwards so offsets are right
		t = t->prev;
	}
}


void mmd_critic_markup_accept_range(DString * d, size_t start, size_t len) {
	token * t = critic_parse_substring(d->str, start, len);

	if (t && t->child) {
		accept_token_tree(d, t->child->tail);
	}

	token_free(t);
}


void mmd_critic_markup_accept(DString * d) {
	mmd_critic_markup_accept_range(d, 0, d->currentStringLength);
}


void reject_token_tree(DString * d, token * t);
void reject_token(DString * d, token * t);


void reject_token_tree_sub(DString * d, token * t) {
	while (t && t->type != CM_SUB_DIV) {
		d_string_erase(d, t->start, t->len);
		t = t->prev;
	}

	while (t) {

		reject_token(d, t);

		t = t->prev;
	}
}


void reject_token(DString * d, token * t) {
	switch (t->type) {
		case CM_SUB_CLOSE:
			if (t->mate) {
				d_string_erase(d, t->start, t->len);
			}

			break;

		case CM_SUB_OPEN:
		case CM_DEL_OPEN:
		case CM_DEL_CLOSE:
		case CM_HI_OPEN:
		case CM_HI_CLOSE:
			if (!t->mate) {
				break;
			}

		case CM_SUB_DIV:
		case CM_ADD_PAIR:
		case CM_COM_PAIR:
			// Erase these
			d_string_erase(d, t->start, t->len);
			break;

		case CM_SUB_PAIR:

			// Erase new version and markers
			if (t->child) {
				reject_token_tree_sub(d, t->child->mate);
			}

			break;

		case CM_DEL_PAIR:
		case CM_HI_PAIR:

			// Check children
			if (t->child) {
				reject_token_tree(d, t->child->mate);
			}

			break;
	}
}


void reject_token_tree(DString * d, token * t) {
	while (t) {
		reject_token(d, t);

		// Iterate backwards so offsets are right
		t = t->prev;
	}
}


void mmd_critic_markup_reject_range(DString * d, size_t start, size_t len) {
	token * t = critic_parse_substring(d->str, start, len);

	if (t && t->child) {
		reject_token_tree(d, t->child->tail);
	}

	token_free(t);

}


void mmd_critic_markup_reject(DString * d) {
	mmd_critic_markup_reject_range(d, 0, d->currentStringLength);
}


#ifdef TEST
void Test_critic(CuTest* tc) {
	#ifdef kUseObjectPool
	token_pool_init();
	#endif

	DString * test = d_string_new("{--foo bar--}");
	mmd_critic_markup_reject(test);
	CuAssertStrEquals(tc, "foo bar", test->str);

	d_string_erase(test, 0, -1);
	d_string_append(test, "{++foo bar++}");
	mmd_critic_markup_reject(test);
	CuAssertStrEquals(tc, "", test->str);


	d_string_erase(test, 0, -1);
	d_string_append(test, "{--foo bar--}");
	mmd_critic_markup_accept(test);
	CuAssertStrEquals(tc, "", test->str);

	d_string_erase(test, 0, -1);
	d_string_append(test, "{++foo bar++}");
	mmd_critic_markup_accept(test);
	CuAssertStrEquals(tc, "foo bar", test->str);

	d_string_erase(test, 0, -1);
	d_string_append(test, "{++foo{--bat--}bar++}");
	mmd_critic_markup_accept(test);
	CuAssertStrEquals(tc, "foobar", test->str);

	d_string_erase(test, 0, -1);
	d_string_append(test, "{--foo{-- bat --}bar--}");
	mmd_critic_markup_reject(test);
	CuAssertStrEquals(tc, "foo bat bar", test->str);

	d_string_erase(test, 0, -1);
	d_string_append(test, "{--foo{++ bat ++}bar--}");
	mmd_critic_markup_reject(test);
	CuAssertStrEquals(tc, "foobar", test->str);

	d_string_erase(test, 0, -1);
	d_string_append(test, "{==foo bar==}");
	mmd_critic_markup_reject(test);
	CuAssertStrEquals(tc, "foo bar", test->str);

	d_string_erase(test, 0, -1);
	d_string_append(test, "{==foo bar==}");
	mmd_critic_markup_accept(test);
	CuAssertStrEquals(tc, "foo bar", test->str);

	d_string_erase(test, 0, -1);
	d_string_append(test, "{>>foo bar<<}");
	mmd_critic_markup_reject(test);
	CuAssertStrEquals(tc, "", test->str);

	d_string_erase(test, 0, -1);
	d_string_append(test, "{>>foo bar<<}");
	mmd_critic_markup_accept(test);
	CuAssertStrEquals(tc, "", test->str);

	d_string_erase(test, 0, -1);
	d_string_append(test, "{++foo++}{>>bar<<}");
	mmd_critic_markup_accept(test);
	CuAssertStrEquals(tc, "foo", test->str);

	d_string_erase(test, 0, -1);
	d_string_append(test, "{++foo++}{>>bar<<}");
	mmd_critic_markup_reject(test);
	CuAssertStrEquals(tc, "", test->str);

	#ifdef kUseObjectPool
	// Decrement counter and clean up token pool
	token_pool_drain();

	token_pool_free();
	#endif
}
#endif
