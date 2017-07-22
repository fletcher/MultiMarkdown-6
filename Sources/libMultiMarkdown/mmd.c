/**

	MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file mmd.c

	@brief Create MMD parsing engine


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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "char.h"
#include "d_string.h"
#include "epub.h"
#include "i18n.h"
#include "lexer.h"
#include "libMultiMarkdown.h"
#include "mmd.h"
#include "object_pool.h"
#include "opendocument.h"
#include "parser.h"
#include "scanners.h"
#include "stack.h"
#include "textbundle.h"
#include "token.h"
#include "token_pairs.h"
#include "writer.h"
#include "version.h"


// Basic parser function declarations
void * ParseAlloc();
void Parse();
void ParseFree();
void ParseTrace();

void mmd_pair_tokens_in_block(token * block, token_pair_engine * e, stack * s);


/// strdup() not available on all platforms
static char * my_strdup(const char * source) {
	char * result = malloc(strlen(source) + 1);

	if (result) {
		strcpy(result, source);
	}

	return result;
}


/// Build MMD Engine
mmd_engine * mmd_engine_create(DString * d, unsigned long extensions) {
	mmd_engine * e = malloc(sizeof(mmd_engine));

	if (e) {
		e->dstr = d;

		e->root = NULL;

		e->extensions = extensions;

		e->recurse_depth = 0;

		e->allow_meta = (extensions & EXT_COMPATIBILITY) ? false : true;

		e->language = LC_EN;
		e->quotes_lang = ENGLISH;

		e->abbreviation_stack = stack_new(0);
		e->citation_stack = stack_new(0);
		e->definition_stack = stack_new(0);
		e->footnote_stack = stack_new(0);
		e->glossary_stack = stack_new(0);
		e->header_stack = stack_new(0);
		e->link_stack = stack_new(0);
		e->metadata_stack = stack_new(0);
		e->table_stack = stack_new(0);
		e->asset_hash = NULL;

		e->pairings1 = token_pair_engine_new();
		e->pairings2 = token_pair_engine_new();
		e->pairings3 = token_pair_engine_new();
		e->pairings4 = token_pair_engine_new();

		// CriticMarkup
		if (extensions & EXT_CRITIC) {
			token_pair_engine_add_pairing(e->pairings1, CRITIC_ADD_OPEN, CRITIC_ADD_CLOSE, PAIR_CRITIC_ADD, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
			token_pair_engine_add_pairing(e->pairings1, CRITIC_DEL_OPEN, CRITIC_DEL_CLOSE, PAIR_CRITIC_DEL, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
			token_pair_engine_add_pairing(e->pairings1, CRITIC_COM_OPEN, CRITIC_COM_CLOSE, PAIR_CRITIC_COM, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
			token_pair_engine_add_pairing(e->pairings1, CRITIC_SUB_OPEN, CRITIC_SUB_DIV_A, PAIR_CRITIC_SUB_DEL, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
			token_pair_engine_add_pairing(e->pairings1, CRITIC_SUB_DIV_B, CRITIC_SUB_CLOSE, PAIR_CRITIC_SUB_ADD, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
			token_pair_engine_add_pairing(e->pairings1, CRITIC_HI_OPEN, CRITIC_HI_CLOSE, PAIR_CRITIC_HI, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
		}

		// HTML Comments
		token_pair_engine_add_pairing(e->pairings2, HTML_COMMENT_START, HTML_COMMENT_STOP, PAIR_HTML_COMMENT, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);

		// Brackets, Parentheses, Angles
		token_pair_engine_add_pairing(e->pairings3, BRACKET_LEFT, BRACKET_RIGHT, PAIR_BRACKET, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);

		if (extensions & EXT_NOTES) {
			token_pair_engine_add_pairing(e->pairings3, BRACKET_CITATION_LEFT, BRACKET_RIGHT, PAIR_BRACKET_CITATION, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
			token_pair_engine_add_pairing(e->pairings3, BRACKET_FOOTNOTE_LEFT, BRACKET_RIGHT, PAIR_BRACKET_FOOTNOTE, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
			token_pair_engine_add_pairing(e->pairings3, BRACKET_GLOSSARY_LEFT, BRACKET_RIGHT, PAIR_BRACKET_GLOSSARY, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
			token_pair_engine_add_pairing(e->pairings3, BRACKET_ABBREVIATION_LEFT, BRACKET_RIGHT, PAIR_BRACKET_ABBREVIATION, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
		} else {
			token_pair_engine_add_pairing(e->pairings3, BRACKET_CITATION_LEFT, BRACKET_RIGHT, PAIR_BRACKET, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
			token_pair_engine_add_pairing(e->pairings3, BRACKET_FOOTNOTE_LEFT, BRACKET_RIGHT, PAIR_BRACKET, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
			token_pair_engine_add_pairing(e->pairings3, BRACKET_GLOSSARY_LEFT, BRACKET_RIGHT, PAIR_BRACKET, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
			token_pair_engine_add_pairing(e->pairings3, BRACKET_ABBREVIATION_LEFT, BRACKET_RIGHT, PAIR_BRACKET, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
		}

		token_pair_engine_add_pairing(e->pairings3, BRACKET_VARIABLE_LEFT, BRACKET_RIGHT, PAIR_BRACKET_VARIABLE, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);

		token_pair_engine_add_pairing(e->pairings3, BRACKET_IMAGE_LEFT, BRACKET_RIGHT, PAIR_BRACKET_IMAGE, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
		token_pair_engine_add_pairing(e->pairings3, PAREN_LEFT, PAREN_RIGHT, PAIR_PAREN, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
		token_pair_engine_add_pairing(e->pairings3, ANGLE_LEFT, ANGLE_RIGHT, PAIR_ANGLE, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
		token_pair_engine_add_pairing(e->pairings3, BRACE_DOUBLE_LEFT, BRACE_DOUBLE_RIGHT, PAIR_BRACES, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);

		// Strong/Emph
		token_pair_engine_add_pairing(e->pairings4, STAR, STAR, PAIR_STAR, 0);
		token_pair_engine_add_pairing(e->pairings4, UL, UL, PAIR_UL, 0);

		// Quotes and Backticks
		token_pair_engine_add_pairing(e->pairings3, BACKTICK, BACKTICK, PAIR_BACKTICK, PAIRING_PRUNE_MATCH | PAIRING_MATCH_LENGTH);

		token_pair_engine_add_pairing(e->pairings4, BACKTICK,   QUOTE_RIGHT_ALT,   PAIR_QUOTE_ALT, PAIRING_ALLOW_EMPTY | PAIRING_MATCH_LENGTH);
		token_pair_engine_add_pairing(e->pairings4, QUOTE_SINGLE, QUOTE_SINGLE, PAIR_QUOTE_SINGLE, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
		token_pair_engine_add_pairing(e->pairings4, QUOTE_DOUBLE, QUOTE_DOUBLE, PAIR_QUOTE_DOUBLE, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);

		// Math
		if (!(extensions & EXT_COMPATIBILITY)) {
			token_pair_engine_add_pairing(e->pairings3, MATH_PAREN_OPEN, MATH_PAREN_CLOSE, PAIR_MATH, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
			token_pair_engine_add_pairing(e->pairings3, MATH_BRACKET_OPEN, MATH_BRACKET_CLOSE, PAIR_MATH, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
			token_pair_engine_add_pairing(e->pairings3, MATH_DOLLAR_SINGLE, MATH_DOLLAR_SINGLE, PAIR_MATH, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
			token_pair_engine_add_pairing(e->pairings3, MATH_DOLLAR_DOUBLE, MATH_DOLLAR_DOUBLE, PAIR_MATH, PAIRING_ALLOW_EMPTY | PAIRING_PRUNE_MATCH);
		}

		// Superscript/Subscript
		if (!(extensions & EXT_COMPATIBILITY)) {
			token_pair_engine_add_pairing(e->pairings4, SUPERSCRIPT, SUPERSCRIPT, PAIR_SUPERSCRIPT, PAIRING_PRUNE_MATCH);
			token_pair_engine_add_pairing(e->pairings4, SUBSCRIPT, SUBSCRIPT, PAIR_SUBSCRIPT, PAIRING_PRUNE_MATCH);
		}

		// Text Braces -- for raw text syntax
		if (!(extensions & EXT_COMPATIBILITY)) {
			token_pair_engine_add_pairing(e->pairings4, TEXT_BRACE_LEFT, TEXT_BRACE_RIGHT, PAIR_BRACE, PAIRING_PRUNE_MATCH);
			token_pair_engine_add_pairing(e->pairings4, RAW_FILTER_LEFT, TEXT_BRACE_RIGHT, PAIR_RAW_FILTER, PAIRING_PRUNE_MATCH);
		}
	}

	return e;
}

/// Create MMD Engine using an existing DString (A new copy is *not* made)
mmd_engine * mmd_engine_create_with_dstring(DString * d, unsigned long extensions) {
	return mmd_engine_create(d, extensions);
}


/// Create MMD Engine using a C string (A private copy of the string will be
/// made.  The one passed here can be freed by the calling function)
mmd_engine * mmd_engine_create_with_string(const char * str, unsigned long extensions) {
	DString * d = d_string_new(str);

	return mmd_engine_create(d, extensions);
}


/// Set language and smart quotes language
void mmd_engine_set_language(mmd_engine * e, short language) {
	if (!e) {
		return;
	}

	e->language = language;

	switch (language) {
		case LC_DE:
			e->quotes_lang = GERMAN;
			break;

		case LC_EN:
			e->quotes_lang = ENGLISH;
			break;

		case LC_ES:
			e->quotes_lang = ENGLISH;
			break;

		case LC_FR:
			e->quotes_lang = FRENCH;
			break;

		case LC_NL:
			e->quotes_lang = DUTCH;
			break;

		case LC_SV:
			e->quotes_lang = SWEDISH;
			break;

		default:
			e->quotes_lang = ENGLISH;
	}
}


void mmd_engine_reset(mmd_engine * e) {
	if (e->root) {
		token_tree_free(e->root);
		e->root = NULL;
	}

	// Abbreviations need to be freed
	while (e->abbreviation_stack->size) {
		footnote_free(stack_pop(e->abbreviation_stack));
	}

	// Citations need to be freed
	while (e->citation_stack->size) {
		footnote_free(stack_pop(e->citation_stack));
	}

	// Footnotes need to be freed
	while (e->footnote_stack->size) {
		footnote_free(stack_pop(e->footnote_stack));
	}

	// Glossaries need to be freed
	while (e->glossary_stack->size) {
		footnote_free(stack_pop(e->glossary_stack));
	}

	// Links need to be freed
	while (e->link_stack->size) {
		link_free(stack_pop(e->link_stack));
	}

	// Metadata needs to be freed
	while (e->metadata_stack->size) {
		meta_free(stack_pop(e->metadata_stack));
	}

	// Free asset hash
	asset * a, * a_tmp;
	HASH_ITER(hh, e->asset_hash, a, a_tmp) {
		HASH_DEL(e->asset_hash, a);	// Remove item from hash
		asset_free(a);				// Free the asset
	}

	// Reset other stacks
	e->definition_stack->size = 0;
	e->header_stack->size = 0;
	e->table_stack->size = 0;
}


/// Free an existing MMD Engine
void mmd_engine_free(mmd_engine * e, bool freeDString) {
	if (e == NULL) {
		return;
	}

	mmd_engine_reset(e);

	if (freeDString) {
		d_string_free(e->dstr, true);
	}

	token_pair_engine_free(e->pairings1);
	token_pair_engine_free(e->pairings2);
	token_pair_engine_free(e->pairings3);
	token_pair_engine_free(e->pairings4);

	// Pointers to blocks that are freed elsewhere
	stack_free(e->definition_stack);
	stack_free(e->header_stack);
	stack_free(e->table_stack);

	// Takedown
	stack_free(e->abbreviation_stack);
	stack_free(e->citation_stack);
	stack_free(e->footnote_stack);
	stack_free(e->glossary_stack);
	stack_free(e->link_stack);
	stack_free(e->metadata_stack);

	free(e);
}


bool line_is_empty(token * t) {
	while (t) {
		switch (t->type) {
			case NON_INDENT_SPACE:
			case INDENT_TAB:
			case INDENT_SPACE:
				t = t->next;
				break;

			case TEXT_LINEBREAK:
			case TEXT_NL:
				return true;

			default:
				return false;
		}
	}

	return true;
}


/// Determine what sort of line this is
void mmd_assign_line_type(mmd_engine * e, token * line) {
	if (!line) {
		return;
	}

	if (!line->child) {
		line->type = LINE_EMPTY;
		return;
	}

	const char * source = e->dstr->str;

	token * t = NULL;
	short temp_short;
	size_t scan_len;

	// Skip non-indenting space
	if (line->child->type == NON_INDENT_SPACE) {
		token_remove_first_child(line);
	} else if (line->child->type == TEXT_PLAIN && line->child->len == 1) {
		if (source[line->child->start] == ' ') {
			token_remove_first_child(line);
		}
	}

	if (line->child == NULL) {
		line->type = LINE_EMPTY;
		return;
	}

	switch (line->child->type) {
		case INDENT_TAB:
			if (line_is_empty(line->child)) {
				line->type = LINE_EMPTY;
				e->allow_meta = false;
			} else {
				line->type = LINE_INDENTED_TAB;
			}

			break;

		case INDENT_SPACE:
			if (line_is_empty(line->child)) {
				line->type = LINE_EMPTY;
				e->allow_meta = false;
			} else {
				line->type = LINE_INDENTED_SPACE;
			}

			break;

		case ANGLE_LEFT:
			if (scan_html_block(&source[line->start])) {
				line->type = LINE_HTML;
			} else {
				line->type = LINE_PLAIN;
			}

			break;

		case ANGLE_RIGHT:
			line->type = LINE_BLOCKQUOTE;
			line->child->type = MARKER_BLOCKQUOTE;
			break;

		case BACKTICK:
			if (e->extensions & EXT_COMPATIBILITY) {
				line->type = LINE_PLAIN;
				break;
			}

			scan_len = scan_fence_end(&source[line->child->start]);

			if (scan_len) {
				switch (line->child->len) {
					case 3:
						line->type = LINE_FENCE_BACKTICK_3;
						break;

					case 4:
						line->type = LINE_FENCE_BACKTICK_4;
						break;

					default:
						line->type = LINE_FENCE_BACKTICK_5;
						break;
				}

				break;
			} else {
				scan_len = scan_fence_start(&source[line->child->start]);

				if (scan_len) {
					switch (line->child->len) {
						case 3:
							line->type = LINE_FENCE_BACKTICK_START_3;
							break;

						case 4:
							line->type = LINE_FENCE_BACKTICK_START_4;
							break;

						default:
							line->type = LINE_FENCE_BACKTICK_START_5;
							break;
					}

					break;
				}
			}

			line->type = LINE_PLAIN;
			break;

		case COLON:
			line->type = LINE_PLAIN;

			if (e->extensions & EXT_COMPATIBILITY) {
				break;
			}

			if (scan_definition(&source[line->child->start])) {
				line->type = LINE_DEFINITION;
			}

			break;

		case HASH1:
		case HASH2:
		case HASH3:
		case HASH4:
		case HASH5:
		case HASH6:
			if (scan_atx(&source[line->child->start])) {
				line->type = (line->child->type - HASH1) + LINE_ATX_1;
				line->child->type = (line->type - LINE_ATX_1) + MARKER_H1;

				// Strip trailing whitespace from '#' sequence
				line->child->len = line->child->type - MARKER_H1 + 1;

				// Strip trailing '#' sequence if present
				if (line->child->tail->type == TEXT_NL) {
					if ((line->child->tail->prev->type >= HASH1) &&
					        (line->child->tail->prev->type <= HASH6)) {
						line->child->tail->prev->type -= HASH1;
						line->child->tail->prev->type += MARKER_H1;
					}
				} else {
					if ((line->child->tail->type >= HASH1) &&
					        (line->child->tail->type <= HASH6)) {
						line->child->tail->type -= TEXT_EMPTY;
						line->child->tail->type += MARKER_H1;
					}
				}
			} else {
				line->type = LINE_PLAIN;
			}

			break;

		case HTML_COMMENT_START:
			if (!line->child->next || !line->child->next->next) {
				line->type = LINE_START_COMMENT;
			} else {
				line->type = LINE_PLAIN;
			}

			break;

		case HTML_COMMENT_STOP:
			if (!line->child->next || !line->child->next->next) {
				line->type = LINE_STOP_COMMENT;
			} else {
				line->type = LINE_PLAIN;
			}

			break;

		case TEXT_NUMBER_POSS_LIST:
			switch (source[line->child->next->start]) {
				case ' ':
				case '\t':
					line->type = LINE_LIST_ENUMERATED;
					line->child->type = MARKER_LIST_ENUMERATOR;

					switch (line->child->next->type) {
						case TEXT_PLAIN:

							// Strip whitespace between bullet and text
							while (char_is_whitespace(source[line->child->next->start])) {
								line->child->next->start++;
								line->child->next->len--;
							}

							break;

						case INDENT_SPACE:
						case INDENT_TAB:
						case NON_INDENT_SPACE:
							t = line->child;

							while (t->next && ((t->next->type == INDENT_SPACE) ||
							                   (t->next->type == INDENT_TAB) ||
							                   (t->next->type == NON_INDENT_SPACE))) {
								tokens_prune(t->next, t->next);
							}

							break;
					}

					break;

				default:
					line->type = LINE_PLAIN;
					line->child->type = TEXT_PLAIN;
					break;
			}

			break;

		case EQUAL:

			// Could this be a setext heading marker?
			if (scan_setext(&source[line->child->start])) {
				line->type = LINE_SETEXT_1;
			} else {
				line->type = LINE_PLAIN;
			}

			break;

		case DASH_N:
		case DASH_M:
			if (scan_setext(&source[line->child->start])) {
				line->type = LINE_SETEXT_2;
				break;
			}

		case STAR:
		case UL:
			// Could this be a horizontal rule?
			t = line->child->next;
			temp_short = line->child->len;

			while (t) {
				switch (t->type) {
					case DASH_N:
					case DASH_M:
						if (t->type == line->child->type) {
							t = t->next;

							if (t) {
								temp_short += t->len;
							}
						} else {
							temp_short = 0;
							t = NULL;
						}

						break;

					case STAR:
					case UL:
						if (t->type == line->child->type) {
							t = t->next;
							temp_short++;
						} else {
							temp_short = 0;
							t = NULL;
						}

						break;

					case NON_INDENT_SPACE:
					case INDENT_TAB:
					case INDENT_SPACE:
						t = t->next;
						break;

					case TEXT_PLAIN:
						if ((t->len == 1) && (source[t->start] == ' ')) {
							t = t->next;
							break;
						}

						temp_short = 0;
						t = NULL;
						break;

					case TEXT_NL:
					case TEXT_LINEBREAK:
						t = NULL;
						break;

					default:
						temp_short = 0;
						t = NULL;
						break;
				}
			}

			if (temp_short > 2) {
				// This is a horizontal rule, not a list item
				line->type = LINE_HR;
				break;
			}

			if (line->child->type == UL) {
				// Revert to plain for this type
				line->type = LINE_PLAIN;
				break;
			}

			// If longer than 1 character, then it can't be a list marker, so it's a
			// plain line
			if (line->child->len > 1) {
				line->type = LINE_PLAIN;
				break;
			}

		case PLUS:
			if (!line->child->next) {
				// TODO: Should this be an empty list item instead??
				line->type = LINE_PLAIN;
			} else {
				switch (source[line->child->next->start]) {
					case ' ':
					case '\t':
						line->type = LINE_LIST_BULLETED;
						line->child->type = MARKER_LIST_BULLET;

						switch (line->child->next->type) {
							case TEXT_PLAIN:

								// Strip whitespace between bullet and text
								while (char_is_whitespace(source[line->child->next->start])) {
									line->child->next->start++;
									line->child->next->len--;
								}

								break;

							case INDENT_SPACE:
							case INDENT_TAB:
							case NON_INDENT_SPACE:
								t = line->child;

								while (t->next && ((t->next->type == INDENT_SPACE) ||
								                   (t->next->type == INDENT_TAB) ||
								                   (t->next->type == NON_INDENT_SPACE))) {
									tokens_prune(t->next, t->next);
								}

								break;
						}

						break;

					default:
						line->type = LINE_PLAIN;
						break;
				}
			}

			break;

		case TEXT_LINEBREAK:
		case TEXT_NL:
			e->allow_meta = false;
			line->type = LINE_EMPTY;
			break;

		case TOC:
			line->type = (e->extensions & EXT_COMPATIBILITY) ? LINE_PLAIN : LINE_TOC;
			break;

		case BRACKET_LEFT:
			if (e->extensions & EXT_COMPATIBILITY) {
				scan_len = scan_ref_link_no_attributes(&source[line->start]);
				line->type = (scan_len) ? LINE_DEF_LINK : LINE_PLAIN;
			} else {
				scan_len = scan_ref_link(&source[line->start]);
				line->type = (scan_len) ? LINE_DEF_LINK : LINE_PLAIN;
			}

			break;

		case BRACKET_ABBREVIATION_LEFT:
			if (e->extensions & EXT_NOTES) {
				scan_len = scan_ref_abbreviation(&source[line->start]);
				line->type = (scan_len) ? LINE_DEF_ABBREVIATION : LINE_PLAIN;
			} else {
				scan_len = scan_ref_link_no_attributes(&source[line->start]);
				line->type = (scan_len) ? LINE_DEF_LINK : LINE_PLAIN;
			}

			break;

		case BRACKET_CITATION_LEFT:
			if (e->extensions & EXT_NOTES) {
				scan_len = scan_ref_citation(&source[line->start]);
				line->type = (scan_len) ? LINE_DEF_CITATION : LINE_PLAIN;
			} else {
				scan_len = scan_ref_link_no_attributes(&source[line->start]);
				line->type = (scan_len) ? LINE_DEF_LINK : LINE_PLAIN;
			}

			break;

		case BRACKET_FOOTNOTE_LEFT:
			if (e->extensions & EXT_NOTES) {
				scan_len = scan_ref_foot(&source[line->start]);
				line->type = (scan_len) ? LINE_DEF_FOOTNOTE : LINE_PLAIN;
			} else {
				scan_len = scan_ref_link_no_attributes(&source[line->start]);
				line->type = (scan_len) ? LINE_DEF_LINK : LINE_PLAIN;
			}

			break;

		case BRACKET_GLOSSARY_LEFT:
			if (e->extensions & EXT_NOTES) {
				scan_len = scan_ref_glossary(&source[line->start]);
				line->type = (scan_len) ? LINE_DEF_GLOSSARY : LINE_PLAIN;
			} else {
				scan_len = scan_ref_link_no_attributes(&source[line->start]);
				line->type = (scan_len) ? LINE_DEF_LINK : LINE_PLAIN;
			}

			break;

		case PIPE:

			// If PIPE is first, save checking later and assign LINE_TABLE now
			if (!(e->extensions & EXT_COMPATIBILITY)) {
				scan_len = scan_table_separator(&source[line->start]);
				line->type = (scan_len) ? LINE_TABLE_SEPARATOR : LINE_TABLE;

				break;
			}

		case TEXT_PLAIN:
			if (e->allow_meta && !(e->extensions & EXT_COMPATIBILITY)) {
				scan_len = scan_url(&source[line->start]);

				if (scan_len == 0) {
					scan_len = scan_meta_line(&source[line->start]);
					line->type = (scan_len) ? LINE_META : LINE_PLAIN;
					break;
				}
			}

		default:
			line->type = LINE_PLAIN;
			break;
	}

	if ((line->type == LINE_PLAIN) &&
	        !(e->extensions & EXT_COMPATIBILITY)) {
		// Check if this is a potential table line
		token * walker = line->child;

		while (walker != NULL) {
			if (walker->type == PIPE) {
				scan_len = scan_table_separator(&source[line->start]);
				line->type = (scan_len) ? LINE_TABLE_SEPARATOR : LINE_TABLE;

				return;
			}

			walker = walker->next;
		}
	}
}


/// Strip leading indenting space from line (if present)
void deindent_line(token  * line) {
	if (!line || !line->child) {
		return;
	}

	token * t;

	switch (line->child->type) {
		case INDENT_TAB:
		case INDENT_SPACE:
			t = line->child;
			line->child = t->next;
			t->next = NULL;

			if (line->child) {
				line->child->prev = NULL;
				line->child->tail = t->tail;
			}

			token_free(t);
			break;
	}
}


/// Strip leading indenting space from block
/// (for recursively parsing nested lists)
void deindent_block(mmd_engine * e, token * block) {
	if (!block || !block->child) {
		return;
	}

	token * t = block->child;

	while (t != NULL) {
		deindent_line(t);
		mmd_assign_line_type(e, t);

		t = t->next;
	}
}


/// Strip leading blockquote marker from line
void strip_quote_markers_from_line(token * line, const char * source) {
	if (!line || !line->child) {
		return;
	}

	token * t;

	switch (line->child->type) {
		case MARKER_BLOCKQUOTE:
		case NON_INDENT_SPACE:
			t = line->child;
			line->child = t->next;
			t->next = NULL;

			if (line->child) {
				line->child->prev = NULL;
				line->child->tail = t->tail;
			}

			token_free(t);
			break;
	}

	if (line->child && (line->child->type == TEXT_PLAIN)) {
		// Strip leading whitespace from first text token
		t = line->child;

		while (t->len && char_is_whitespace(source[t->start])) {
			t->start++;
			t->len--;
		}

		if (t->len == 0) {
			line->child = t->next;
			t->next = NULL;

			if (line->child) {
				line->child->prev = NULL;
				line->child->tail = t->tail;
			}

			token_free(t);
		}
	}
}


/// Strip leading blockquote markers and non-indent space
/// (for recursively parsing blockquotes)
void strip_quote_markers_from_block(mmd_engine * e, token * block) {
	if (!block || !block->child) {
		return;
	}

	token * t = block->child;

	while (t != NULL) {
		strip_quote_markers_from_line(t, e->dstr->str);
		mmd_assign_line_type(e, t);

		t = t->next;
	}
}


/// Create a token chain from source string
/// stop_on_empty_line allows us to stop parsing part of the way through
token * mmd_tokenize_string(mmd_engine * e, size_t start, size_t len, bool stop_on_empty_line) {
	// Reset metadata flag
	e->allow_meta = (e->extensions & EXT_COMPATIBILITY) ? false : true;


	// Create a scanner (for re2c)
	Scanner s;
	s.start = &e->dstr->str[start];
	s.cur = s.start;

	// Strip trailing whitespace
//	while (len && char_is_whitespace_or_line_ending(str[len - 1]))
//		len--;

	// Where do we stop parsing?
	const char * stop = &e->dstr->str[start] + len;

	int type;								// TOKEN type
	token * t;								// Create tokens for incorporation

	token * root = token_new(0, start, 0);		// Store the final parse tree here
	token * line = token_new(0, start, 0);		// Store current line here

	const char * last_stop = &e->dstr->str[start];	// Remember where last token ended

	do {
		// Scan for next token (type of 0 means there is nothing left);
		type = scan(&s, stop);

		//if (type && s.start != last_stop) {
		if (s.start != last_stop) {
			// We skipped characters between tokens

			if (type) {
				// Create a default token type for the skipped characters
				t = token_new(TEXT_PLAIN, (size_t)(last_stop - e->dstr->str), (size_t)(s.start - last_stop));

				token_append_child(line, t);
			} else {
				if (stop > last_stop) {
					// Source text ends without newline
					t = token_new(TEXT_PLAIN, (size_t)(last_stop - e->dstr->str), (size_t)(stop - last_stop));

					token_append_child(line, t);
				}
			}
		} else if (type == 0 && stop > last_stop) {
			// Source text ends without newline
			t = token_new(TEXT_PLAIN, (size_t)(last_stop - e->dstr->str), (size_t)(stop - last_stop));
			token_append_child(line, t);
		}


		switch (type) {
			case 0:
				// 0 means we finished with input
				// Add current line to root

				// What sort of line is this?
				mmd_assign_line_type(e, line);

				token_append_child(root, line);
				break;

			case TEXT_LINEBREAK:
			case TEXT_NL:
				// We hit the end of a line
				t = token_new(type, (size_t)(s.start - e->dstr->str), (size_t)(s.cur - s.start));
				token_append_child(line, t);

				// What sort of line is this?
				mmd_assign_line_type(e, line);

				token_append_child(root, line);

				// If this is first line, do we have proper metadata?
				if (e->allow_meta && root->child == line) {
					if (line->type == LINE_SETEXT_2) {
						line->type = LINE_YAML;
					} else if (line->type != LINE_META) {
						e->allow_meta = false;
					}
				}

				if (stop_on_empty_line) {
					if (line->type == LINE_EMPTY) {
						return root;
					}
				}

				line = token_new(0, s.cur - e->dstr->str, 0);
				break;

			default:
				t = token_new(type, (size_t)(s.start - e->dstr->str), (size_t)(s.cur - s.start));
				token_append_child(line, t);
				break;
		}

		// Remember where token ends to detect skipped characters
		last_stop = s.cur;
	} while (type != 0);


	return root;
}


/// Parse token tree
void mmd_parse_token_chain(mmd_engine * e, token * chain) {

	if (e->recurse_depth == kMaxParseRecursiveDepth) {
		return;
	}

	e->recurse_depth++;

	void* pParser = ParseAlloc (malloc);		// Create a parser (for lemon)
	token * walker = chain->child;				// Walk the existing tree
	token * remainder;							// Hold unparsed tail of chain

	#ifndef NDEBUG
	ParseTrace(stderr, "parser >>");
	#endif

	// Remove existing token tree
	e->root = NULL;

	while (walker != NULL) {
		remainder = walker->next;

		// Snip token from remainder
		walker->next = NULL;
		walker->tail = walker;

		if (remainder) {
			remainder->prev = NULL;
		}

		#ifndef NDEBUG
		fprintf(stderr, "\nNew line\n");
		#endif

		Parse(pParser, walker->type, walker, e);

		walker = remainder;
	}

	// Signal finish to parser
	#ifndef NDEBUG
	fprintf(stderr, "\nFinish parse\n");
	#endif
	Parse(pParser, 0, NULL, e);

	// Disconnect of (now empty) root
	chain->child = NULL;
	token_append_child(chain, e->root);
	e->root = NULL;

	ParseFree(pParser, free);

	e->recurse_depth--;
}


void mmd_pair_tokens_in_chain(token * head, token_pair_engine * e, stack * s) {

	while (head != NULL) {
		mmd_pair_tokens_in_block(head, e, s);

		head = head->next;
	}
}


/// Match token pairs inside block
void mmd_pair_tokens_in_block(token * block, token_pair_engine * e, stack * s) {
	if (block == NULL || e == NULL) {
		return;
	}

	switch (block->type) {
		case BLOCK_BLOCKQUOTE:
		case BLOCK_DEFLIST:
		case BLOCK_DEFINITION:
		case BLOCK_DEF_ABBREVIATION:
		case BLOCK_DEF_CITATION:
		case BLOCK_DEF_FOOTNOTE:
		case BLOCK_DEF_GLOSSARY:
		case BLOCK_DEF_LINK:
		case BLOCK_H1:
		case BLOCK_H2:
		case BLOCK_H3:
		case BLOCK_H4:
		case BLOCK_H5:
		case BLOCK_H6:
		case BLOCK_PARA:
		case BLOCK_SETEXT_1:
		case BLOCK_SETEXT_2:
		case BLOCK_TERM:
			token_pairs_match_pairs_inside_token(block, e, s, 0);
			break;

		case DOC_START_TOKEN:
		case BLOCK_LIST_BULLETED:
		case BLOCK_LIST_BULLETED_LOOSE:
		case BLOCK_LIST_ENUMERATED:
		case BLOCK_LIST_ENUMERATED_LOOSE:
			mmd_pair_tokens_in_chain(block->child, e, s);
			break;

		case BLOCK_LIST_ITEM:
		case BLOCK_LIST_ITEM_TIGHT:
			token_pairs_match_pairs_inside_token(block, e, s, 0);
			mmd_pair_tokens_in_chain(block->child, e, s);
			break;

		case LINE_TABLE:
		case BLOCK_TABLE:
			// TODO: Need to parse into cells first
			token_pairs_match_pairs_inside_token(block, e, s, 0);
			mmd_pair_tokens_in_chain(block->child, e, s);
			break;

		case BLOCK_EMPTY:
		case BLOCK_CODE_INDENTED:
		case BLOCK_CODE_FENCED:
		default:
			// Nothing to do here
			return;
	}
}


/// Ambidextrous tokens can open OR close a pair.  This routine gives the opportunity
/// to change this behavior on case-by-case basis.  For example, in `foo **bar** foo`, the
/// first set of asterisks can open, but not close a pair.  The second set can close, but not
/// open a pair.  This allows for complex behavior without having to bog down the tokenizer
/// with figuring out which type of asterisk we have.  Default behavior is that open and close
/// are enabled, so we just have to figure out when to turn it off.
void mmd_assign_ambidextrous_tokens_in_block(mmd_engine * e, token * block, size_t start_offset) {
	if (block == NULL || block->child == NULL) {
		return;
	}

	size_t offset;		// Temp variable for use below
	size_t lead_count, lag_count, pre_count, post_count;

	token * t = block->child;

	char * str = e->dstr->str;

	while (t != NULL) {
		switch (t->type) {
			case BLOCK_META:

				// Do we treat this like metadata?
				if (!(e->extensions & EXT_COMPATIBILITY) &&
				        !(e->extensions & EXT_NO_METADATA)) {
					break;
				}

				// This is not metadata
				t->type = BLOCK_PARA;

			case DOC_START_TOKEN:
			case BLOCK_BLOCKQUOTE:
			case BLOCK_DEF_ABBREVIATION:
			case BLOCK_DEFLIST:
			case BLOCK_DEFINITION:
			case BLOCK_H1:
			case BLOCK_H2:
			case BLOCK_H3:
			case BLOCK_H4:
			case BLOCK_H5:
			case BLOCK_H6:
			case BLOCK_LIST_BULLETED:
			case BLOCK_LIST_BULLETED_LOOSE:
			case BLOCK_LIST_ENUMERATED:
			case BLOCK_LIST_ENUMERATED_LOOSE:
			case BLOCK_LIST_ITEM:
			case BLOCK_LIST_ITEM_TIGHT:
			case BLOCK_PARA:
			case BLOCK_SETEXT_1:
			case BLOCK_SETEXT_2:
			case BLOCK_TABLE:
			case BLOCK_TERM:
			case LINE_LIST_BULLETED:
			case LINE_LIST_ENUMERATED:
				// Assign child tokens of blocks
				mmd_assign_ambidextrous_tokens_in_block(e, t, start_offset);
				break;

			case CRITIC_SUB_DIV:
				// Divide this into two tokens
				t->child = token_new(CRITIC_SUB_DIV_B, t->start + 1, 1);
				t->child->next = t->next;
				t->next = t->child;
				t->child = NULL;
				t->len = 1;
				t->type = CRITIC_SUB_DIV_A;
				break;

			case STAR:
				// Look left and skip over neighboring '*' characters
				offset = t->start;

				while ((offset != 0) && ((str[offset] == '*') || (str[offset] == '_'))) {
					offset--;
				}

				// We can only close if there is something to left besides whitespace
				if ((offset == 0) || (char_is_whitespace_or_line_ending(str[offset]))) {
					// Whitespace or punctuation to left, so can't close
					t->can_close = 0;
				}

				// Look right and skip over neighboring '*' characters
				offset = t->start + 1;

				while ((str[offset] == '*') || (str[offset] == '_')) {
					offset++;
				}

				// We can only open if there is something to right besides whitespace/punctuation
				if (char_is_whitespace_or_line_ending(str[offset])) {
					// Whitespace to right, so can't open
					t->can_open = 0;
				}

				// If we're in the middle of a word, then we need to be more precise
				if (t->can_open && t->can_close) {
					lead_count = 0;			//!< '*' in run before current
					lag_count = 0;			//!< '*' in run after current
					pre_count = 0;			//!< '*' before word
					post_count = 0;			//!< '*' after word

					offset = t->start - 1;

					// How many '*' in this run before current token?
					while (offset && (str[offset] == '*')) {
						lead_count++;
						offset--;
					}

					// Skip over letters/numbers
					// TODO: Need to fix this to actually get run at beginning of word, not in middle,
					// e.g. **foo*bar*foo*bar**
					while (offset && (!char_is_whitespace_or_line_ending_or_punctuation(str[offset]))) {
						offset--;
					}

					// Are there '*' at the beginning of this word?
					while ((offset != -1) && (str[offset] == '*')) {
						pre_count++;
						offset--;
					}

					offset = t->start + 1;

					// How many '*' in this run after current token?
					while (str[offset] == '*') {
						lag_count++;
						offset++;
					}

					// Skip over letters/numbers
					// TODO: Same as above
					while (!char_is_whitespace_or_line_ending_or_punctuation(str[offset])) {
						offset++;
					}

					// Are there '*' at the end of this word?
					while (offset && (str[offset] == '*')) {
						post_count++;
						offset++;
					}

					// Are there '*' before/after word?
					if (pre_count + post_count > 0) {
						if (pre_count + post_count == lead_count + lag_count + 1) {
							// Same number outside as in the current run
							// **foo****bar**
							if (pre_count == post_count) {
								// **foo****bar**
								// We want to wrap the word, since
								// <strong>foo</strong><strong>bar</strong> doesn't make sense
								t->can_open = 0;
								t->can_close = 0;
							} else if (pre_count == 0) {
								// foo**bar**
								// Open only so we don't close outside the word
								t->can_close = 0;
							} else if (post_count == 0) {
								// **foo**bar
								// Close only so we don't close outside the word
								t->can_open = 0;
							}
						} else if (pre_count == lead_count + lag_count + 1 + post_count) {
							// ***foo**bar*
							// We want to close what's open
							t->can_open = 0;
						} else if (post_count == pre_count + lead_count + lag_count + 1) {
							// *foo**bar***
							// We want to open a set to close at the end
							t->can_close = 0;
						} else {
							if (pre_count != lead_count + lag_count + 1) {
								// **foo**bar -> close, otherwise don't
								t->can_close = 0;
							}

							if (post_count != lead_count + lag_count + 1) {
								// foo**bar** -> open, otherwise don't
								t->can_open = 0;
							}
						}
					}
				}

				break;

			case UL:
				// Look left and skip over neighboring '_' characters
				offset = t->start;

				while ((offset != 0) && ((str[offset] == '_') || (str[offset] == '*'))) {
					offset--;
				}

				if ((offset == 0) || (char_is_whitespace_or_line_ending(str[offset]))) {
					// Whitespace to left, so can't close
					t->can_close = 0;
				}

				// We don't allow intraword underscores (e.g.  `foo_bar_foo`)
				if ((offset > 0) && (char_is_alphanumeric(str[offset]))) {
					// Letters to left, so can't open
					t->can_open = 0;
				}

				// Look right and skip over neighboring '_' characters
				offset = t->start + 1;

				while ((str[offset] == '*') || (str[offset] == '_')) {
					offset++;
				}

				if (char_is_whitespace_or_line_ending(str[offset])) {
					// Whitespace to right, so can't open
					t->can_open = 0;
				}

				if (char_is_alphanumeric(str[offset])) {
					// Letters to right, so can't close
					t->can_close = 0;
				}

				break;

			case BACKTICK:
				// Backticks are used for code spans, but also for ``foo'' double quote syntax.
				// We care only about the quote syntax.
				offset = t->start;

				// TODO: This does potentially prevent ``foo `` from closing due to space before closer?
				// Bug or feature??
				if (t->len != 2) {
					break;
				}

				if ((offset == 0) || (str[offset] != '`' && char_is_whitespace_or_line_ending_or_punctuation(str[offset - 1]))) {
					// Whitespace or punctuation to left, so can't close
					t->can_close = 0;
				}

				break;

			case QUOTE_SINGLE:
				// Some of these are actually APOSTROPHE's and should not be paired
				offset = t->start;

				if (!((offset == 0) ||
				        (char_is_whitespace_or_line_ending_or_punctuation(str[offset - 1])) ||
				        (char_is_whitespace_or_line_ending_or_punctuation(str[offset + 1])))) {
					t->type = APOSTROPHE;
					break;
				}

				if (offset && (char_is_punctuation(str[offset - 1])) &&
				        (char_is_alphanumeric(str[offset + 1]))) {
					// If possessive apostrophe, e.g. `x`'s
					if (str[offset + 1] == 's' || str[offset + 1] == 'S') {
						if (char_is_whitespace_or_line_ending_or_punctuation(str[offset + 2])) {
							t->type = APOSTROPHE;
							break;
						}
					}
				}

			case QUOTE_DOUBLE:
				offset = t->start;

				if ((offset == 0) || (char_is_whitespace_or_line_ending(str[offset - 1]))) {
					t->can_close = 0;
				}

				if (char_is_whitespace_or_line_ending(str[offset + 1])) {
					t->can_open = 0;
				}

				break;

			case DASH_N:
				if (!(e->extensions & EXT_SMART)) {
					break;
				}

				// We want `1-2` to trigger a DASH_N, but regular hyphen otherwise (`a-b`)
				// This doesn't apply to `--` or `---`
				offset = t->start;

				if (t->len == 1) {
					// Check whether we have '1-2'
					if ((offset == 0) || (!char_is_digit(str[offset - 1])) ||
					        (!char_is_digit(str[offset + 1]))) {
						t->type = TEXT_PLAIN;
					}
				}

				break;

			case MATH_DOLLAR_SINGLE:
			case MATH_DOLLAR_DOUBLE:
				if (e->extensions & EXT_COMPATIBILITY) {
					break;
				}

				offset = t->start;

				// Look left
				if ((offset == 0) || (char_is_whitespace_or_line_ending(str[offset - 1]))) {
					// Whitespace to left, so can't close
					t->can_close = 0;
				} else if ((offset != 0) && (!char_is_whitespace_or_line_ending_or_punctuation(str[offset - 1]))) {
					// No whitespace or punctuation to left, can't open
					t->can_open = 0;
				}

				// Look right
				offset = t->start + t->len;

				if (char_is_whitespace_or_line_ending(str[offset])) {
					// Whitespace to right, so can't open
					t->can_open = 0;
				} else if (!char_is_whitespace_or_line_ending_or_punctuation(str[offset])) {
					// No whitespace or punctuation to right, can't close
					t->can_close = 0;
				}

				break;

			case SUPERSCRIPT:
			case SUBSCRIPT:
				if (e->extensions & EXT_COMPATIBILITY) {
					t->type = TEXT_PLAIN;
					break;
				}

				offset = t->start;

				// Look left -- no whitespace to left
				if ((offset == 0) || (char_is_whitespace_or_line_ending_or_punctuation(str[offset - 1]))) {
					t->can_open = 0;
				}

				if ((offset != 0) && (char_is_whitespace_or_line_ending(str[offset - 1]))) {
					t->can_close = 0;
				}

				offset = t->start + t->len;

				if (char_is_whitespace_or_line_ending_or_punctuation(str[offset])) {
					t->can_open = 0;
				}

				// We need to be contiguous in order to match
				if (t->can_close) {
					offset = t->start;
					t->can_close = 0;

					while ((offset > 0) && !(char_is_whitespace_or_line_ending(str[offset - 1]))) {
						if (str[offset - 1] == str[t->start]) {
							t->can_close = 1;
							break;
						}

						offset--;
					}
				}

				// We need to be contiguous in order to match
				if (t->can_open) {
					offset = t->start + t->len;
					t->can_open = 0;

					while (!(char_is_whitespace_or_line_ending(str[offset]))) {
						if (str[offset] == str[t->start]) {
							t->can_open = 1;
							break;
						}

						offset++;
					}

					// Are we a standalone, e.g x^2
					if (!t->can_close && !t->can_open) {
						offset = t->start + t->len;

						while (!char_is_whitespace_or_line_ending_or_punctuation(str[offset])) {
							offset++;
						}

						t->len = offset - t->start;
						t->can_close = 0;

						// Shift next token right and move those characters as child node
						// It's possible that one (or more?) tokens are entirely subsumed.
						while (t->next && t->next->start + t->next->len < offset) {
							tokens_prune(t->next, t->next);
						}

						if ((t->next != NULL) && ((t->next->type == TEXT_PLAIN) || (t->next->type == TEXT_NUMBER_POSS_LIST))) {
							t->next->len = t->next->start + t->next->len - offset;
							t->next->start = offset;
						}

						t->child = token_new(TEXT_PLAIN, t->start + 1, t->len - 1);
					}
				}

				break;
		}

		t = t->next;
	}

}


/// Strong/emph parsing is done using single `*` and `_` characters, which are
/// then combined in a separate routine here to determine when
/// consecutive characters should be interpreted as STRONG instead of EMPH
/// \todo: Perhaps combining this with the routine when they are paired
/// would improve performance?
void pair_emphasis_tokens(token * t) {
	token * closer;

	while (t != NULL) {
		if (t->mate != NULL) {
			switch (t->type) {
				case STAR:
				case UL:
					closer = t->mate;

					if (t->next &&
					        (t->next->mate == closer->prev) &&
					        (t->type == t->next->type) &&
					        (t->next->mate != t) &&
					        (t->start + t->len == t->next->start) &&
					        (closer->start == closer->prev->start + closer->prev->len)) {

						// We have a strong pair
						t->type = STRONG_START;
						t->len = 2;
						closer->type = STRONG_STOP;
						closer->len = 2;
						closer->start--;

						tokens_prune(t->next, t->next);
						tokens_prune(closer->prev, closer->prev);

						token_prune_graft(t, closer, PAIR_STRONG);
					} else {
						t->type = EMPH_START;
						closer->type = EMPH_STOP;
						token_prune_graft(t, closer, PAIR_EMPH);
					}

					break;

				default:
					break;
			}

		}

		if (t->child != NULL) {
			switch (t->type) {
				case PAIR_BACKTICK:
				case PAIR_MATH:
					break;

				default:
					pair_emphasis_tokens(t->child);
					break;
			}
		}

		t = t->next;
	}
}


void recursive_parse_list_item(mmd_engine * e, token * block) {
	token * marker = token_copy(block->child->child);

	// Strip list marker from first line
	token_remove_first_child(block->child);

	// Remove one indent level from all lines to allow recursive parsing
	deindent_block(e, block);

	mmd_parse_token_chain(e, block);

	// Insert marker back in place
	marker->next = block->child->child;

	if (block->child->child) {
		block->child->child->prev = marker;
	}

	block->child->child = marker;
}


void recursive_parse_indent(mmd_engine * e, token * block) {
	// Remove one indent level from all lines to allow recursive parsing
	deindent_block(e, block);

	// First line is now plain text
	block->child->type = LINE_PLAIN;

	// Strip tokens?
	switch (block->type) {
		case BLOCK_DEFINITION:
			// Strip leading ':' from definition
			token_remove_first_child(block->child);
			break;
	}

	mmd_parse_token_chain(e, block);
}


void is_list_loose(token * list) {
	bool loose = false;

	token * walker = list->child;

	if (walker == NULL) {
		return;
	}

	while (walker->next != NULL) {
		if (walker->type == BLOCK_LIST_ITEM) {
			if (walker->child->type == BLOCK_PARA) {
				loose = true;
			} else {
				walker->type = BLOCK_LIST_ITEM_TIGHT;
			}
		}

		walker = walker->next;
	}

	if (loose) {
		switch (list->type) {
			case BLOCK_LIST_BULLETED:
				list->type = BLOCK_LIST_BULLETED_LOOSE;
				break;

			case BLOCK_LIST_ENUMERATED:
				list->type = BLOCK_LIST_ENUMERATED_LOOSE;
				break;
		}
	}
}


/// Is this actually an HTML block?
void is_para_html(mmd_engine * e, token * block) {
	if ((block == NULL) ||
	        (block->child == NULL) ||
	        (block->child->type != LINE_PLAIN)) {
		return;
	}

	token * t = block->child->child;

	if (t->type == ANGLE_LEFT || t->type == HTML_COMMENT_START) {
		if (scan_html_block(&(e->dstr->str[t->start]))) {
			block->type = BLOCK_HTML;
			return;
		}

		if (scan_html_line(&(e->dstr->str[t->start]))) {
			block->type = BLOCK_HTML;
			return;
		}
	}
}


void recursive_parse_blockquote(mmd_engine * e, token * block) {
	// Strip blockquote markers (if present)
	strip_quote_markers_from_block(e, block);

	mmd_parse_token_chain(e, block);
}


void metadata_stack_describe(mmd_engine * e) {
	meta * m;

	for (int i = 0; i < e->metadata_stack->size; ++i) {
		m = stack_peek_index(e->metadata_stack, i);
		fprintf(stderr, "'%s': '%s'\n", m->key, m->value);
	}
}


void strip_line_tokens_from_metadata(mmd_engine * e, token * metadata) {
	token * l = metadata->child;
	char * source = e->dstr->str;

	meta * m = NULL;
	size_t start, len;

	DString * d = d_string_new("");

	while (l) {
		switch (l->type) {
			case LINE_META:
meta:
				if (m) {
					meta_set_value(m, d->str);
					d_string_erase(d, 0, -1);
				}

				len = scan_meta_key(&source[l->start]);
				m = meta_new(source, l->start, len);
				start = l->start + len + 1;
				len = l->start + l->len - start - 1;
				d_string_append_c_array(d, &source[start], len);
				stack_push(e->metadata_stack, m);
				break;

			case LINE_INDENTED_TAB:
			case LINE_INDENTED_SPACE:
				while (l->len && char_is_whitespace(source[l->start])) {
					l->start++;
					l->len--;
				}

			case LINE_PLAIN:
plain:
				d_string_append_c(d, '\n');
				d_string_append_c_array(d, &source[l->start], l->len);
				break;

			case LINE_SETEXT_2:
			case LINE_YAML:
				break;

			case LINE_TABLE:
				if (scan_meta_line(&source[l->start])) {
					goto meta;
				} else {
					goto plain;
				}

			default:
				fprintf(stderr, "ERROR!\n");
				token_describe(l, NULL);
				break;
		}

		l = l->next;
	}

	// Finish last line
	if (m) {
		meta_set_value(m, d->str);
	}

	d_string_free(d, true);
}


void strip_line_tokens_from_deflist(mmd_engine * e, token * deflist) {
	token * walker = deflist->child;

	while (walker) {
		switch (walker->type) {
			case LINE_EMPTY:
				walker->type = TEXT_EMPTY;
				break;

			case LINE_PLAIN:
				walker->type = BLOCK_TERM;

			case BLOCK_TERM:
				break;

			case BLOCK_DEFINITION:
				strip_line_tokens_from_block(e, walker);
				break;
		}

		walker = walker->next;
	}
}


void strip_line_tokens_from_table(mmd_engine * e, token * table) {
	token * walker = table->child;

	while (walker) {
		switch (walker->type) {
			case BLOCK_TABLE_SECTION:
				strip_line_tokens_from_block(e, walker);
				break;

			case BLOCK_TABLE_HEADER:
				strip_line_tokens_from_block(e, walker);
				break;

			case LINE_EMPTY:
				walker->type = TEXT_EMPTY;
				break;
		}

		walker = walker->next;
	}
}


void parse_table_row_into_cells(token * row) {
	token * first = NULL;
	token * last = NULL;

	token * walker = row->child;

	if (walker->type == PIPE) {
		walker->type = TABLE_DIVIDER;
		first = walker->next;
	} else {
		first = walker;
		last = first;
	}

	walker = walker->next;

	while (walker) {
		switch (walker->type) {
			case PIPE:
				token_prune_graft(first, last, TABLE_CELL);
				first = NULL;
				last = NULL;
				walker->type = TABLE_DIVIDER;
				break;

			case TEXT_NL:
			case TEXT_LINEBREAK:
				break;

			default:
				if (!first) {
					first = walker;
				}

				last = walker;
		}

		walker = walker->next;
	}

	if (first) {
		token_prune_graft(first, last, TABLE_CELL);
	}
}


void strip_line_tokens_from_block(mmd_engine * e, token * block) {
	if ((block == NULL) || (block->child == NULL)) {
		return;
	}

	#ifndef NDEBUG
	fprintf(stderr, "Strip line tokens from %d (%lu:%lu) (child %d)\n", block->type, block->start, block->len, block->child->type);
	token_tree_describe(block, NULL);
	#endif

	token * l = block->child;

	// Custom actions
	switch (block->type) {
		case BLOCK_META:
			// Handle metadata differently
			return strip_line_tokens_from_metadata(e, block);

		case BLOCK_CODE_INDENTED:

			// Strip trailing empty lines from indented code blocks
			while (l->tail->type == LINE_EMPTY) {
				token_remove_last_child(block);
			}

			break;

		case BLOCK_DEFLIST:
			// Handle definition lists
			return strip_line_tokens_from_deflist(e, block);

		case BLOCK_TABLE:
			// Handle tables
			return strip_line_tokens_from_table(e, block);
	}

	token * children = NULL;
	block->child = NULL;

	token * temp;

	// Move contents of line directly into the parent block
	while (l != NULL) {
		switch (l->type) {
			case LINE_SETEXT_1:
			case LINE_SETEXT_2:
				if ((block->type == BLOCK_SETEXT_1) ||
				        (block->type == BLOCK_SETEXT_2)) {
					temp = l->next;
					tokens_prune(l, l);
					l = temp;
					break;
				}

			case LINE_DEFINITION:
				if (block->type == BLOCK_DEFINITION) {
					// Remove leading colon
					token_remove_first_child(l);
				}

			case LINE_ATX_1:
			case LINE_ATX_2:
			case LINE_ATX_3:
			case LINE_ATX_4:
			case LINE_ATX_5:
			case LINE_ATX_6:
			case LINE_BLOCKQUOTE:
			case LINE_CONTINUATION:
			case LINE_DEF_ABBREVIATION:
			case LINE_DEF_CITATION:
			case LINE_DEF_FOOTNOTE:
			case LINE_DEF_GLOSSARY:
			case LINE_DEF_LINK:
			case LINE_EMPTY:
			case LINE_LIST_BULLETED:
			case LINE_LIST_ENUMERATED:
			case LINE_META:
			case LINE_PLAIN:
			case LINE_START_COMMENT:
			case LINE_STOP_COMMENT:
handle_line:

				// Remove leading non-indent space from line
				if (block->type != BLOCK_CODE_FENCED && l->child && l->child->type == NON_INDENT_SPACE) {
					token_remove_first_child(l);
				}

			case LINE_INDENTED_TAB:
			case LINE_INDENTED_SPACE:

				// Strip leading indent (Only the first one)
				if (block->type != BLOCK_CODE_FENCED && l->child && ((l->child->type == INDENT_SPACE) || (l->child->type == INDENT_TAB))) {
					token_remove_first_child(l);
				}

				// If we're not a code block, strip additional indents
				if ((block->type != BLOCK_CODE_INDENTED) &&
				        (block->type != BLOCK_CODE_FENCED)) {
					while (l->child && ((l->child->type == INDENT_SPACE) || (l->child->type == INDENT_TAB))) {
						token_remove_first_child(l);
					}
				}

				// Add contents of line to parent block
				token_append_child(block, l->child);

				// Disconnect line from it's contents
				l->child = NULL;

				// Need to remember first line we strip
				if (children == NULL) {
					children = l;
				}

				// Advance to next line
				l = l->next;
				break;

			case BLOCK_DEFINITION:
				// Sometimes these get created unintentionally inside other blocks
				// Process inside it, then treat it like a line to be stripped

				// Change to plain line
				l->child->type = LINE_PLAIN;
				strip_line_tokens_from_block(e, l);

				// Move children to parent
				// Add ':' back
				if (e->dstr->str[l->child->start - 1] == ':') {
					temp = token_new(COLON, l->child->start - 1, 1);
					token_append_child(block, temp);
				}

				token_append_child(block, l->child);
				l->child = NULL;

				if (children == NULL) {
					children = l;
				}

				l = l->next;
				break;

			case LINE_TABLE_SEPARATOR:
			case LINE_TABLE:
				if (block->type == BLOCK_TABLE_HEADER) {
					l->type = (l->type == LINE_TABLE) ? TABLE_ROW : LINE_TABLE_SEPARATOR;
					parse_table_row_into_cells(l);
				} else if (block->type == BLOCK_TABLE_SECTION) {
					l->type =  TABLE_ROW;
					parse_table_row_into_cells(l);
				} else {
					goto handle_line;
				}

			default:
				// token_describe(block, e->dstr->str);
				// fprintf(stderr, "Unspecified line type %d inside block type %d\n", l->type, block->type);
				// This is a block, need to remove it from chain and
				// Add to parent
				temp = l->next;

				token_pop_link_from_chain(l);
				token_append_child(block, l);

				// Advance to next line
				l = temp;
				break;
		}
	}

	// Free token chain of line types
	token_tree_free(children);
}


/// Parse part of the string into a token tree
token * mmd_engine_parse_substring(mmd_engine * e, size_t byte_start, size_t byte_len) {
	// First, clean up any leftovers from previous parse

	mmd_engine_reset(e);

	// Tokenize the string
	token * doc = mmd_tokenize_string(e, byte_start, byte_len, false);

	// Parse tokens into blocks
	mmd_parse_token_chain(e, doc);

	if (doc) {
		// Parse blocks for pairs
		mmd_assign_ambidextrous_tokens_in_block(e, doc, 0);

		// Prepare stack to be used for token pairing
		// This avoids allocating/freeing one for each iteration.
		stack * pair_stack = stack_new(0);


		mmd_pair_tokens_in_block(doc, e->pairings1, pair_stack);
		mmd_pair_tokens_in_block(doc, e->pairings2, pair_stack);
		mmd_pair_tokens_in_block(doc, e->pairings3, pair_stack);
		mmd_pair_tokens_in_block(doc, e->pairings4, pair_stack);

		// Free stack
		stack_free(pair_stack);

		pair_emphasis_tokens(doc);

		#ifndef NDEBUG
		token_tree_describe(doc, e->dstr->str);
		#endif
	}

	return doc;
}


/// Parse the entire string into a token tree
void mmd_engine_parse_string(mmd_engine * e) {
	if (e) {
		e->root = mmd_engine_parse_substring(e, 0, e->dstr->currentStringLength);
	}
}


/// Does the text have metadata?
bool mmd_string_has_metadata(char * source, size_t * end) {
	bool result;

	mmd_engine * e = mmd_engine_create_with_string(source, 0);
	result = mmd_engine_has_metadata(e, end);

	mmd_engine_free(e, true);

	return result;
}


/// Does the text have metadata?
bool mmd_d_string_has_metadata(DString * source, size_t * end) {
	bool result;

	mmd_engine * e = mmd_engine_create_with_dstring(source, 0);
	result = mmd_engine_has_metadata(e, end);

	mmd_engine_free(e, false);

	return result;
}


/// Does the text have metadata?
bool mmd_engine_has_metadata(mmd_engine * e, size_t * end) {
	bool result = false;

	if (!e) {
		return false;
	}

	if (!(scan_meta_line(&e->dstr->str[0]))) {
		// First line is not metadata, so can't have metadata
		// Saves the time of an unnecessary parse
		// TODO:  Need faster confirmation of actual metadata than full tokenizing
		if (end) {
			*end = 0;
		}

		return false;
	}

	// Free existing parse tree
	if (e->root) {
		token_tree_free(e->root);
	}

	// Tokenize the string (up until first empty line)
	token * doc = mmd_tokenize_string(e, 0, e->dstr->currentStringLength, true);

	// Parse tokens into blocks
	mmd_parse_token_chain(e, doc);

	if (doc) {
		if (doc->child && doc->child->type == BLOCK_META) {
			result = true;

			if (end) {
				*end = doc->child->len;
			}
		}

		token_tree_free(doc);
	}

	return result;
}


/// Return metadata keys, one per line
/// Returned char * must be freed
char * mmd_string_metadata_keys(char * source) {
	char * result;

	mmd_engine * e = mmd_engine_create_with_string(source, 0);
	result = mmd_engine_metadata_keys(e);

	mmd_engine_free(e, true);

	return result;
}


/// Return metadata keys, one per line
/// Returned char * must be freed
char * mmd_d_string_metadata_keys(DString * source) {
	char * result;

	mmd_engine * e = mmd_engine_create_with_dstring(source, 0);
	result = mmd_engine_metadata_keys(e);

	mmd_engine_free(e, false);

	return result;
}


/// Return metadata keys, one per line
/// Returned char * must be freed
char * mmd_engine_metadata_keys(mmd_engine * e) {
	if (e->metadata_stack->size == 0) {
		// Ensure we have checked for metadata
		if (!mmd_engine_has_metadata(e, NULL)) {
			return NULL;
		}
	}

	char * result = NULL;
	DString * output = d_string_new("");

	meta * m;

	for (int i = 0; i < e->metadata_stack->size; ++i) {
		m = stack_peek_index(e->metadata_stack, i);

		d_string_append_printf(output, "%s\n", m->key);
	}

	result = output->str;
	d_string_free(output, false);

	return result;
}


/// Extract desired metadata as string value
/// Returned char * must be freed
char * mmd_string_metavalue_for_key(char * source, const char * key) {
	char * result;

	mmd_engine * e = mmd_engine_create_with_string(source, 0);
	result = mmd_engine_metavalue_for_key(e, key);
	result = my_strdup(result);

	mmd_engine_free(e, true);

	return result;
}


/// Extract desired metadata as string value
/// Returned char * must be freed
char * mmd_d_string_metavalue_for_key(DString * source, const char * key) {
	char * result;

	mmd_engine * e = mmd_engine_create_with_dstring(source, 0);
	result = mmd_engine_metavalue_for_key(e, key);

	if (result) {
		result = my_strdup(result);
	}

	mmd_engine_free(e, false);

	return result;
}


/// Grab metadata without processing entire document
/// Returned char * does not need to be freed
char * mmd_engine_metavalue_for_key(mmd_engine * e, const char * key) {
	if (e->metadata_stack->size == 0) {
		// Ensure we have checked for metadata
		if (!mmd_engine_has_metadata(e, NULL)) {
			return NULL;
		}
	}

	char * result = NULL;
	char * clean = label_from_string(key);

	meta * m;

	for (int i = 0; i < e->metadata_stack->size; ++i) {
		m = stack_peek_index(e->metadata_stack, i);

		if (strcmp(clean, m->key) == 0) {
			// We have a match
			free(clean);
			return m->value;
		}
	}

	free(clean);
	return result;
}


/// Insert/replace metadata in string, returning new string
char * mmd_string_update_metavalue_for_key(const char * source, const char * key, const char * value) {
	mmd_engine * e = mmd_engine_create_with_string(source, 0);
	mmd_engine_update_metavalue_for_key(e, key, value);

	DString * d = e->dstr;

	mmd_engine_free(e, false);

	char * result = d->str;
	d_string_free(d, false);

	return result;
}


/// Insert/replace metadata value in DString
void mmd_d_string_update_metavalue_for_key(DString * source, const char * key, const char * value) {
	mmd_engine * e = mmd_engine_create_with_dstring(source, 0);
	mmd_engine_update_metavalue_for_key(e, key, value);

	mmd_engine_free(e, false);
}


/// Insert/replace metadata value in mmd_engine
void mmd_engine_update_metavalue_for_key(mmd_engine * e, const char * key, const char * value) {
	bool has_meta = true;
	size_t meta_end = 0;

	// Check for metadata and character
	if (!mmd_engine_has_metadata(e, &meta_end)) {
		has_meta = false;
	}

	// Get clean metadata key for match
	char * clean = label_from_string(key);

	// Determine range to excise and replace
	size_t start = -1;
	size_t end = -1;
	size_t len = -1;

	meta * m;

	for (int i = 0; i < e->metadata_stack->size; ++i) {
		m = stack_peek_index(e->metadata_stack, i);

		if (strcmp(clean, m->key) == 0) {
			// We have a match
			start = m->start;
		} else if (start != -1) {
			// We have already found a match
			if (end == -1) {
				// This is the next metadata key, so determine length
				end = m->start;
			}
		}
	}

	DString * temp = d_string_new(key);
	d_string_append(temp, ":\t");
	d_string_append(temp, value);
	d_string_append_c(temp, '\n');

	if (start != -1) {
		// We're replacing existing metadata

		// Figure out where to start
		char * begin = &(e->dstr->str[start]);

		while (*begin != ':') {
			begin++;
		}

		begin++;

		while (char_is_whitespace(*begin)) {
			begin++;
		}

		start = begin - e->dstr->str;

		if (end == -1) {
			// Replace until the end of the metadata (last key)
			len = meta_end - start;
		} else {
			len = end - start;
		}

		d_string_erase(e->dstr, start, len);
		d_string_insert(e->dstr, start, "\n");
		d_string_insert(e->dstr, start, value);
	} else if (meta_end != 0) {
		// We're appending metadata at the end
		d_string_insert(e->dstr, meta_end, temp->str);
	} else {
		// There is no metadata, so prepend before document
		d_string_append_c(temp, '\n');
		d_string_prepend(e->dstr, temp->str);
	}

	d_string_free(temp, true);
	free(clean);
}


/// Convert MMD text to specified format, with specified extensions, and language
/// Returned char * must be freed
char * mmd_string_convert(const char * source, unsigned long extensions, short format, short language) {
	char * result;

	mmd_engine * e = mmd_engine_create_with_string(source, extensions);

	mmd_engine_set_language(e, language);

	result = mmd_engine_convert(e, format);

	mmd_engine_free(e, true);			// The engine has a private copy of source that must be freed

	return result;
}


/// Convert MMD text to specified format, with specified extensions, and language
/// Returned char * must be freed
char * mmd_d_string_convert(DString * source, unsigned long extensions, short format, short language) {
	char * result;

	mmd_engine * e = mmd_engine_create_with_dstring(source, extensions);

	mmd_engine_set_language(e, language);

	result = mmd_engine_convert(e, format);

	mmd_engine_free(e, false);			// The engine doesn't own the DString, so don't free it.

	return result;
}


/// Convert MMD text to specified format, with specified extensions, and language
/// Returned char * must be freed
char * mmd_engine_convert(mmd_engine * e, short format) {
	char * result;

	mmd_engine_parse_string(e);

	DString * output = d_string_new("");

	mmd_engine_export_token_tree(output, e, format);

	// Add newline to result
	d_string_append_c(output, '\n');

	result = output->str;

	d_string_free(output, false);

	return result;
}


/// Convert MMD text and write results to specified file -- used for "complex" output formats requiring
/// multiple documents (e.g. EPUB)
void mmd_string_convert_to_file(const char * source, unsigned long extensions, short format, short language, const char * directory, const char * filepath) {

	mmd_engine * e = mmd_engine_create_with_string(source, extensions);

	mmd_engine_set_language(e, language);

	mmd_engine_parse_string(e);

	mmd_engine_free(e, true);			// The engine has a private copy of source, so free it.
}


/// Convert MMD text and write results to specified file -- used for "complex" output formats requiring
/// multiple documents (e.g. EPUB)
void mmd_d_string_convert_to_file(DString * source, unsigned long extensions, short format, short language, const char * directory, const char * filepath) {

	mmd_engine * e = mmd_engine_create_with_dstring(source, extensions);

	mmd_engine_set_language(e, language);

	mmd_engine_convert_to_file(e, format, directory, filepath);

	mmd_engine_free(e, false);			// The engine doesn't own the DString, so don't free it.
}


/// Convert MMD text and write results to specified file -- used for "complex" output formats requiring
/// multiple documents (e.g. EPUB)
void mmd_engine_convert_to_file(mmd_engine * e, short format, const char * directory, const char * filepath) {
	FILE * output_stream;

	DString * output = d_string_new("");

	mmd_engine_parse_string(e);

	mmd_engine_export_token_tree(output, e, format);

	// Now we have the input source string, the output string, the (modified) parse tree, and engine stacks

	switch (format) {
		case FORMAT_EPUB:
			epub_write_wrapper(filepath, output->str, e, directory);
			break;

		case FORMAT_TEXTBUNDLE:
			// TODO: Need to implement this
			break;

		case FORMAT_TEXTBUNDLE_COMPRESSED:
			textbundle_write_wrapper(filepath, output->str, e, directory);
			break;

		default:

			// Basic formats just write to file
			if (!(output_stream = fopen(filepath, "w"))) {
				// Failed to open file
				perror(filepath);
			} else {
				fputs(output->str, output_stream);
				fputc('\n', output_stream);
				fclose(output_stream);
			}

			break;
	}

	d_string_free(output, true);
}


DString * mmd_string_convert_to_data(const char * source, unsigned long extensions, short format, short language, const char * directory) {
	mmd_engine * e = mmd_engine_create_with_string(source, extensions);

	mmd_engine_set_language(e, language);

	DString * result = mmd_engine_convert_to_data(e, format, directory);

	mmd_engine_free(e, true);

	return result;
}


DString * mmd_d_string_convert_to_data(DString * source, unsigned long extensions, short format, short language, const char * directory) {
	mmd_engine * e = mmd_engine_create_with_dstring(source, extensions);

	mmd_engine_set_language(e, language);

	DString * result =  mmd_engine_convert_to_data(e, format, directory);

	mmd_engine_free(e, false);			// The engine doesn't own the DString, so don't free it.

	return result;
}


DString * mmd_engine_convert_to_data(mmd_engine * e, short format, const char * directory) {
	DString * output = d_string_new("");
	DString * result = NULL;

	mmd_engine_parse_string(e);

	mmd_engine_export_token_tree(output, e, format);

	switch (format) {
		case FORMAT_EPUB:
			result = epub_create(output->str, e, directory);

			d_string_free(output, true);
			break;

		case FORMAT_TEXTBUNDLE:
		case FORMAT_TEXTBUNDLE_COMPRESSED:
			result = textbundle_create(output->str, e, directory);

			d_string_free(output, true);
			break;

		case FORMAT_ODT:
			result = opendocument_text_create(output->str, e, directory);

			d_string_free(output, true);
			break;

		case FORMAT_FODT:
			result = opendocument_flat_text_create(output->str, e, directory);

			d_string_free(output, true);
			break;

		default:
			result = output;
			// Add newline to result
			d_string_append_c(result, '\n');
			break;
	}

	return result;
}


/// Return string containing engine version.
char * mmd_version(void) {
	char * result;
	result = my_strdup(MULTIMARKDOWN_VERSION);
	return result;
}
