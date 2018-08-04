/**

	MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file writer.c

	@brief Coordinate conversion of token tree to output formats.


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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libMultiMarkdown.h"

#include "aho-corasick.h"
#include "beamer.h"
#include "char.h"
#include "d_string.h"
#include "html.h"
#include "i18n.h"
#include "latex.h"
#include "memoir.h"
#include "mmd.h"
#include "opendocument-content.h"
#include "opml.h"
#include "parser.h"
#include "scanners.h"
#include "token.h"
#include "uuid.h"
#include "writer.h"


void store_citation(scratch_pad * scratch, footnote * f);

void store_footnote(scratch_pad * scratch, footnote * f);

void store_glossary(scratch_pad * scratch, footnote * f);

void store_link(scratch_pad * scratch, link * l);

void store_metadata(scratch_pad * scratch, meta * m);

void store_abbreviation(scratch_pad * scratch, footnote * a);


/// strndup not available on all platforms
static char * my_strndup(const char * source, size_t n) {
	if (source == NULL) {
		return NULL;
	}

	size_t len = 0;
	char * result;
	const char * test = source;

	// strlen is too slow is strlen(source) >> n
	for (len = 0; len < n; ++len) {
		if (test == '\0') {
			break;
		}

		test++;
	}

	result = malloc(len + 1);

	if (result) {
		memcpy(result, source, len);
		result[len] = '\0';
	}

	return result;
}


/// strdup() not available on all platforms
static char * my_strdup(const char * source) {
	if (source == NULL) {
		return NULL;
	}

	char * result = malloc(strlen(source) + 1);

	if (result) {
		strcpy(result, source);
	}

	return result;
}


/// Temporary storage while exporting parse tree to output format
scratch_pad * scratch_pad_new(mmd_engine * e, short format) {
	scratch_pad * p = malloc(sizeof(scratch_pad));

	if (p) {
		p->padded = 2;							// Prevent unnecessary leading space
		p->list_is_tight = false;				// Tight vs Loose list
		p->skip_token = 0;						// Skip over next n tokens
		p->close_para = true;

		p->extensions = e->extensions;
		p->output_format = format;
		p->quotes_lang = e->quotes_lang;
		p->language = e->language;

		p->header_stack = e->header_stack;

		p->outline_stack = stack_new(0);
		p->opml_item_closed = 1;

		p->recurse_depth = 0;

		p->base_header_level = 1;

		p->odf_para_type = BLOCK_PARA;

		if (e->extensions & EXT_RANDOM_FOOT) {
			p->random_seed_base = rand() % 32000;
		} else {
			p->random_seed_base = 0;
		}

		// Store links in a hash for rapid retrieval when exporting
		p->link_hash = NULL;
		link * l;

		for (int i = 0; i < e->link_stack->size; ++i) {
			l = stack_peek_index(e->link_stack, i);

			store_link(p, l);
		}

		// Store citations in a hash for rapid retrieval when exporting
		footnote * f;

		p->used_citations = stack_new(0);
		p->inline_citations_to_free = stack_new(0);
		p->citation_being_printed = 0;
		p->bibtex_file = NULL;

		p->citation_hash = NULL;

		for (int i = 0; i < e->citation_stack->size; ++i) {
			f = stack_peek_index(e->citation_stack, i);

			store_citation(p, f);
		}

		// Store footnotes in a hash for rapid retrieval when exporting
		p->used_footnotes = stack_new(0);				// Store footnotes as we use them
		p->inline_footnotes_to_free = stack_new(0);		// Inline footnotes need to be freed
		p->footnote_being_printed = 0;
		p->footnote_para_counter = -1;

		p->footnote_hash = NULL;				// Store defined footnotes in a hash

		for (int i = 0; i < e->footnote_stack->size; ++i) {
			f = stack_peek_index(e->footnote_stack, i);

			store_footnote(p, f);
		}

		// Store glossaries in a hash for rapid retrieval when exporting
		p->used_glossaries = stack_new(0);
		p->inline_glossaries_to_free = stack_new(0);
		p->glossary_being_printed = 0;

		p->glossary_hash = NULL;

		for (int i = 0; i < e->glossary_stack->size; ++i) {
			f = stack_peek_index(e->glossary_stack, i);

			store_glossary(p, f);
		}

		// Store abbreviations in a hash for rapid retrieval when exporting
		p->used_abbreviations = stack_new(0);
		p->inline_abbreviations_to_free = stack_new(0);

		p->abbreviation_hash = NULL;

		for (int i = 0; i < e->abbreviation_stack->size; ++i) {
			f = stack_peek_index(e->abbreviation_stack, i);

			store_abbreviation(p, f);
		}

		// Store metadata in a hash for rapid retrieval when exporting
		p->meta_hash = NULL;
		meta * m;

		for (int i = 0; i < e->metadata_stack->size; ++i) {
			m = stack_peek_index(e->metadata_stack, i);

			store_metadata(p, m);
		}


		// Store used assets in a hash
		p->asset_hash = NULL;
		p->store_assets = 0;
		p->remember_assets = 0;
	}

	return p;
}


void scratch_pad_free(scratch_pad * scratch) {
//	HASH_CLEAR(hh, scratch->link_hash);

	stack_free(scratch->outline_stack);

	link * l, * l_tmp;

	// Free link hash
	HASH_ITER(hh, scratch->link_hash, l, l_tmp) {
		HASH_DEL(scratch->link_hash, l); 	// Remove item from hash
		free(l);		// "Shallow" free -- the pointers will be freed
		// with the original later.
	}

	fn_holder * f, * f_tmp;


	// Free footnote hash
	HASH_ITER(hh, scratch->footnote_hash, f, f_tmp) {
		HASH_DEL(scratch->footnote_hash, f);	// Remove item from hash
		free(f);		// Free the fn_holder
	}
	stack_free(scratch->used_footnotes);

	while (scratch->inline_footnotes_to_free->size) {
		footnote_free(stack_pop(scratch->inline_footnotes_to_free));
	}

	stack_free(scratch->inline_footnotes_to_free);


	// Free citation hash
	HASH_ITER(hh, scratch->citation_hash, f, f_tmp) {
		HASH_DEL(scratch->citation_hash, f);	// Remove item from hash
		free(f);		// Free the fn_holder
	}
	stack_free(scratch->used_citations);

	while (scratch->inline_citations_to_free->size) {
		footnote_free(stack_pop(scratch->inline_citations_to_free));
	}

	stack_free(scratch->inline_citations_to_free);

	free(scratch->bibtex_file);

	// Free glossary hash
	HASH_ITER(hh, scratch->glossary_hash, f, f_tmp) {
		HASH_DEL(scratch->glossary_hash, f);	// Remove item from hash
		free(f);		// Free the fn_holder
	}
	stack_free(scratch->used_glossaries);

	while (scratch->inline_glossaries_to_free->size) {
		footnote_free(stack_pop(scratch->inline_glossaries_to_free));
	}

	stack_free(scratch->inline_glossaries_to_free);


	// Free abbreviation hash
	HASH_ITER(hh, scratch->abbreviation_hash, f, f_tmp) {
		HASH_DEL(scratch->abbreviation_hash, f);	// Remove item from hash
		free(f);		// Free the fn_holder
	}
	stack_free(scratch->used_abbreviations);

	while (scratch->inline_abbreviations_to_free->size) {
		footnote_free(stack_pop(scratch->inline_abbreviations_to_free));
	}

	stack_free(scratch->inline_abbreviations_to_free);


	// Free metadata hash
	meta * m, * m_tmp;

	HASH_ITER(hh, scratch->meta_hash, m, m_tmp) {
		HASH_DEL(scratch->meta_hash, m); 	// Remove item from hash
		// Don't free meta pointer since it is freed with the mmd_engine
		//meta_free(m);
	}

	free(scratch);
}


/// Ensure at least num newlines at end of output buffer
void pad(DString * d, short num, scratch_pad * scratch) {
	while (num > scratch->padded) {
		d_string_append_c(d, '\n');
		scratch->padded++;
	}
}


void print_token_raw(DString * out, const char * source, token * t) {
	if (t) {
		switch (t->type) {
			case EMPH_START:
			case EMPH_STOP:
			case STRONG_START:
			case STRONG_STOP:
			case TEXT_EMPTY:
				break;

			case PAIR_EMPH:
			case PAIR_STRONG:
				print_token_tree_raw(out, source, t->child);
				break;

			default:
				d_string_append_c_array(out, &source[t->start], t->len);
				break;
		}
	}
}


void print_token_tree_raw(DString * out, const char * source, token * t) {
	while (t) {
		print_token_raw(out, source, t);

		t = t->next;
	}
}


char * text_inside_pair(const char * source, token * pair) {
	char * result = NULL;

	if (source && pair) {
		if (pair->child && pair->child->mate) {
			// [foo], [^foo], [#foo] should give different strings -- use closer len
			result = my_strndup(&source[pair->start + pair->child->mate->len], pair->len - (pair->child->mate->len * 2));
		} else {
			if (pair->child) {
				result = my_strndup(&source[pair->start + pair->child->len], pair->len - (pair->child->len + 1));
			}
		}
	}

	return result;
}


char * label_from_string(const char * str) {
	const char * next_char;
	char * label = NULL;

	DString * out = d_string_new("");

	while (*str != '\0') {
		next_char = str;
		next_char++;

		if ((*next_char & 0xC0) == 0x80) {
			// Allow multibyte characters
			d_string_append_c(out, *str);

			while ((*next_char & 0xC0) == 0x80) {
				str++;
				d_string_append_c(out, *str);
				next_char++;
			}
		} else if ((*str >= '0' && *str <= '9') || (*str >= 'A' && *str <= 'Z')
				   || (*str >= 'a' && *str <= 'z') || (*str == '.') || (*str == '_')
				   || (*str == '-') || (*str == ':')) {
			// Allow 0-9, A-Z, a-z, ., _, -, :
			d_string_append_c(out, tolower(*str));
		}

		str++;
	}

	label = out->str;
	d_string_free(out, false);

	return label;
}


char * label_from_token(const char * source, token * t) {
	char * label = NULL;

	DString * raw = d_string_new("");

	d_string_append_c_array(raw, &source[t->start], t->len);

	label = label_from_string(raw->str);

	d_string_free(raw, true);

	return label;
}


char * label_from_header(const char * source, token * t) {
	char * result;
	token * temp_token = manual_label_from_header(t, source);

	if (temp_token) {
		result = label_from_token(source, temp_token);
	} else {
		result = label_from_token(source, t);
	}

	return result;
}


/// Clean up whitespace in string for standardization
char * clean_string(const char * str, bool lowercase) {
	if (str == NULL) {
		return NULL;
	}

	DString * out = d_string_new("");
	char * clean = NULL;
	bool block_whitespace = true;

	while (*str != '\0') {
		switch (*str) {
			case '\t':
			case ' ':
			case '\n':
			case '\r':
				if (!block_whitespace) {
					d_string_append_c(out, ' ');
					block_whitespace = true;
				}

				break;

			default:
				if (lowercase) {
					d_string_append_c(out, tolower(*str));
				} else {
					d_string_append_c(out, *str);
				}

				block_whitespace = false;
				break;
		}

		str++;
	}

	clean = out->str;

	// Trim trailing whitespace/newlines
	while (out->currentStringLength && char_is_whitespace_or_line_ending(clean[out->currentStringLength - 1])) {
		out->currentStringLength--;
		clean[out->currentStringLength] = '\0';
	}

	d_string_free(out, false);

	// Trim trailing whitespace
	return clean;
}


char * clean_string_from_range(const char * source, size_t start, size_t len, bool lowercase) {
	char * clean = NULL;

	DString * raw = d_string_new("");

	d_string_append_c_array(raw, &source[start], len);

	clean = clean_string(raw->str, lowercase);

	d_string_free(raw, true);

	return clean;
}


char * clean_string_from_token(const char * source, token * t, bool lowercase) {
	return clean_string_from_range(source, t->start, t->len, lowercase);
}


char * clean_inside_pair(const char * source, token * t, bool lowercase) {
	char * text = text_inside_pair(source, t);

	char * clean = clean_string(text, lowercase);

	free(text);

	return clean;
}


attr * attr_new(char * key, char * value) {
	attr * a = malloc(sizeof(attr));
	size_t len = strlen(value);

	// Strip quotes if present
	if (value[0] == '"') {
		value++;
		len--;
	}

	if (value[len - 1] == '"') {
		value[len - 1] = '\0';
	}

	if (a) {
		a->key = key;
		a->value = my_strdup(value);
		a->next = NULL;
	}

	return a;
}


attr * parse_attributes(char * source) {
	attr * attributes = NULL;
	attr * a = NULL;
	char * key = NULL;
	char * value = NULL;
	size_t scan_len;
	size_t pos = 0;

	while (source[pos] != '\0' && scan_attr(&source[pos])) {
		pos +=  scan_spnl(&source[pos]);

		// Get key
		scan_len = scan_key(&source[pos]);
		key = my_strndup(&source[pos], scan_len);

		// Skip '='
		pos += scan_len + 1;

		// Get value
		scan_len = scan_value(&source[pos]);
		value = my_strndup(&source[pos], scan_len);

		pos += scan_len;

		if (a) {
			a->next = attr_new(key, value);
			a = a->next;
		} else {
			#ifndef __clang_analyzer__
			a = attr_new(key, value);
			attributes = a;
			#endif
		}

		free(value);	// We stored a modified copy
	}

	return attributes;
}


link * link_new(const char * source, token * label, char * url, char * title, char * attributes, short flags) {
	link * l = malloc(sizeof(link));

	if (l) {
		l->label = label;

		if (label) {
			l->clean_text = clean_inside_pair(source, label, true);
			l->label_text = label_from_token(source, label);
		} else {
			l->clean_text = NULL;
			l->label_text = NULL;
		}

		l->url = clean_string(url, false);
		l->title = (title == NULL) ? NULL : my_strdup(title);
		l->attributes = (attributes == NULL) ? NULL : parse_attributes(attributes);

		l->flags = flags;
	}

	return l;
}


/// Store shallow copies of links in the storage hash.  The link
/// itself is new, but references the same data as the original.
/// This allows the copied link to simply be `free()`'d without
/// freeing the pointers.
link * link_shallow_copy(link * l) {
	link * new = malloc(sizeof(link));

	if (new) {
		new->label = l->label;
		new->clean_text = l->clean_text;
		new->label_text = l->label_text;
		new->url = l->url;
		new->title = l->title;
		new->attributes = l->attributes;
	}

	return new;
}


/// Copy stored links to a hash for quick searching during export.
/// Links are stored via a clean version of their text(from
/// `clean_string()`) and a label version (`label_from_string()`).
/// The first link for each string is stored.
void store_link(scratch_pad * scratch, link * l) {
	link * temp_link;

	// Add link via `clean_text`?
	if (l->clean_text && l->clean_text[0] != '\0') {
		HASH_FIND_STR(scratch->link_hash, l->clean_text, temp_link);

		if (!temp_link) {
			// Only add if another link is not found with clean_text
			temp_link = link_shallow_copy(l);
			HASH_ADD_KEYPTR(hh, scratch->link_hash, l->clean_text, strlen(l->clean_text), temp_link);
		}
	}

	// Add link via `label_text`?
	if (l->label_text && l->label_text[0] != '\0') {
		HASH_FIND_STR(scratch->link_hash, l->label_text, temp_link);

		if (!temp_link) {
			// Only add if another link is not found with label_text
			temp_link = link_shallow_copy(l);
			HASH_ADD_KEYPTR(hh, scratch->link_hash, l->label_text, strlen(l->label_text), temp_link);
		}
	}
}

link * retrieve_link(scratch_pad * scratch, const char * key) {
	link * l;

	HASH_FIND_STR(scratch->link_hash, key, l);

	if (l) {
		return l;
	}

	char * clean = clean_string(key, true);

	HASH_FIND_STR(scratch->link_hash, clean, l);

	free(clean);

	return l;
}


fn_holder * fn_holder_new(footnote * f) {
	fn_holder * h = malloc(sizeof(fn_holder));

	if (h) {
		h->note = f;
	}

	return h;
}


void store_footnote(scratch_pad * scratch, footnote * f) {
	fn_holder * temp_holder;

	// Store by `clean_text`?
	if (f->clean_text && f->clean_text[0] != '\0') {
		HASH_FIND_STR(scratch->footnote_hash, f->clean_text, temp_holder);

		if (!temp_holder) {
			temp_holder = fn_holder_new(f);
			HASH_ADD_KEYPTR(hh, scratch->footnote_hash, f->clean_text, strlen(f->clean_text), temp_holder);
		}
	}

	// Store by `label_text`?
	if (f->label_text && f->label_text[0] != '\0') {
		HASH_FIND_STR(scratch->footnote_hash, f->label_text, temp_holder);

		if (!temp_holder) {
			temp_holder = fn_holder_new(f);
			HASH_ADD_KEYPTR(hh, scratch->footnote_hash, f->label_text, strlen(f->label_text), temp_holder);
		}
	}
}


void store_citation(scratch_pad * scratch, footnote * f) {
	fn_holder * temp_holder;

	// Store by `clean_text`?
	if (f->clean_text && f->clean_text[0] != '\0') {
		HASH_FIND_STR(scratch->citation_hash, f->clean_text, temp_holder);

		if (!temp_holder) {
			temp_holder = fn_holder_new(f);
			HASH_ADD_KEYPTR(hh, scratch->citation_hash, f->clean_text, strlen(f->clean_text), temp_holder);
		}
	}

	// Store by `label_text`?
	if (f->label_text && f->label_text[0] != '\0') {
		HASH_FIND_STR(scratch->citation_hash, f->label_text, temp_holder);

		if (!temp_holder) {
			temp_holder = fn_holder_new(f);
			HASH_ADD_KEYPTR(hh, scratch->citation_hash, f->label_text, strlen(f->label_text), temp_holder);
		}
	}
}


void store_glossary(scratch_pad * scratch, footnote * f) {
	fn_holder * temp_holder;

	// Store by `clean_text`?
	if (f->clean_text && f->clean_text[0] != '\0') {
		HASH_FIND_STR(scratch->glossary_hash, f->clean_text, temp_holder);

		if (!temp_holder) {
			temp_holder = fn_holder_new(f);
			HASH_ADD_KEYPTR(hh, scratch->glossary_hash, f->clean_text, strlen(f->clean_text), temp_holder);
		}
	}

	// Store by `label_text`?
	if (f->label_text && f->label_text[0] != '\0') {
		HASH_FIND_STR(scratch->glossary_hash, f->label_text, temp_holder);

		if (!temp_holder) {
			temp_holder = fn_holder_new(f);
			HASH_ADD_KEYPTR(hh, scratch->glossary_hash, f->label_text, strlen(f->label_text), temp_holder);
		}
	}
}


void store_metadata(scratch_pad * scratch, meta * m) {
	meta * temp;

	// Store by `key`
	if (m->key && m->key[0] != '\0') {
		HASH_FIND_STR(scratch->meta_hash, m->key, temp);

		if (!temp) {
			HASH_ADD_KEYPTR(hh, scratch->meta_hash, m->key, strlen(m->key), m);
		}
	}
}


void store_abbreviation(scratch_pad * scratch, footnote * f) {
	fn_holder * temp_holder;

	// Store by `label_text`
	if (f->label_text && f->label_text[0] != '\0') {
		HASH_FIND_STR(scratch->abbreviation_hash, f->label_text, temp_holder);

		if (!temp_holder) {
			temp_holder = fn_holder_new(f);
			HASH_ADD_KEYPTR(hh, scratch->abbreviation_hash, f->label_text, strlen(f->label_text), temp_holder);
		}
	}
}


void link_free(link * l) {
	free(l->label_text);
	free(l->clean_text);
	free(l->url);
	free(l->title);
//	free(l->id);

	attr * a = l->attributes;
	attr * b;

	while (a) {
		b = a->next;
		free(a->key);
		free(a->value);
		free(a);
		a = b;
	}

	free(l);
}


void whitespace_accept(token ** remainder) {
	while (token_chain_accept_multiple(remainder, 3, NON_INDENT_SPACE, INDENT_SPACE, INDENT_TAB));
}


/// Find link based on label
link * extract_link_from_stack(scratch_pad * scratch, const char * target) {
	char * key = clean_string(target, true);

	link * temp = NULL;

	HASH_FIND_STR(scratch->link_hash, key, temp);

	free(key);

	if (temp) {
		return temp;
	}

	key = label_from_string(target);

	HASH_FIND_STR(scratch->link_hash, key, temp);

	free(key);

	return temp;
}


bool validate_url(const char * url) {
	size_t len = scan_url(url);

	return (len && len == strlen(url)) ? true : false;
}


char * destination_accept(const char * source, token ** remainder, bool validate) {
	char * url = NULL;
	char * clean = NULL;
	token * t = NULL;
	size_t start;
	size_t scan_len;

	if (*remainder == NULL) {
		return url;
	}

	switch ((*remainder)->type) {
		case PAIR_PAREN:
		case PAIR_ANGLE:
		case PAIR_QUOTE_SINGLE:
		case PAIR_QUOTE_DOUBLE:
			t = token_chain_accept_multiple(remainder, 2, PAIR_ANGLE, PAIR_PAREN);
			url = text_inside_pair(source, t);
			break;

		default:
			start = (*remainder)->start;

			// Skip any whitespace
			while (char_is_whitespace(source[start])) {
				start++;
			}

			scan_len = scan_destination(&source[start]);

			// Grab destination string
			url = my_strndup(&source[start], scan_len);

			// Advance remainder to end of destination
			while ((*remainder)->next &&
					(*remainder)->next->start < start + scan_len) {
				*remainder = (*remainder)->next;
			}

			t = (*remainder);	// We need to remember this for below
			// Move remainder beyond destination
			*remainder = (*remainder)->next;

			// Is there a space in a URL concatenated with a title or attribute?
			// e.g. [foo]: http://foo.bar/ class="foo"
			// Since only one space between URL and class, they are joined.

			if (t->type == TEXT_PLAIN) {
				// Trim leading whitespace
				token_trim_leading_whitespace(t, source);
				token_split_on_char(t, source, ' ');
				*remainder = t->next;
			}

			break;
	}

	// Is this a valid URL?
	clean = clean_string(url, false);

	if (validate && !validate_url(clean)) {
		free(clean);
		clean = NULL;
	}

	free(url);
	return clean;
}


char * url_accept(const char * source, size_t start, size_t max_len, size_t * end_pos, bool validate) {
	char * url = NULL;
	char * clean = NULL;
	size_t scan_len;

	scan_len = scan_destination(&source[start]);

	if (scan_len) {
		if (scan_len > max_len) {
			scan_len = max_len;
		}

		if (end_pos) {
			*end_pos = start + scan_len;
		}

		// Is this <foo>?
		if ((source[start] == '<') &&
				(source[start + scan_len - 1] == '>')) {
			// Strip '<' and '>'
			start++;
			scan_len -= 2;
		}

		url = my_strndup(&source[start], scan_len);

		clean = clean_string(url, false);

		if (validate && !validate_url(clean)) {
			free(clean);
			clean = NULL;
		}

		free(url);
	}

	return clean;
}


/// Extract url string from `(foo)` or `(<foo>)` or `(foo "bar")`
void extract_from_paren(token * paren, const char * source, char ** url, char ** title, char ** attributes) {
	size_t scan_len;
	size_t pos = paren->child->next->start;


	size_t attr_len;

	// Skip whitespace
	while (char_is_whitespace(source[pos])) {
		pos++;
	}

	// Grab URL
	*url = url_accept(source, pos, paren->start + paren->len - 1 - pos, &pos, false);

	// Skip whitespace
	while (char_is_whitespace(source[pos])) {
		pos++;
	}

	// Grab title, if present
	scan_len = scan_title(&source[pos]);

	if (scan_len) {
		*title = my_strndup(&source[pos + 1], scan_len - 2);
		pos += scan_len;
	}

	// Skip whitespace
	while (char_is_whitespace(source[pos])) {
		pos++;
	}

	// Grab attributes, if present
	attr_len = scan_attributes(&source[pos]);

	if (attr_len) {
		*attributes = my_strndup(&source[pos], attr_len);
	}
}


/// Create a link from an explicit "inline" link `[foo](bar)`
link * explicit_link(scratch_pad * scratch, token * bracket, token * paren, const char * source) {
	char * url_char = NULL;
	char * title_char = NULL;
	char * attr_char = NULL;
	link * l = NULL;

	extract_from_paren(paren, source, &url_char, &title_char, &attr_char);

	if (attr_char) {
		if (!(scratch->extensions & EXT_COMPATIBILITY)) {
			l = link_new(source, NULL, url_char, title_char, attr_char, LINK_INLINE);
		}
	} else {
		l = link_new(source, NULL, url_char, title_char, attr_char, LINK_INLINE);
	}

	free(url_char);
	free(title_char);
	free(attr_char);

	return l;
}


footnote * footnote_new(const char * source, token * label, token * content, bool lowercase) {
	footnote * f = malloc(sizeof(footnote));
	token * walker;

	if (f) {
		f->label = label;
		f->clean_text = (label == NULL) ? NULL : clean_inside_pair(source, label, lowercase);
		f->label_text = (label == NULL) ? NULL : label_from_token(source, label);
		f->free_para  = false;
		f->count = -1;

		if (content) {
			switch (content->type) {
				case BLOCK_PARA:
					f->content = content;
					break;

				case TEXT_PLAIN:
					token_trim_leading_whitespace(content, source);

				default:
					// Trim trailing newlines
					walker = content->tail;
					while (walker) {
						switch (walker->type) {
							case TEXT_NL:
							case TEXT_NL_SP:
								content->tail = walker->prev;
								token_free(walker);
								walker = content->tail;
								walker->next = NULL;
								break;

							default:
								walker = NULL;
								break;
						}
					}

					f->content = token_new_parent(content, BLOCK_PARA);
					f->free_para = true;
					break;
			}
		} else {
			f->content = NULL;
		}
	}

	return f;
}


void footnote_free(footnote * f) {
	if (f) {
		if (f->free_para) {
			#ifdef kUseObjectPool
			// Nothing to do here
			#else
			free(f->content);
			#endif
		}

		free(f->clean_text);
		free(f->label_text);

		free(f);
	}
}


meta * meta_new(const char * source, size_t key_start, size_t len) {
	meta * m = malloc(sizeof(meta));
	char * key;

	if (m) {
		key = my_strndup(&source[key_start], len);
		m->key = label_from_string(key);
		free(key);
		m->value = NULL;
		m->start = key_start;
	}

	return m;
}


void meta_set_value(meta * m, const char * value) {
	if (value) {
		if (m->value) {
			free(m->value);
		}

		m->value = clean_string(value, false);
	}
}


void meta_free(meta * m) {
	free(m->key);
	free(m->value);

	free(m);
}


/// Find metadata based on key
meta * extract_meta_from_stack(scratch_pad * scratch, const char * target) {
	char * key = clean_string(target, true);

	meta * temp = NULL;

	HASH_FIND_STR(scratch->meta_hash, key, temp);

	free(key);

	return temp;
}


char * extract_metadata(scratch_pad * scratch, const char * target) {
	char * clean = label_from_string(target);

	meta * m = extract_meta_from_stack(scratch, clean);
	free(clean);

	if (m) {
		return m->value;
	}

	return NULL;
}


abbr * abbr_new(const char * source, token * label, token * content) {
	abbr * a = malloc(sizeof(abbr));

	if (a) {
		a->abbr = text_inside_pair(source, label);
		a->abbr_len = strlen(a->abbr);
		a->expansion = clean_string_from_range(source, content->start, content->len, false);
		a->expansion_len = strlen(a->expansion);
	}

	return a;
}

void abbreviation_free(abbr * a) {
	if (a) {
		free(a->abbr);
		free(a->expansion);
		free(a);
	}
}


bool definition_extract(mmd_engine * e, token ** remainder) {
	char * source = e->dstr->str;
	token * label = NULL;
	token * title = NULL;
	char * url_char = NULL;
	char * title_char = NULL;
	char * attr_char = NULL;
	token * temp = NULL;
	size_t attr_len;

	link * l = NULL;
	footnote * f = NULL;

	// Store label
	label = *remainder;

	*remainder = (*remainder)->next;

	// Prepare for parsing

	// Account for settings

	switch (label->type) {
		case PAIR_BRACKET_CITATION:
		case PAIR_BRACKET_FOOTNOTE:
		case PAIR_BRACKET_GLOSSARY:
			if (e->extensions & EXT_NOTES) {
				if (!token_chain_accept(remainder, COLON)) {
					return false;
				}

				title = *remainder;		// Track first token of content in 'title'

				// Store for later use
				switch (label->type) {
					case PAIR_BRACKET_CITATION:
						f = footnote_new(e->dstr->str, label, title, true);
						stack_push(e->citation_stack, f);
						break;

					case PAIR_BRACKET_FOOTNOTE:
						f = footnote_new(e->dstr->str, label, title, true);
						stack_push(e->footnote_stack, f);
						break;

					case PAIR_BRACKET_GLOSSARY:
						f = footnote_new(e->dstr->str, label, title, false);
						stack_push(e->glossary_stack, f);
						break;
				}

				break;
			}

		case PAIR_BRACKET:

			// Reference Link Definition

			if (!token_chain_accept(remainder, COLON)) {
				return false;
			}

			// Skip space
			whitespace_accept(remainder);

			// Grab destination
			url_char = destination_accept(e->dstr->str, remainder, false);

			whitespace_accept(remainder);

			// Grab title, if present
			temp = *remainder;

			title = token_chain_accept_multiple(remainder, 2, PAIR_QUOTE_DOUBLE, PAIR_QUOTE_SINGLE);

			if (!title) {
				// See if there's a title on next line
				whitespace_accept(remainder);
				token_chain_accept_multiple(remainder, 2, TEXT_NL, TEXT_LINEBREAK);
				whitespace_accept(remainder);

				title = token_chain_accept_multiple(remainder, 2, PAIR_QUOTE_DOUBLE, PAIR_QUOTE_SINGLE);

				if (!title) {
					*remainder = temp;
				}
			}

			title_char = text_inside_pair(e->dstr->str, title);

			// Get attributes
			if ((*remainder) && (((*remainder)->type != TEXT_NL) && ((*remainder)->type != TEXT_LINEBREAK))) {
				if (!(e->extensions & EXT_COMPATIBILITY)) {
					attr_len = scan_attributes(&source[(*remainder)->start]);

					if (attr_len) {
						attr_char = my_strndup(&source[(*remainder)->start], attr_len);

						// Skip forward
						attr_len += (*remainder)->start;

						while ((*remainder) && (*remainder)->start < attr_len) {
							*remainder = (*remainder)->next;
						}
					}

					l = link_new(e->dstr->str, label, url_char, title_char, attr_char, LINK_REFERENCE);
				} else {
					// Not valid match
				}
			} else {
				l = link_new(e->dstr->str, label, url_char, title_char, attr_char, LINK_REFERENCE);
			}

			// Store link for later use
			if (l) {
				stack_push(e->link_stack, l);
			}

			break;

		case PAIR_BRACKET_VARIABLE:
			fprintf(stderr, "Process variable:\n");
			token_describe(label, e->dstr->str);
			break;

		default:
			// Rest of block is not definitions (or has already been processed)
			return false;
	}

	// Advance to next line
	token_skip_until_type_multiple(remainder, 2, TEXT_NL, TEXT_LINEBREAK);

	if (*remainder) {
		*remainder = (*remainder)->next;
	}

	// Clean up
	free(url_char);
	free(title_char);
	free(attr_char);

	return true;
}


void process_definition_block(mmd_engine * e, token * block) {
	footnote * f;

	token * label = block->child;

	if (label->type == BLOCK_PARA) {
		label = label->child;
	}

	switch (block->type) {
		case BLOCK_DEF_ABBREVIATION:
		case BLOCK_DEF_CITATION:
		case BLOCK_DEF_FOOTNOTE:
		case BLOCK_DEF_GLOSSARY:
			switch (block->type) {
				case BLOCK_DEF_ABBREVIATION:
					// Strip leading '>'' from term
					f = footnote_new(e->dstr->str, label, block->child, false);

					if (f && f->clean_text) {
						memmove(f->clean_text, &(f->clean_text)[1], strlen(f->clean_text));

						while (char_is_whitespace((f->clean_text)[0])) {
							memmove(f->clean_text, &(f->clean_text)[1], strlen(f->clean_text));
						}
					}

					// Adjust the properties
					free(f->label_text);
					f->label_text = f->clean_text;

					if (f->content->child &&
							f->content->child->next &&
							f->content->child->next->next) {
						f->clean_text = clean_string_from_range(e->dstr->str, f->content->child->next->next->start, block->start + block->len - f->content->child->next->next->start, false);
					} else {
						f->clean_text = NULL;
					}

					stack_push(e->abbreviation_stack, f);
					break;

				case BLOCK_DEF_CITATION:
					f = footnote_new(e->dstr->str, label, block->child, true);
					stack_push(e->citation_stack, f);
					break;

				case BLOCK_DEF_FOOTNOTE:
					f = footnote_new(e->dstr->str, label, block->child, true);
					stack_push(e->footnote_stack, f);
					break;

				case BLOCK_DEF_GLOSSARY:
					// Strip leading '?' from term
					f = footnote_new(e->dstr->str, label, block->child, false);

					if (f && f->clean_text) {
						memmove(f->clean_text, &(f->clean_text)[1], strlen(f->clean_text));
					}

					//if (f && f->label_text)
					//		memmove(f->label_text, &(f->label_text)[1],strlen(f->label_text));

					stack_push(e->glossary_stack, f);
					break;
			}

			label->type = TEXT_EMPTY;

			if (label->next) {
				label->next->type = TEXT_EMPTY;
			}

			strip_leading_whitespace(label, e->dstr->str);
			break;

		case BLOCK_DEF_LINK:
			definition_extract(e, &(label));
			break;

		default:
			fprintf(stderr, "process %d\n", block->type);
	}

	block->type = BLOCK_EMPTY;
}


void process_definition_stack(mmd_engine * e) {
	for (int i = 0; i < e->definition_stack->size; ++i) {
		process_definition_block(e, stack_peek_index(e->definition_stack, i));
	}
}

token * manual_label_from_header(token * h, const char * source) {
	if (!h || !h->child) {
		return NULL;
	}

	token * walker = h->child->tail;
	token * label = NULL;
	short count = 0;

	while (walker) {
		switch (walker->type) {
			case MANUAL_LABEL:
				// Already identified
				label = walker;
				walker = NULL;
				break;

			case INDENT_TAB:
			case INDENT_SPACE:
			case NON_INDENT_SPACE:
			case TEXT_NL:
			case TEXT_LINEBREAK:
			case TEXT_EMPTY:
			case MARKER_H1:
			case MARKER_H2:
			case MARKER_H3:
			case MARKER_H4:
			case MARKER_H5:
			case MARKER_H6:
				walker = walker->prev;
				break;

			case TEXT_PLAIN:
				if (walker->len == 1) {
					if (source[walker->start] == ' ') {
						walker = walker->prev;
						break;
					}
				}

				walker = NULL;
				break;

			case PAIR_BRACKET:
				label = walker;

				while (walker && walker->type == PAIR_BRACKET) {
					walker = walker->prev;
					count++;
				}

				if (count % 2 == 0) {
					// Even count
					label = NULL;
				} else {
					// Odd count
					label->type = MANUAL_LABEL;
				}

			default:
				walker = NULL;
		}
	}

	return label;
}


void process_header_to_links(mmd_engine * e, token * h) {
	char * label;

	// See if we have a manual label
	token * manual = manual_label_from_header(h, e->dstr->str);

	if (manual) {
		label = label_from_token(e->dstr->str, manual);
		h = manual;
	} else {
		label = label_from_token(e->dstr->str, h);
	}

	DString * url = d_string_new("#");

	d_string_append(url, label);

	link * l = link_new(e->dstr->str, h, url->str, NULL, NULL, LINK_AUTO);

	// Store link for later use
	stack_push(e->link_stack, l);

	d_string_free(url, true);
	free(label);
}


void process_header_stack(mmd_engine * e) {
	// NTD in compatibility mode or if disabled
	if (e->extensions & EXT_NO_LABELS) {
		return;
	}

	for (int i = 0; i < e->header_stack->size; ++i) {
		process_header_to_links(e, stack_peek_index(e->header_stack, i));
	}
}


void process_table_to_link(mmd_engine * e, token * t) {
	// Is there a caption
	if (table_has_caption(t)) {
		token * temp_token = t->next->child;

		if (temp_token->next &&
				temp_token->next->type == PAIR_BRACKET) {
			temp_token = temp_token->next;
		}

		char * label = label_from_token(e->dstr->str, temp_token);

		DString * url = d_string_new("#");
		d_string_append(url, label);

		link * l = link_new(e->dstr->str, temp_token, url->str, NULL, NULL, LINK_AUTO);

		stack_push(e->link_stack, l);

		d_string_free(url, true);
		free(label);
	}
}


void process_table_stack(mmd_engine * e) {
	for (int i = 0; i < e->table_stack->size; ++i) {
		process_table_to_link(e, stack_peek_index(e->table_stack, i));
	}
}


/// Parse metadata
void process_metadata_stack(mmd_engine * e, scratch_pad * scratch) {
	if ((scratch->extensions & EXT_NO_METADATA) ||
			(scratch->extensions & EXT_COMPATIBILITY)) {
		return;
	}

	meta * m;
	short header_level = -10;
	char * temp_char = NULL;

	for (int i = 0; i < e->metadata_stack->size; ++i) {
		// Check for certain metadata keys
		m = stack_peek_index(e->metadata_stack, i);

		if (strcmp(m->key, "baseheaderlevel") == 0) {
			if (header_level == -10) {
				header_level = atoi(m->value);
			}
		} else if (strcmp(m->key, "epubheaderlevel") == 0) {
			if (scratch->output_format == FORMAT_EPUB) {
				header_level = atoi(m->value);
			}
		} else if (strcmp(m->key, "htmlheaderlevel") == 0) {
			if (scratch->output_format == FORMAT_HTML) {
				header_level = atoi(m->value);
			}
		} else if (strcmp(m->key, "xhtmlheaderlevel") == 0) {
			if (scratch->output_format == FORMAT_HTML) {
				header_level = atoi(m->value);
			}
		} else if (strcmp(m->key, "latexheaderlevel") == 0) {
			if ((scratch->output_format == FORMAT_LATEX) ||
					(scratch->output_format == FORMAT_BEAMER) ||
					(scratch->output_format == FORMAT_MEMOIR)) {
				header_level = atoi(m->value);
			}
		} else if (strcmp(m->key, "odfheaderlevel") == 0) {
			if ((scratch->output_format == FORMAT_ODT) ||
					(scratch->output_format == FORMAT_FODT)) {
				header_level = atoi(m->value);
			}
		} else if (strcmp(m->key, "language") == 0) {
			temp_char = label_from_string(m->value);

			if (strcmp(temp_char, "de") == 0) {
				scratch->language = LC_DE;
				scratch->quotes_lang = GERMAN;
			} else if (strcmp(temp_char, "es") == 0) {
				scratch->language = LC_ES;
				scratch->quotes_lang = SPANISH;
			} else if (strcmp(temp_char, "fr") == 0) {
				scratch->language = LC_FR;
				scratch->quotes_lang = FRENCH;
			} else if (strcmp(temp_char, "nl") == 0) {
				scratch->language = LC_NL;
				scratch->quotes_lang = DUTCH;
			} else if (strcmp(temp_char, "sv") == 0) {
				scratch->language = LC_SV;
				scratch->quotes_lang = SWEDISH;
			} else {
				scratch->language = LC_EN;
				scratch->quotes_lang = ENGLISH;
			}

			free(temp_char);
		} else if (strcmp(m->key, "latexmode") == 0) {
			if (scratch->output_format == FORMAT_LATEX) {
				temp_char = label_from_string(m->value);

				if (strcmp(temp_char, "beamer") == 0) {
					scratch->output_format = FORMAT_BEAMER;
				} else if (strcmp(temp_char, "memoir") == 0) {
					scratch->output_format = FORMAT_MEMOIR;
				}

				free(temp_char);
			}
		} else if (strcmp(m->key, "quoteslanguage") == 0) {
			temp_char = label_from_string(m->value);

			if ((strcmp(temp_char, "dutch") == 0) ||
					(strcmp(temp_char, "nl") == 0)) {
				scratch->quotes_lang = DUTCH;
			} else if ((strcmp(temp_char, "french") == 0) ||
					   (strcmp(temp_char, "fr") == 0)) {
				scratch->quotes_lang = FRENCH;
			} else if ((strcmp(temp_char, "german") == 0) ||
					   (strcmp(temp_char, "de") == 0)) {
				scratch->quotes_lang = GERMAN;
			} else if (strcmp(temp_char, "germanguillemets") == 0) {
				scratch->quotes_lang = GERMANGUILL;
			} else if ((strcmp(temp_char, "spanish") == 0) ||
					   (strcmp(temp_char, "es") == 0)) {
				scratch->quotes_lang = SPANISH;
			} else if ((strcmp(temp_char, "swedish") == 0) ||
					   (strcmp(temp_char, "sv") == 0)) {
				scratch->quotes_lang = SWEDISH;
			} else {
				scratch->quotes_lang = ENGLISH;
			}

			free(temp_char);
		} else if (strcmp(m->key, "bibtex") == 0) {
			scratch->bibtex_file = my_strdup(m->value);

			// Trigger complete document unless explicitly denied
			if (!(scratch->extensions & EXT_SNIPPET)) {
				scratch->extensions |= EXT_COMPLETE;
			}
		} else {
			// Any other key triggers complete document
			if (!(scratch->extensions & EXT_SNIPPET)) {
				scratch->extensions |= EXT_COMPLETE;
			}
		}

	}

	if (header_level != -10) {
		scratch->base_header_level = header_level;
	}
}


void automatic_search_text(mmd_engine * e, token * t, trie * ac) {
	match * m = ac_trie_leftmost_longest_search(ac, e->dstr->str, t->start, t->len);

	match * walker;

	token * tok = t;

	if (m) {
		walker = m->next;

		while (walker) {
			token_split(tok, walker->start, walker->len, walker->match_type);

			// Advance token to next token
			while (tok && (tok->start < walker->start + walker->len)) {
				tok = tok->next;
			}

			// Advance to next match (if present)
			walker = walker->next;
		}
	}

	match_free(m);
}


/// Determine which nodes to descend into to search for abbreviations
void automatic_search(mmd_engine * e, token * t, trie * ac) {
	while (t) {
		switch (t->type) {
			case TEXT_PLAIN:
				automatic_search_text(e, t, ac);
				break;

			case DOC_START_TOKEN:
			case BLOCK_BLOCKQUOTE:
			case BLOCK_DEFINITION:
			case BLOCK_DEFLIST:
			case BLOCK_LIST_BULLETED:
			case BLOCK_LIST_BULLETED_LOOSE:
			case BLOCK_LIST_ENUMERATED:
			case BLOCK_LIST_ENUMERATED_LOOSE:
			case BLOCK_LIST_ITEM_TIGHT:
			case BLOCK_LIST_ITEM:
			case BLOCK_PARA:
			case BLOCK_TABLE:
			case BLOCK_TABLE_HEADER:
			case BLOCK_TABLE_SECTION:
			case BLOCK_TERM:
			case LINE_LIST_BULLETED:
			case LINE_LIST_ENUMERATED:
			case PAIR_BRACKET:
			case PAIR_BRACKET_FOOTNOTE:
			case PAIR_BRACKET_GLOSSARY:
			case PAIR_BRACKET_IMAGE:
			case PAIR_QUOTE_DOUBLE:
			case PAIR_QUOTE_SINGLE:
			case PAIR_STAR:
			case PAIR_UL:
			case TABLE_CELL:
			case TABLE_ROW:
				automatic_search(e, t->child, ac);
				break;

//			case PAIR_PAREN:
			default:
				break;
		}

		t = t->next;
	}
}


void identify_global_search_terms(mmd_engine * e, scratch_pad * scratch) {
	// Only search if we have a target
	size_t count = e->abbreviation_stack->size + e->glossary_stack->size;

	if (count == 0) {
		return;
	}

	trie * ac = trie_new(0);
	footnote * f;

	// Add abbreviations to search trie
	for (int i = 0; i < e->abbreviation_stack->size; ++i) {
		f = stack_peek_index(e->abbreviation_stack, i);
		trie_insert(ac, f->label_text, PAIR_BRACKET_ABBREVIATION);
	}

	// Add glossary to search trie (without leading '?')
	for (int i = 0; i < e->glossary_stack->size; ++i) {
		f = stack_peek_index(e->glossary_stack, i);
		trie_insert(ac, f->clean_text, PAIR_BRACKET_GLOSSARY);
	}

	ac_trie_prepare(ac);
	automatic_search(e, e->root, ac);
	trie_free(ac);
}


void mmd_engine_export_token_tree(DString * out, mmd_engine * e, short format) {

	// Process potential reference definitions
	process_definition_stack(e);

	// Process headers for potential cross-reference targets
	process_header_stack(e);

	// Process tables for potential cross-reference targets
	process_table_stack(e);

	// Create scratch pad
	scratch_pad * scratch = scratch_pad_new(e, format);

	// Process metadata
	process_metadata_stack(e, scratch);

	// Process abbreviations, glossary, etc.
	if (!(e->extensions & EXT_COMPATIBILITY)) {
		identify_global_search_terms(e, scratch);
	}


	switch (scratch->output_format) {
		case FORMAT_BEAMER:
			if (scratch->extensions & EXT_COMPLETE) {
				mmd_start_complete_latex(out, e->dstr->str, scratch);
			}

			mmd_export_token_tree_beamer(out, e->dstr->str, e->root, scratch);

			// Close out any existing outline levels
			mmd_outline_add_beamer(out, NULL, scratch);

			mmd_export_citation_list_beamer(out, e->dstr->str, scratch);

			if (scratch->extensions & EXT_COMPLETE) {
				mmd_end_complete_beamer(out, e->dstr->str, scratch);
			}

			break;

		case FORMAT_EPUB:
		case FORMAT_TEXTBUNDLE:
		case FORMAT_TEXTBUNDLE_COMPRESSED:
			scratch->store_assets = true;

			mmd_start_complete_html(out, e->dstr->str, scratch);

			mmd_export_token_tree_html(out, e->dstr->str, e->root, scratch);
			mmd_export_footnote_list_html(out, e->dstr->str, scratch);
			mmd_export_glossary_list_html(out, e->dstr->str, scratch);
			mmd_export_citation_list_html(out, e->dstr->str, scratch);

			mmd_end_complete_html(out, e->dstr->str, scratch);

			break;

		case FORMAT_HTML:
			if (scratch->extensions & EXT_COMPLETE) {
				mmd_start_complete_html(out, e->dstr->str, scratch);
			}

			mmd_export_token_tree_html(out, e->dstr->str, e->root, scratch);
			mmd_export_footnote_list_html(out, e->dstr->str, scratch);
			mmd_export_glossary_list_html(out, e->dstr->str, scratch);
			mmd_export_citation_list_html(out, e->dstr->str, scratch);

			if (scratch->extensions & EXT_COMPLETE) {
				mmd_end_complete_html(out, e->dstr->str, scratch);
			}

			break;

		case FORMAT_LATEX:
			if (scratch->extensions & EXT_COMPLETE) {
				mmd_start_complete_latex(out, e->dstr->str, scratch);
			}

			mmd_export_token_tree_latex(out, e->dstr->str, e->root, scratch);
			mmd_export_citation_list_latex(out, e->dstr->str, scratch);

			if (scratch->extensions & EXT_COMPLETE) {
				mmd_end_complete_latex(out, e->dstr->str, scratch);
			}

			break;

		case FORMAT_MEMOIR:
			if (scratch->extensions & EXT_COMPLETE) {
				mmd_start_complete_latex(out, e->dstr->str, scratch);
			}

			mmd_export_token_tree_memoir(out, e->dstr->str, e->root, scratch);
			mmd_export_citation_list_latex(out, e->dstr->str, scratch);

			if (scratch->extensions & EXT_COMPLETE) {
				mmd_end_complete_latex(out, e->dstr->str, scratch);
			}

			break;

		case FORMAT_ODT:
			scratch->store_assets = true;

		case FORMAT_FODT:
//			mmd_start_complete_odf(out, e->dstr->str, scratch);

			mmd_export_token_tree_opendocument(out, e->dstr->str, e->root, scratch);

//			mmd_end_complete_odf(out, e->dstr->str, scratch);
			break;

		case FORMAT_OPML:
			mmd_export_token_tree_opml(out, e->dstr->str, e->root, scratch);
			break;
	}

	// Preserve asset_hash for possible use in export
	e->asset_hash = scratch->asset_hash;

	scratch_pad_free(scratch);
}


void parse_brackets(const char * source, scratch_pad * scratch, token * bracket, link ** final_link, short * skip_token, bool * free_link) {
	link * temp_link = NULL;
	char * temp_char = NULL;
	short temp_short = 0;

	// What is next?
	token * next = bracket->next;

	if (next) {
		temp_short = 1;
	}

	// Do not free this link after using it
	*free_link = false;

	if (next && next->type == PAIR_PAREN) {
		// We have `[foo](bar)` or `![foo](bar)`

		temp_link = explicit_link(scratch, bracket, next, source);

		if (temp_link) {
			// Don't output brackets
			bracket->child->type = TEXT_EMPTY;
			bracket->child->mate->type = TEXT_EMPTY;

			// This was an explicit link
			*final_link = temp_link;

			// Skip over parentheses
			*skip_token = temp_short;

			// Free this link
			*free_link = true;
			return;
		}
	}

	if (next && next->type == PAIR_BRACKET) {
		// Is this a reference link? `[foo][bar]` or `![foo][bar]`
		temp_char = text_inside_pair(source, next);

		if (temp_char[0] == '\0') {
			// Empty label, use first bracket (e.g. implicit link `[foo][]`)
			free(temp_char);
			temp_char = text_inside_pair(source, bracket);
		}
	} else {
		// This may be a simplified implicit link, e.g. `[foo]`

		// But not if it's nested brackets, since it would not
		// end up being a valid reference
		token * walker = bracket->child;

		while (walker) {
			switch (walker->type) {
				case PAIR_BRACKET:
				case PAIR_BRACKET_CITATION:
				case PAIR_BRACKET_FOOTNOTE:
				case PAIR_BRACKET_GLOSSARY:
				case PAIR_BRACKET_VARIABLE:
				case PAIR_BRACKET_ABBREVIATION:
					*final_link = NULL;
					return;
			}

			walker = walker->next;
		}

		temp_char = text_inside_pair(source, bracket);
		// Don't skip tokens
		temp_short = 0;
	}

	temp_link = extract_link_from_stack(scratch, temp_char);

	if (temp_char) {
		free(temp_char);
	}

	if (temp_link) {
		// Don't output brackets
		if (bracket->child) {
			bracket->child->type = TEXT_EMPTY;

			if (bracket->child->mate) {
				bracket->child->mate->type = TEXT_EMPTY;
			}
		}

		*final_link = temp_link;

		// Skip over second bracket if present
		*skip_token = temp_short;
		return;
	}

	// No existing links, so nothing to do
	*final_link = NULL;
}


void mark_citation_as_used(scratch_pad * scratch, footnote * c) {
	if (c->count == -1) {
		// Add citation to used stack
		stack_push(scratch->used_citations, c);

		// Update counter
		c->count = scratch->used_citations->size;
	}
}


void mark_footnote_as_used(scratch_pad * scratch, footnote * f) {
	if (f->count == -1) {
		// Add footnote to used stack
		stack_push(scratch->used_footnotes, f);

		// Update counter
		f->count = scratch->used_footnotes->size;
	}
}


void mark_glossary_as_used(scratch_pad * scratch, footnote * c) {
	if (c->count == -1) {
		// Add glossary to used stack
		stack_push(scratch->used_glossaries, c);

		// Update counter
		c->count = scratch->used_glossaries->size;
	}
}


void mark_abbreviation_as_used(scratch_pad * scratch, footnote * c) {
	if (c->count == -1) {
		// Add abbreviation to used stack
		stack_push(scratch->used_abbreviations, c);

		// Update counter
		c->count = scratch->used_abbreviations->size;
	}
}


size_t extract_citation_from_stack(scratch_pad * scratch, const char * target) {
	char * key = clean_string(target, true);

	fn_holder * h;

	HASH_FIND_STR(scratch->citation_hash, key, h);

	free(key);

	if (h) {
		mark_citation_as_used(scratch, h->note);
		return h->note->count;
	}

	key = label_from_string(target);

	HASH_FIND_STR(scratch->citation_hash, key, h);

	free(key);

	if (h) {
		mark_citation_as_used(scratch, h->note);
		return h->note->count;
	}

	// None found
	return -1;
}


size_t extract_footnote_from_stack(scratch_pad * scratch, const char * target) {
	char * key = clean_string(target, true);

	fn_holder * h;

	HASH_FIND_STR(scratch->footnote_hash, key, h);

	free(key);

	if (h) {
		mark_footnote_as_used(scratch, h->note);
		return h->note->count;
	}

	key = label_from_string(target);

	HASH_FIND_STR(scratch->footnote_hash, key, h);

	free(key);

	if (h) {
		mark_footnote_as_used(scratch, h->note);
		return h->note->count;
	}

	// None found
	return -1;
}


size_t extract_abbreviation_from_stack(scratch_pad * scratch, const char * target) {
	char * key = clean_string(target, false);

	fn_holder * h;

	HASH_FIND_STR(scratch->abbreviation_hash, key, h);

	free(key);

	if (h) {
		mark_abbreviation_as_used(scratch, h->note);
		return h->note->count;
	}

	key = label_from_string(target);

	HASH_FIND_STR(scratch->abbreviation_hash, key, h);

	free(key);

	if (h) {
		mark_abbreviation_as_used(scratch, h->note);
		return h->note->count;
	}

	// None found
	return -1;
}


size_t extract_glossary_from_stack(scratch_pad * scratch, const char * target) {
	char * key = clean_string(target, false);

	fn_holder * h;

	HASH_FIND_STR(scratch->glossary_hash, key, h);

	free(key);

	if (h) {
		mark_glossary_as_used(scratch, h->note);
		return h->note->count;
	}

	key = label_from_string(target);

	HASH_FIND_STR(scratch->glossary_hash, key, h);

	free(key);

	if (h) {
		mark_glossary_as_used(scratch, h->note);
		return h->note->count;
	}

	// None found
	return -1;
}


void footnote_from_bracket(const char * source, scratch_pad * scratch, token * t, short * num) {
	// Get text inside bracket
	char * text = text_inside_pair(source, t);
	short footnote_id = extract_footnote_from_stack(scratch, text);

	free(text);

	if (footnote_id == -1) {
		// No match, this is an inline footnote -- create a new one
		t->child->type = TEXT_EMPTY;
		t->child->mate->type = TEXT_EMPTY;

		// Create footnote
		footnote * temp = footnote_new(source, NULL, t->child, true);

		// Store as used
		stack_push(scratch->used_footnotes, temp);
		*num = scratch->used_footnotes->size;
		temp->count = *num;

		// We need to free this one later since it doesn't exist
		// in the engine's stack, on the scratch_pad stack
		stack_push(scratch->inline_footnotes_to_free, temp);
	} else {
		// Footnote in stack
		*num = footnote_id;
	}
}


void citation_from_bracket(const char * source, scratch_pad * scratch, token * t, short * num) {
	// Get text inside bracket
	char * text = text_inside_pair(source, t);
	short citation_id = extract_citation_from_stack(scratch, text);

	free(text);

	if (citation_id == -1) {
		// No match, this is an inline citation -- create a new one

		t->child->type = TEXT_EMPTY;
		t->child->mate->type = TEXT_EMPTY;

		// *UNLESS* we are using BibTeX, in which case we leave them alone
		if (scratch->bibtex_file) {
			*num = -1;
			return;
		}

		// Create citation
		footnote * temp = footnote_new(source, t, t->child, true);

		// Store as used
		stack_push(scratch->used_citations, temp);
		*num = scratch->used_citations->size;
		temp->count = *num;

		// We need to free this one later since it doesn't exist
		// in the engine's stack, on the scratch_pad stack
		stack_push(scratch->inline_citations_to_free, temp);
	} else {
		// Citation in stack
		*num = citation_id;
	}
}


void glossary_from_bracket(const char * source, scratch_pad * scratch, token * t, short * num) {
	// Get text inside bracket
	char * text;

	if (t->child) {
		text = text_inside_pair(source, t);
		memmove(text, &text[1], strlen(text));
	} else {
		text = malloc(t->len + 1);
		memcpy(text, &source[t->start], t->len);
		text[t->len] = '\0';
	}

	short glossary_id = extract_glossary_from_stack(scratch, text);

	free(text);

	if (glossary_id == -1) {
		// No match, this is an inline glossary -- create a new glossary entry
		if (t->child) {
			t->child->type = TEXT_EMPTY;
			t->child->mate->type = TEXT_EMPTY;
		}

		// Create glossary
		token * label = t->child;

		while (label && label->type != PAIR_PAREN) {
			label = label->next;
		}

		if (label) {
			footnote * temp = footnote_new(source, label, label->next, false);

			// Store as used
			stack_push(scratch->used_glossaries, temp);
			*num = scratch->used_glossaries->size;
			temp->count = *num;

			// We need to free this one later since it doesn't exist
			// in the engine's stack, on the scratch_pad stack
			stack_push(scratch->inline_glossaries_to_free, temp);
		} else {
			// Improperly formatted glossary
			*num = -1;
		}
	} else {
		// Glossary in stack
		*num = glossary_id;
	}
}


void abbreviation_from_bracket(const char * source, scratch_pad * scratch, token * t, short * num) {
	// Get text inside bracket
	char * text;

	if (t->child) {
		text = text_inside_pair(source, t);
	} else {
		text = malloc(t->len + 2);
		text[0] = '>';
		memcpy(&text[1], &source[t->start], t->len);
		text[t->len + 1] = '\0';
	}

	short abbr_id = extract_abbreviation_from_stack(scratch, &text[1]);

	free(text);

	if (abbr_id == -1) {
		// No match, this is an inline glossary -- create a new glossary entry
		if (t->child) {
			t->child->type = TEXT_EMPTY;
			t->child->mate->type = TEXT_EMPTY;
		}

		// Create glossary
		token * label = t->child;

		while (label && label->type != PAIR_PAREN) {
			label = label->next;
		}

		if (label) {
			footnote * temp = footnote_new(source, label, label->next, false);

			// Adjust the properties
			free(temp->label_text);
			temp->label_text = temp->clean_text;

			if (temp->content && temp->content->child) {
				temp->clean_text = clean_string_from_range(source, temp->content->child->start, t->start + t->len - t->child->mate->len - temp->content->child->start, false);
			}

			// Store as used
			stack_push(scratch->used_abbreviations, temp);
			*num = scratch->used_abbreviations->size;
			temp->count = *num;

			// We need to free this one later since it doesn't exist
			// in the engine's stack, on the scratch_pad stack
			stack_push(scratch->inline_abbreviations_to_free, temp);
		} else {
			// Improperly formatted glossary
			*num = -1;
		}
	} else {
		// Glossary in stack
		*num = abbr_id;
	}
}


void read_table_column_alignments(const char * source, token * table, scratch_pad * scratch) {
	token * walker = table->child->child;

	scratch->table_alignment[0] = '\0';
	scratch->table_column_count = 0;

	if (walker == NULL) {
		return;
	}

	// Find the separator line
	while (walker->next) {
		walker = walker->next;
	}

	walker->type = TEXT_EMPTY;

	// Iterate through cells to create alignment string
	short counter = 0;
	short align = 0;

	walker = walker->child;

	while (walker) {
		switch (walker->type) {
			case TABLE_CELL:
				align = scan_alignment_string(&source[walker->start]);

				switch (align) {
					case ALIGN_LEFT:
						scratch->table_alignment[counter] = 'l';
						break;

					case ALIGN_RIGHT:
						scratch->table_alignment[counter] = 'r';
						break;

					case ALIGN_CENTER:
						scratch->table_alignment[counter] = 'c';
						break;

					case ALIGN_LEFT | ALIGN_WRAP:
						scratch->table_alignment[counter] = 'L';
						break;

					case ALIGN_RIGHT | ALIGN_WRAP:
						scratch->table_alignment[counter] = 'R';
						break;

					case ALIGN_CENTER | ALIGN_WRAP:
						scratch->table_alignment[counter] = 'C';
						break;

					case ALIGN_WRAP:
						scratch->table_alignment[counter] = 'N';
						break;

					default:
						scratch->table_alignment[counter] = 'n';
				}

				counter++;
				break;
		}

		walker = walker->next;
	}

	scratch->table_alignment[counter] = '\0';
	scratch->table_column_count = counter;
}


void strip_leading_whitespace(token * chain, const char * source) {
	while (chain) {
		switch (chain->type) {
			case INDENT_TAB:
			case INDENT_SPACE:
			case NON_INDENT_SPACE:
				chain->type = TEXT_EMPTY;

			case TEXT_EMPTY:
				chain = chain->next;
				break;

			case TEXT_PLAIN:
				token_trim_leading_whitespace(chain, source);

			default:
				return;
		}

		if (chain) {
			chain = chain->next;
		}
	}
}


bool table_has_caption(token * t) {

	if (t->next && t->next->type == BLOCK_PARA) {
		t = t->next->child;

		if (t->type == PAIR_BRACKET) {
			t = t->next;

			if (t && t->next &&
					t->next->type == PAIR_BRACKET) {
				t = t->next;
			}

			if (t == NULL) {
				// End of file
				return true;
			}

			if (t && t->next &&
					((t->next->type == TEXT_NL) ||
					 (t->next->type == TEXT_LINEBREAK))) {
				t = t->next;
			}

			if (t && t->next == NULL) {
				return true;
			}
		}
	}

	return false;
}


/// Grab the first "word" after the end of the fence marker:
/// ````perl
/// or
/// ```` perl
char * get_fence_language_specifier(token * fence, const char * source) {
	char * result = NULL;
	size_t start = fence->start + fence->len;
	size_t len = 0;

	while (char_is_whitespace(source[start])) {
		start++;
	}

	while (!char_is_whitespace_or_line_ending(source[start + len])) {
		len++;
	}

	if (len) {
		result = my_strndup(&source[start], len);
	}

	return result;
}


short raw_level_for_header(token * header) {
	switch (header->type) {
		case BLOCK_H1:
		case BLOCK_SETEXT_1:
			return 1;

		case BLOCK_H2:
		case BLOCK_SETEXT_2:
			return 2;

		case BLOCK_H3:
			return 3;

		case BLOCK_H4:
			return 4;

		case BLOCK_H5:
			return 5;

		case BLOCK_H6:
			return 6;
	}

	return 0;
}


asset * asset_new(char * url, scratch_pad * scratch) {
	asset * a = malloc(sizeof(asset));

	if (a) {
		a->url = my_strdup(url);

		// Create a unique local asset path
		a->asset_path = uuid_new();
	}

	return a;
}


void asset_free(asset * a) {
	if (a) {
		free(a->url);
		free(a->asset_path);
	}

	free(a);
}


asset * extract_asset(scratch_pad * scratch, char * url) {
	asset * a;

	HASH_FIND_STR(scratch->asset_hash, url, a);

	return a;
}


void store_asset(scratch_pad * scratch, char * url) {
	asset * a = extract_asset(scratch, url);

	// Only store if this url has not already been stored
	if (!a) {
		// Asset not found - create new one
		a = asset_new(url, scratch);
		HASH_ADD_KEYPTR(hh, scratch->asset_hash, a->url, strlen(a->url), a);
	}
}


bool raw_filter_text_matches(char * pattern, short format) {
	if (!pattern) {
		return false;
	}

	if (strcmp("*", pattern) == 0) {
		return true;
	} else if (strcmp("{=*}", pattern) == 0) {
		return true;
	} else {
		switch (format) {
			case FORMAT_HTML:
				if (strstr(pattern, "html")) {
					return true;
				}

				break;

			case FORMAT_ODT:
			case FORMAT_FODT:
				if (strstr(pattern, "odt")) {
					return true;
				}

				break;

			case FORMAT_EPUB:
				if (strstr(pattern, "epub")) {
					return true;
				}

				break;

			case FORMAT_MEMOIR:
			case FORMAT_BEAMER:
			case FORMAT_LATEX:
				if (strstr(pattern, "latex")) {
					return true;
				}

				break;
		}
	}

	return false;
}


/// Determine whether raw filter matches specified format
bool raw_filter_matches(token * t, const char * source, short format) {
	bool result = false;

	if (t->type != PAIR_RAW_FILTER) {
		return result;
	}

	char * pattern = my_strndup(&source[t->child->start + 2], t->child->mate->start - t->child->start - 2);

	result = raw_filter_text_matches(pattern, format);

	free(pattern);

	return result;
}

