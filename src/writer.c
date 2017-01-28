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

#include "char.h"
#include "d_string.h"
#include "html.h"
#include "mmd.h"
#include "scanners.h"
#include "token.h"
#include "writer.h"


void store_citation(scratch_pad * scratch, footnote * f);

void store_footnote(scratch_pad * scratch, footnote * f);

void store_link(scratch_pad * scratch, link * l);

void store_metadata(scratch_pad * scratch, meta * m);


/// Temporary storage while exporting parse tree to output format
scratch_pad * scratch_pad_new(mmd_engine * e) {
	scratch_pad * p = malloc(sizeof(scratch_pad));

	if (p) {
		p->padded = 2;							// Prevent unnecessary leading space
		p->list_is_tight = false;				// Tight vs Loose list
		p->skip_token = 0;						// Skip over next n tokens

		p->extensions = e->extensions;
		p->quotes_lang = e->quotes_lang;
		p->language = e->language;

		// Store links in a hash for rapid retrieval when exporting
		p->link_hash = NULL;
		link * l;

		for (int i = 0; i < e->link_stack->size; ++i)
		{
			l = stack_peek_index(e->link_stack, i);

			store_link(p, l);
		}

		// Store footnotes in a hash for rapid retrieval when exporting
		p->used_footnotes = stack_new(0);				// Store footnotes as we use them
		p->inline_footnotes_to_free = stack_new(0);		// Inline footnotes need to be freed
		p->footnote_being_printed = 0;
		p->footnote_para_counter = -1;

		p->footnote_hash = NULL;				// Store defined footnotes in a hash

		footnote * f;

		for (int i = 0; i < e->footnote_stack->size; ++i)
		{
			f = stack_peek_index(e->footnote_stack, i);

			store_footnote(p, f);
		}

		// Store citations in a hash for rapid retrieval when exporting
		p->used_citations = stack_new(0);
		p->inline_citations_to_free = stack_new(0);
		p->citation_being_printed = 0;

		p->citation_hash = NULL;

		for (int i = 0; i < e->citation_stack->size; ++i)
		{
			f = stack_peek_index(e->citation_stack, i);

			store_citation(p, f);
		}

		// Store links in a hash for rapid retrieval when exporting
		p->meta_hash = NULL;
		meta * m;

		for (int i = 0; i < e->metadata_stack->size; ++i)
		{
			m = stack_peek_index(e->metadata_stack, i);

			store_metadata(p, m);
		}
	}

	return p;
}


void scratch_pad_free(scratch_pad * scratch) {
//	HASH_CLEAR(hh, scratch->link_hash);

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
		result = strndup(&source[pair->start + pair->child->len], pair->len - (pair->child->len + 1));
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
			|| (*str >= 'a' && *str <= 'z') || (*str == '.') || (*str== '_')
			|| (*str== '-') || (*str== ':'))
		{
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


/// Clean up whitespace in string for standardization
char * clean_string(const char * str, bool lowercase) {
	if (str == NULL)
		return NULL;
	
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
				if (lowercase)
					d_string_append_c(out, tolower(*str));
				else
					d_string_append_c(out, *str);

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


char * clean_string_from_token(const char * source, token * t, bool lowercase) {
	char * clean = NULL;

	DString * raw = d_string_new("");

	d_string_append_c_array(raw, &source[t->start], t->len);

	clean = clean_string(raw->str, lowercase);

	d_string_free(raw, true);

	return clean;
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
		a->value = strdup(value);
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

	while (scan_attr(&source[pos])) {
		pos +=  scan_spnl(&source[pos]);

		// Get key
		scan_len = scan_key(&source[pos]);
		key = strndup(&source[pos], scan_len);
		
		// Skip '='
		pos += scan_len + 1;

		// Get value
		scan_len = scan_value(&source[pos]);
		value = strndup(&source[pos], scan_len);

		pos += scan_len;

		if (a) {
			a->next = attr_new(key, value);
			a = a->next;
		} else {
			a = attr_new(key, value);
			attributes = a;
		}

		free(value);	// We stored a copy
	}

	return attributes;
}


link * link_new(const char * source, token * label, char * url, char * title, char * attributes) {
	link * l = malloc(sizeof(link));

	if (l) {
		l->label = label;
		l->clean_text = clean_inside_pair(source, label, true);
		l->label_text = label_from_token(source, label);
		l->url = clean_string(url, false);
		l->title = (title == NULL) ? NULL : strdup(title);
		l->attributes = (attributes == NULL) ? NULL : parse_attributes(attributes);
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
	HASH_FIND_STR(scratch->link_hash, l->clean_text, temp_link);
	
	if (!temp_link) {
		// Only add if another link is not found with clean_text
		temp_link = link_shallow_copy(l);
		HASH_ADD_KEYPTR(hh, scratch->link_hash, l->clean_text, strlen(l->clean_text), temp_link);
	}

	// Add link via `label_text`?
	HASH_FIND_STR(scratch->link_hash, l->label_text, temp_link);

	if (!temp_link) {
		// Only add if another link is not found with label_text
		temp_link = link_shallow_copy(l);
		HASH_ADD_KEYPTR(hh, scratch->link_hash, l->label_text, strlen(l->label_text), temp_link);
	}
}

link * retrieve_link(scratch_pad * scratch, const char * key) {
	link * l;

	HASH_FIND_STR(scratch->link_hash, key, l);

	if (l)
		return l;

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
	HASH_FIND_STR(scratch->footnote_hash, f->clean_text, temp_holder);

	if (!temp_holder) {
		temp_holder = fn_holder_new(f);
		HASH_ADD_KEYPTR(hh, scratch->footnote_hash, f->clean_text, strlen(f->clean_text), temp_holder);
	}

	// Store by `label_text`?
	HASH_FIND_STR(scratch->footnote_hash, f->label_text, temp_holder);

	if (!temp_holder) {
		temp_holder = fn_holder_new(f);
		HASH_ADD_KEYPTR(hh, scratch->footnote_hash, f->label_text, strlen(f->label_text), temp_holder);
	}
}


void store_citation(scratch_pad * scratch, footnote * f) {
	fn_holder * temp_holder;

	// Store by `clean_text`?
	HASH_FIND_STR(scratch->citation_hash, f->clean_text, temp_holder);

	if (!temp_holder) {
		temp_holder = fn_holder_new(f);
		HASH_ADD_KEYPTR(hh, scratch->citation_hash, f->clean_text, strlen(f->clean_text), temp_holder);
	}

	// Store by `label_text`?
	HASH_FIND_STR(scratch->citation_hash, f->label_text, temp_holder);

	if (!temp_holder) {
		temp_holder = fn_holder_new(f);
		HASH_ADD_KEYPTR(hh, scratch->citation_hash, f->label_text, strlen(f->label_text), temp_holder);
	}
}


void store_metadata(scratch_pad * scratch, meta * m) {
	meta * temp;

	// Store by `key`
	HASH_FIND_STR(scratch->meta_hash, m->key, temp);

	if (!temp) {
		HASH_ADD_KEYPTR(hh, scratch->meta_hash, m->key, strlen(m->key), m);
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

	if (temp)
		return temp;

	key = label_from_string(target);

	HASH_FIND_STR(scratch->link_hash, key, temp);

	free(key);

	return temp;
}


bool validate_url(const char * url) {
	size_t len = scan_url(url);

	return (len && len == strlen(url)) ? true : false;
}


char * url_accept(const char * source, token ** remainder, bool validate) {
	char * url = NULL;
	char * clean = NULL;
	token * t = NULL;
	token * first = NULL;
	token * last = NULL;

	switch ((*remainder)->type) {
		case PAIR_PAREN:
		case PAIR_ANGLE:
		case PAIR_QUOTE_SINGLE:
		case PAIR_QUOTE_DOUBLE:
			t = token_chain_accept_multiple(remainder, 2, PAIR_ANGLE, PAIR_PAREN);
			url = text_inside_pair(source, t);
			break;
		case TEXT_PLAIN:
			first = *remainder;
			
			// Grab parts for URL
			while (token_chain_accept_multiple(remainder, 5, AMPERSAND, COLON, TEXT_PERIOD, TEXT_PLAIN, UL));

			last = (*remainder)->prev;

			// Is there a space in a URL concatenated with a title or attribute?
			// e.g. [foo]: http://foo.bar/ class="foo"
			// Since only one space between URL and class, they are joined.

			if (last->type == TEXT_PLAIN) {
				// Trim leading whitespace
				token_trim_leading_whitespace(last, source);
				token_split_on_char(last, source, ' ');
				*remainder = last->next;
			}

			url = strndup(&source[first->start], last->start + last->len - first->start);
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


/// Extract url string from `(foo)` or `(<foo>)` or `(foo "bar")`
void extract_from_paren(token * paren, const char * source, char ** url, char ** title, char ** attributes) {
	token * t;
	size_t attr_len;

	token * remainder = paren->child->next;

	if (remainder) {
		// Skip whitespace
		whitespace_accept(&remainder);

		// Grab URL
		*url = url_accept(source, &remainder, false);

		// Skip whitespace
		whitespace_accept(&remainder);

		// Grab title, if present
		t = token_chain_accept_multiple(&remainder, 3, PAIR_QUOTE_DOUBLE, PAIR_QUOTE_SINGLE, PAIR_PAREN);

		if (t) {
			*title = text_inside_pair(source, t);
		}

		// Grab attributes, if present
		if (t) {
			attr_len = scan_attributes(&source[t->start + t->len]);
			
			if (attr_len) {
				*attributes = strndup(&source[t->start + t->len], attr_len);
			}
		}
	}
}


/// Create a link from an explicit link `[foo](bar)`
link * explicit_link(scratch_pad * scratch, token * bracket, token * paren, const char * source) {
	char * url_char =NULL;
	char * title_char = NULL;
	char * attr_char = NULL;
	link * l = NULL;

	extract_from_paren(paren, source, &url_char, &title_char, &attr_char);

	if (attr_char) {
		if (!(scratch->extensions & EXT_COMPATIBILITY))
			l = link_new(source, bracket, url_char, title_char, attr_char);
	} else {
		l = link_new(source, bracket, url_char, title_char, attr_char);		
	}

	free(url_char);
	free(title_char);
	free(attr_char);

	return l;
}


footnote * footnote_new(const char * source, token * label, token * content) {
	footnote * f = malloc(sizeof(footnote));

	if (f) {
		f->label = label;
		f->clean_text = (label == NULL) ? NULL : clean_inside_pair(source, label, true);
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
					f->content = token_new_parent(content, BLOCK_PARA);
					f->free_para = true;
					break;
			}
		}
	}

	return f;
}


void footnote_free(footnote * f) {
	if (f) {
		if (f->free_para) {
			// I'm not sure why, but the following causes a memory error.
			// Strangely, not freeing it does *not* seem to cause memory
			// leaks??

			//free(f->content);
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
		key = strndup(&source[key_start], len);
		m->key = label_from_string(key);
		free(key);
		m->value = NULL;
	}

	return m;
}


void meta_set_value(meta * m, const char * value) {
	if (value) {
		if (m->value)
			free(m->value);

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

	switch (label->type) {
		case PAIR_BRACKET:
			// Reference Link Definition

			if (!token_chain_accept(remainder, COLON))
				return false;

			// Skip space
			whitespace_accept(remainder);

			// Grab URL
			url_char = url_accept(e->dstr->str, remainder, false);

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

				if (!title)
					*remainder = temp;
			}

			title_char = text_inside_pair(e->dstr->str, title);

			// Get attributes
			if ((*remainder) && (((*remainder)->type != TEXT_NL) && ((*remainder)->type != TEXT_LINEBREAK))) {
				if (!(e->extensions & EXT_COMPATIBILITY)) {
					attr_len = scan_attributes(&source[(*remainder)->start]);
					
					if (attr_len) {
						attr_char = strndup(&source[(*remainder)->start], attr_len);

						// Skip forward
						attr_len += (*remainder)->start;

						while ((*remainder) && (*remainder)->start < attr_len)
							*remainder = (*remainder)->next;
					}
					
					l = link_new(e->dstr->str, label, url_char, title_char, attr_char);
				} else {
					// Not valid match
				}
			} else {
				l = link_new(e->dstr->str, label, url_char, title_char, attr_char);
			}

			// Store link for later use
			if (l)
				stack_push(e->link_stack, l);

			break;
		case PAIR_BRACKET_CITATION:
			if (!token_chain_accept(remainder, COLON))
				return false;

			title = *remainder;		// Track first token of content in 'title'
			f = footnote_new(e->dstr->str, label, title);

			// Store citation for later use
			stack_push(e->citation_stack, f);
			
			break;
		case PAIR_BRACKET_FOOTNOTE:
			if (!token_chain_accept(remainder, COLON))
				return false;

			title = *remainder;		// Track first token of content in 'title'
			f = footnote_new(e->dstr->str, label, title);

			// Store footnote for later use
			stack_push(e->footnote_stack, f);
			
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
	if (*remainder)
		*remainder = (*remainder)->next;				

	// Clean up
	free(url_char);
	free(title_char);
	free(attr_char);
	
	return true;
}


void process_definition_block(mmd_engine * e, token * block) {
	token * remainder = block->child;
	bool def_list = false;

//	while (remainder) {
		switch (remainder->type) {
			case PAIR_BRACKET_FOOTNOTE:
			case PAIR_BRACKET_CITATION:
			case PAIR_BRACKET_VARIABLE:
				if (!(e->extensions & EXT_NOTES))
					return;
			case PAIR_BRACKET:
				if (definition_extract(e, &remainder))
					def_list = true;
				break;
			default:
				// Rest of block is not definitions (or has already been processed)
				if (def_list) {
					tokens_prune(block->child, remainder->prev);
					block->child = remainder;
				}
				return;
		}
//	}
	
	// Ignore this block in the future
	block->type = BLOCK_EMPTY;
}


void process_definition_stack(mmd_engine * e) {
	for (int i = 0; i < e->definition_stack->size; ++i)
	{
		process_definition_block(e, stack_peek_index(e->definition_stack, i));
	}
}


void process_header_to_links(mmd_engine * e, token * h) {
	char * label = label_from_token(e->dstr->str, h);

	DString * url = d_string_new("#");

	d_string_append(url, label);

	link * l = link_new(e->dstr->str, h, url->str, NULL, NULL);

	// Store link for later use
	stack_push(e->link_stack, l);

	d_string_free(url, true);
	free(label);
}


void process_header_stack(mmd_engine * e) {
	// NTD in compatibility mode or if disabled
	if (e->extensions & EXT_NO_LABELS)
		return;

	for (int i = 0; i < e->header_stack->size; ++i)
	{
		process_header_to_links(e, stack_peek_index(e->header_stack, i));
	}
}


/// Parse metadata
void process_metadata_stack(mmd_engine * e, scratch_pad * scratch) {
	if ((scratch->extensions & EXT_NO_METADATA) ||
		(scratch->extensions & EXT_COMPATIBILITY))
		return;

	meta * m;

	for (int i = 0; i < e->metadata_stack->size; ++i)
	{
		// Check for certain metadata keys
		m = stack_peek_index(e->metadata_stack, i);

		// Certain keys do not force complete documents
		if (!(scratch->extensions & EXT_COMPLETE) &&
			!(scratch->extensions & EXT_SNIPPET)) {
			if ((strcmp(m->key, "baseheaderlevel")	!= 0) &&
				(strcmp(m->key, "xhtmlheaderlevel")	!= 0) &&
				(strcmp(m->key, "htmlheaderlevel")	!= 0) &&
				(strcmp(m->key, "latexheaderlevel")	!= 0) &&
				(strcmp(m->key, "odfheaderlevel")	!= 0) &&
				(strcmp(m->key, "xhtmlheader")		!= 0) &&
				(strcmp(m->key, "htmlheader")		!= 0) &&
				(strcmp(m->key, "quoteslanguage")	!= 0)) {
				// We found a key that is not in the list, so
				// Force a complete document
				scratch->extensions |= EXT_COMPLETE;
			}
		}
	}
}


void mmd_export_token_tree(DString * out, mmd_engine * e, short format) {

	// Process potential reference definitions
	process_definition_stack(e);

	// Process headers for potential cross-reference targets
	process_header_stack(e);

	// Create scratch pad
	scratch_pad * scratch = scratch_pad_new(e);

	// Process metadata
	process_metadata_stack(e, scratch);


	switch (format) {
		case FORMAT_HTML:
			if (scratch->extensions & EXT_COMPLETE)
				mmd_start_complete_html(out, e->dstr->str, scratch);

			mmd_export_token_tree_html(out, e->dstr->str, e->root, 0, scratch);
			mmd_export_footnote_list_html(out, e->dstr->str, scratch);
			mmd_export_citation_list_html(out, e->dstr->str, scratch);

			if (scratch->extensions & EXT_COMPLETE)
				mmd_end_complete_html(out, e->dstr->str, scratch);

			break;
	}

	scratch_pad_free(scratch);
}


void parse_brackets(const char * source, scratch_pad * scratch, token * bracket, link ** final_link, short * skip_token, bool * free_link) {
	link * temp_link = NULL;
	char * temp_char = NULL;
	short temp_short = 0;

	// What is next?
	token * next = bracket->next;

	if (next)
		temp_short = 1;

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
			// Empty label, use first bracket
			free(temp_char);
			temp_char = text_inside_pair(source, bracket);
		}
	} else {
		temp_char = text_inside_pair(source, bracket);
		// Don't skip tokens
		temp_short = 0;
	}

	temp_link = extract_link_from_stack(scratch, temp_char);

	if (temp_char)
		free(temp_char);

	if (temp_link) {
		// Don't output brackets
		bracket->child->type = TEXT_EMPTY;
		bracket->child->mate->type = TEXT_EMPTY;

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
		footnote * temp = footnote_new(source, NULL, t->child);

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
		// No match, this is an inline footnote -- create a new one
		t->child->type = TEXT_EMPTY;
		t->child->mate->type = TEXT_EMPTY;

		// Create footnote
		footnote * temp = footnote_new(source, NULL, t->child);

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

