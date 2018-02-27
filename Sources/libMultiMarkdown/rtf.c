
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "char.h"
#include "d_string.h"
#include "html.h"
#include "i18n.h"
#include "libMultiMarkdown.h"
#include "parser.h"
#include "token.h"
#include "scanners.h"
#include "writer.h"


#define print(x) d_string_append(out, x)
#define print_const(x) d_string_append_c_array(out, x, sizeof(x) - 1)
#define print_char(x) d_string_append_c(out, x)
#define printf(...) d_string_append_printf(out, __VA_ARGS__)
#define print_token(t) d_string_append_c_array(out, &(source[t->start]), t->len)
#define print_localized(x) mmd_print_localized_char_html(out, x, scratch)


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


static void mmd_export_token_tree(DString * out, const char * source, token * t, scratch_pad * scratch);


static void mmd_export_token(DString * out, const char * source, token * t, scratch_pad * scratch) {
	switch(t->type) {
		case 0:
			print_const("{\\rtf1\\ansi\\ansicpg1252\\cocoartf1504\\cocoasubrtf830\n{\\fonttbl\\f0\\fswiss\\fcharset0 Helvetica;}\n{\\colortbl;\\red255\\green255\\blue255;\\red191\\green191\\blue191;}\n{\\*\\expandedcolortbl;;\\csgray\\c79525;}\n{\\*\\listtable{\\list\\listtemplateid1\\listhybrid{\\listlevel\\levelnfc23\\levelnfcn23\\leveljc0\\leveljcn0\\levelfollow0\\levelstartat1\\levelspace360\\levelindent0{\\*\\levelmarker \\{disc\\}}{\\leveltext\\leveltemplateid1\\'01\\uc0\\u8226 ;}{\\levelnumbers;}\\fi-360\\li720\\lin720 }{\\listname ;}\\listid1}}\n{\\*\\listoverridetable{\\listoverride\\listid1\\listoverridecount0\\ls1}}\n\\margl1440\\margr1440\\vieww10800\\viewh8400\\viewkind0\n\\pard\\tx720\\tx1440\\tx2160\\tx2880\\tx3600\\tx4320\\tx5040\\tx5760\\tx6480\\tx7200\\tx7920\\tx8640\\pardirnatural\\partightenfactor0\n\n\\f0\\fs24 \\cf0 ");
			if (t->child) {
				mmd_export_token_tree(out, source, t->child, scratch);
			}
			break;
		case 60:
			print_const("\\\n");
			if (t->child) {
				mmd_export_token_tree(out, source, t->child, scratch);
			}
			break;
		case 62:
			print_const("\\pard\\tx720\\tx1440\\tx2160\\tx2880\\tx3600\\tx4320\\tx5040\\tx5760\\tx6480\\tx7200\\tx7920\\tx8640\\pardirnatural\\partightenfactor0\n\n\\b\\fs36 \\cf0 ");
			if (t->child) {
				mmd_export_token_tree(out, source, t->child, scratch);
			}
			print_const("\n\\b0\\fs24 \\\n");
			break;
		case 63:
			print_const("\n\\b\\fs32 ");
			if (t->child) {
				mmd_export_token_tree(out, source, t->child, scratch);
			}
			print_const("\n\\b0\\fs24 \\\n");
			break;
		case 64:
			print_const("\n\\b\\fs28 ");
			if (t->child) {
				mmd_export_token_tree(out, source, t->child, scratch);
			}
			print_const("\n\\b0\\fs24 \\\n");
			break;
		case 65:
			print_const("\n\\i\\b\\fs26 ");
			if (t->child) {
				mmd_export_token_tree(out, source, t->child, scratch);
			}
			print_const("\n\\i0\\b0\\fs24 \\\n");
			break;
		case 66:
			print_const("\\pard\\tx720\\tx1440\\tx2160\\tx2880\\tx3600\\tx4320\\tx5040\\tx5760\\tx6480\\tx7200\\tx7920\\tx8640\\pardirnatural\\partightenfactor0\n\n\\b \\cf0 ");
			if (t->child) {
				mmd_export_token_tree(out, source, t->child, scratch);
			}
			print_const("\n\\b0 \\\n");
			break;
		case 67:
			print_const("\\pard\\tx720\\tx1440\\tx2160\\tx2880\\tx3600\\tx4320\\tx5040\\tx5760\\tx6480\\tx7200\\tx7920\\tx8640\\pardirnatural\\partightenfactor0\n\n\\i\\b \\cf0 ");
			if (t->child) {
				mmd_export_token_tree(out, source, t->child, scratch);
			}
			print_const("\n\\i0\\b0 \\\n");
			break;
		case 71:
			print_const("\\pard\\tx220\\tx720\\tx1440\\tx2160\\tx2880\\tx3600\\tx4320\\tx5040\\tx5760\\tx6480\\tx7200\\tx7920\\tx8640\\li720\\fi-720\\pardirnatural\\partightenfactor0\n\\ls1\\ilvl0\\cf0 ");
			if (t->child) {
				mmd_export_token_tree(out, source, t->child, scratch);
			}
			print_const("\\pard\\tx720\\tx1440\\tx2160\\tx2880\\tx3600\\tx4320\\tx5040\\tx5760\\tx6480\\tx7200\\tx7920\\tx8640\\pardirnatural\\partightenfactor0\n\\cf0 ");
			break;
		case 74:
			print_const("{\\listtext\t\\'95\t}");
			if (t->child) {
				mmd_export_token_tree(out, source, t->child, scratch);
			}
			break;
		case 80:
			print_const("\n\\itap1\\trowd \\taflags1 \\trgaph108\\trleft-108 \\trbrdrt\\brdrnil \\trbrdrl\\brdrnil \\trbrdrr\\brdrnil \n\\clvertalc \\clshdrawnil \\clbrdrt\\brdrs\\brdrw20\\brdrcf2 \\clbrdrl\\brdrs\\brdrw20\\brdrcf2 \\clbrdrb\\brdrs\\brdrw20\\brdrcf2 \\clbrdrr\\brdrs\\brdrw20\\brdrcf2 \\clpadl100 \\clpadr100 \\gaph\\cellx4320\n\\clvertalc \\clshdrawnil \\clbrdrt\\brdrs\\brdrw20\\brdrcf2 \\clbrdrl\\brdrs\\brdrw20\\brdrcf2 \\clbrdrb\\brdrs\\brdrw20\\brdrcf2 \\clbrdrr\\brdrs\\brdrw20\\brdrcf2 \\clpadl100 \\clpadr100 \\gaph\\cel");
			if (t->child) {
				mmd_export_token_tree(out, source, t->child, scratch);
			}
			break;
		case 82:
			print_const("bl\\itap1\\tx720\\tx1440\\tx2160\\tx2880\\tx3600\\tx4320\\tx5040\\tx5760\\tx6480\\tx7200\\tx7920\\tx8640\\pardirnatural\\partightenfactor0\n\\cf0 musical\\cell \n\\pard\\intbl\\itap1\\tx720\\tx1440\\tx2160\\tx2880\\tx3600\\tx4320\\tx5040\\tx5760\\tx6480\\tx7200\\tx7920\\tx8640\\pardirnatural\\partightenfactor0\n\\cf0 suspicion\\cell \\row\n\n\\itap1\\trowd \\taflags1 \\trgaph108\\trleft-108 \\trbrdrl\\brdrnil \\trbrdrt\\brdrnil \\trbrdrr\\brdrnil \n\\clvertalc \\clshdrawnil \\clbrdrt\\brdrs\\brdrw20\\brdrcf2 \\clbrdrl\\brdrs\\brdrw");
			if (t->child) {
				mmd_export_token_tree(out, source, t->child, scratch);
			}
			break;
		case 115:
			print_const(" \n\\i ");
			if (t->child) {
				mmd_export_token_tree(out, source, t->child, scratch);
			}
			print_const("\n\\i0  ");
			break;
		case 125:
			print_const(" \n\\b ");
			if (t->child) {
				mmd_export_token_tree(out, source, t->child, scratch);
			}
			print_const("\n\\b0  ");
			break;
		case 216:
			print_token(t);
			if (t->child) {
				mmd_export_token_tree(out, source, t->child, scratch);
			}
			break;
		default:
			if (t->child) {
				mmd_export_token_tree(out, source, t->child, scratch);
			} else {
			//	print_token(t);
			}
			break;
	}
}



static void mmd_export_token_tree(DString * out, const char * source, token * t, scratch_pad * scratch) {

	// Prevent stack overflow with "dangerous" input causing extreme recursion
	if (scratch->recurse_depth == kMaxExportRecursiveDepth) {
		return;
	}

	scratch->recurse_depth++;

	while (t != NULL) {
		if (scratch->skip_token) {
			scratch->skip_token--;
		} else {
			mmd_export_token(out, source, t, scratch);
		}

		t = t->next;
	}

	scratch->recurse_depth--;
}


void mmd_export_rtf(DString * out, const char * source, token * t, scratch_pad * scratch) { 
	mmd_export_token_tree(out, source, t, scratch);
}
