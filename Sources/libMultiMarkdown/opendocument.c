/**

	MultiMarkdown -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file opendocument.c

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
	
	uthash library:
		Copyright (c) 2005-2016, Troy D. Hanson
	
		Licensed under Revised BSD license
	
	miniz library:
		Copyright 2013-2014 RAD Game Tools and Valve Software
		Copyright 2010-2014 Rich Geldreich and Tenacious Software LLC
	
		Licensed under the MIT license
	
	argtable3 library:
		Copyright (C) 1998-2001,2003-2011,2013 Stewart Heitmann
		<sheitmann@users.sourceforge.net>
		All rights reserved.
	
		Licensed under the Revised BSD License
	
	
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
	
	
	## Revised BSD License ##
	
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are
	met:
	    * Redistributions of source code must retain the above copyright
	      notice, this list of conditions and the following disclaimer.
	    * Redistributions in binary form must reproduce the above
	      copyright notice, this list of conditions and the following
	      disclaimer in the documentation and/or other materials provided
	      with the distribution.
	    * Neither the name of the <organization> nor the
	      names of its contributors may be used to endorse or promote
	      products derived from this software without specific prior
	      written permission.
	
	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT
	HOLDER> BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES LOSS OF USE, DATA, OR
	PROFITS OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
	

*/

#ifdef USE_CURL
#include <curl/curl.h>
#endif

#include "miniz.h"
#include "opendocument.h"
#include "opendocument-content.h"
#include "transclude.h"
#include "writer.h"
#include "zip.h"

#define print(x) d_string_append(out, x)
#define print_const(x) d_string_append_c_array(out, x, sizeof(x) - 1)
#define print_char(x) d_string_append_c(out, x)
#define printf(...) d_string_append_printf(out, __VA_ARGS__)
#define print_token(t) d_string_append_c_array(out, &(source[t->start]), t->len)
#define print_localized(x) mmd_print_localized_char_html(out, x, scratch)


/// strdup() not available on all platforms
static char * my_strdup(const char * source) {
	char * result = malloc(strlen(source) + 1);

	if (result) {
		strcpy(result, source);
	}

	return result;
}


static bool add_asset_from_file(mz_zip_archive * pZip, asset * a, const char * destination, const char * directory) {
	if (!directory)
		return false;
	
	char * path = path_from_dir_base(directory, a->url);
	mz_bool status;
	bool result = false;

	DString * buffer = scan_file(path);

	if (buffer && buffer->currentStringLength > 0) {
		status = mz_zip_writer_add_mem(pZip, destination, buffer->str, buffer->currentStringLength, MZ_BEST_COMPRESSION);

		if (!status) {
			fprintf(stderr, "Error adding asset '%s' to zip.\n", destination);
		}

		d_string_free(buffer, true);
		result = true;
	}

	free(path);

	return result;
}


/// Create metadata for OpenDocument
char * opendocument_metadata(mmd_engine * e, scratch_pad * scratch) {
	DString * out = d_string_new("");
	meta * m;

	d_string_append(out, "<office:meta>\n");

	// Iterate through metadata keys
	for (m = scratch->meta_hash; m != NULL; m = m->hh.next) {
		if (strcmp(m->key, "author") == 0) {
			print_const("\t<dc:creator>");
			mmd_print_string_opendocument(out, m->value);
			print_const("</dc:creator>\n");
		} else if (strcmp(m->key, "baseheaderlevel") == 0) {
		} else if (strcmp(m->key, "bibliostyle") == 0) {
		} else if (strcmp(m->key, "bibtex") == 0) {
		} else if (strcmp(m->key, "css") == 0) {
		} else if (strcmp(m->key, "htmlfooter") == 0) {
		} else if (strcmp(m->key, "htmlheader") == 0) {
		} else if (strcmp(m->key, "htmlheaderlevel") == 0) {
		} else if (strcmp(m->key, "language") == 0) {
		} else if (strcmp(m->key, "latexbegin") == 0) {
		} else if (strcmp(m->key, "latexconfig") == 0) {
		} else if (strcmp(m->key, "latexfooter") == 0) {
		} else if (strcmp(m->key, "latexheaderlevel") == 0) {
		} else if (strcmp(m->key, "latexinput") == 0) {
		} else if (strcmp(m->key, "latexleader") == 0) {
		} else if (strcmp(m->key, "latexmode") == 0) {
		} else if (strcmp(m->key, "mmdfooter") == 0) {
		} else if (strcmp(m->key, "mmdheader") == 0) {
		} else if (strcmp(m->key, "quoteslanguage") == 0) {
		} else if (strcmp(m->key, "title") == 0) {
			print_const("\t<dc:title>");
			mmd_print_string_opendocument(out, m->value);
			print_const("</dc:title>\n");
		} else if (strcmp(m->key, "transcludebase") == 0) {
		} else if (strcmp(m->key, "xhtmlheader") == 0) {
		} else if (strcmp(m->key, "xhtmlheaderlevel") == 0) {
		} else {
			print_const("\t<meta:user-defined meta:name=\"");
			mmd_print_string_opendocument(out, m->key);
			print_const("\">");
			mmd_print_string_opendocument(out, m->value);
			print_const("</meta:user-defined>\n");
		}
	}

	d_string_append(out, "</office:meta>");
	
	char * result = out->str;
	d_string_free(out, false);
	return result;
}


/// Create full metadata file for OpenDocument
char * opendocument_metadata_file(mmd_engine * e, scratch_pad * scratch) {
	DString * out = d_string_new("");

	char * meta = opendocument_metadata(e, scratch);

	// Open
	d_string_append(out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	d_string_append(out, "<office:document-meta xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\" xmlns:ooo=\"http://openoffice.org/2004/office\" xmlns:grddl=\"http://www.w3.org/2003/g/data-view#\" office:version=\"1.2\">\n");

	// Metadata
	d_string_append(out, meta);

	// Close
	d_string_append(out, "\n</office:document-meta>");

	// Cleanup
	free(meta);
	char * result = out->str;
	d_string_free(out, false);
	return result;
}


/// Create default style information for OpenDocument
char * opendocument_style(int format) {
	DString * out = d_string_new("");

		/* Font Declarations */
	print_const("<office:font-face-decls>\n" \
	"   <style:font-face style:name=\"Courier New\" svg:font-family=\"'Courier New'\"\n" \
    "                    style:font-adornments=\"Regular\"\n" \
    "                    style:font-family-generic=\"modern\"\n" \
    "                    style:font-pitch=\"fixed\"/>\n" \
    "</office:font-face-decls>\n");
    
    /* Append basic style information */
    print_const("<office:styles>\n" \
    "<style:style style:name=\"Standard\" style:family=\"paragraph\" style:class=\"text\">\n" \
    "      <style:paragraph-properties fo:margin-top=\"0in\" fo:margin-bottom=\"0.15in\"" \
    "     fo:text-align=\"justify\" style:justify-single-word=\"false\"/>\n" \
    "   </style:style>\n" \
    "<style:style style:name=\"Preformatted_20_Text\" style:display-name=\"Preformatted Text\"\n" \
    "             style:family=\"paragraph\"\n" \
    "             style:parent-style-name=\"Standard\"\n" \
    "             style:class=\"html\">\n" \
    "   <style:paragraph-properties fo:margin-top=\"0in\" fo:margin-bottom=\"0in\" fo:text-align=\"start\"\n" \
    "                               style:justify-single-word=\"false\"/>\n" \
    "   <style:text-properties style:font-name=\"Courier New\" fo:font-size=\"11pt\"\n" \
    "                          style:font-name-asian=\"Courier New\"\n" \
    "                          style:font-size-asian=\"11pt\"\n" \
    "                          style:font-name-complex=\"Courier New\"\n" \
    "                          style:font-size-complex=\"11pt\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"Source_20_Text\" style:display-name=\"Source Text\"\n" \
    "             style:family=\"text\">\n" \
    "   <style:text-properties style:font-name=\"Courier New\" style:font-name-asian=\"Courier New\"\n" \
    "                          style:font-name-complex=\"Courier New\"\n" \
    "                          fo:font-size=\"11pt\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"List\" style:family=\"paragraph\"\n" \
    "             style:parent-style-name=\"Standard\"\n" \
    "             style:class=\"list\">\n" \
    "   <style:paragraph-properties fo:text-align=\"start\" style:justify-single-word=\"false\"/>\n" \
    "   <style:text-properties style:font-size-asian=\"12pt\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"Quotations\" style:family=\"paragraph\"\n" \
    "             style:parent-style-name=\"Standard\"\n" \
    "             style:class=\"html\">\n" \
    "   <style:paragraph-properties fo:margin-left=\"0.3937in\" fo:margin-right=\"0.3937in\" fo:margin-top=\"0in\"\n" \
    "                               fo:margin-bottom=\"0.1965in\"\n" \
    "                               fo:text-align=\"justify\"" \
    "                               style:justify-single-word=\"false\"" \
    "                               fo:text-indent=\"0in\"\n" \
    "                               style:auto-text-indent=\"false\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"Table_20_Heading\" style:display-name=\"Table Heading\"\n" \
    "             style:family=\"paragraph\"\n" \
    "             style:parent-style-name=\"Table_20_Contents\"\n" \
    "             style:class=\"extra\">\n" \
    "   <style:paragraph-properties fo:text-align=\"center\" style:justify-single-word=\"false\"\n" \
    "                               text:number-lines=\"false\"\n" \
    "                               text:line-number=\"0\"/>\n" \
    "   <style:text-properties fo:font-weight=\"bold\" style:font-weight-asian=\"bold\"\n" \
    "                          style:font-weight-complex=\"bold\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"Horizontal_20_Line\" style:display-name=\"Horizontal Line\"\n" \
    "             style:family=\"paragraph\"\n" \
    "             style:parent-style-name=\"Standard\"\n" \
    "             style:class=\"html\">\n" \
    "   <style:paragraph-properties fo:margin-top=\"0in\" fo:margin-bottom=\"0.1965in\"\n" \
    "                               style:border-line-width-bottom=\"0.0008in 0.0138in 0.0008in\"\n" \
    "                               fo:padding=\"0in\"\n" \
    "                               fo:border-left=\"none\"\n" \
    "                               fo:border-right=\"none\"\n" \
    "                               fo:border-top=\"none\"\n" \
    "                               fo:border-bottom=\"0.0154in double #808080\"\n" \
    "                               text:number-lines=\"false\"\n" \
    "                               text:line-number=\"0\"\n" \
    "                               style:join-border=\"false\"/>\n" \
    "   <style:text-properties fo:font-size=\"6pt\" style:font-size-asian=\"6pt\" style:font-size-complex=\"6pt\"/>\n" \
    "</style:style>\n" \
	"<style:style style:name=\"Footnote_20_anchor\" style:display-name=\"Footnote anchor\"" \
	"              style:family=\"text\">" \
	"    <style:text-properties style:text-position=\"super 58%\"/>" \
	" </style:style>\n" \
  	"<style:style style:name=\"TOC_Item\" style:family=\"paragraph\" style:parent-style-name=\"Standard\">\n" \
  	" <style:paragraph-properties>\n" \
  	"  <style:tab-stops>\n" \
  	"   <style:tab-stop style:position=\"6.7283in\" style:type=\"right\" style:leader-style=\"dotted\" style:leader-text=\".\"/>\n" \
  	"  </style:tab-stops>\n" \
  	" </style:paragraph-properties>\n" \
  	"</style:style>\n" \
	"  <text:notes-configuration text:note-class=\"footnote\" text:default-style-name=\"Footnote\" text:citation-style-name=\"Footnote_20_Symbol\" text:citation-body-style-name=\"Footnote_20_anchor\" text:master-page-name=\"Footnote\" style:num-format=\"a\" text:start-value=\"0\" text:footnotes-position=\"page\" text:start-numbering-at=\"page\"/>\n" \
	"  <text:notes-configuration text:note-class=\"endnote\" text:default-style-name=\"Endnote\" text:citation-style-name=\"Endnote_20_Symbol\" text:citation-body-style-name=\"Endnote_20_anchor\" text:master-page-name=\"Endnote\" style:num-format=\"1\" text:start-value=\"0\"/>\n" \
    "</office:styles>\n");

    /* Automatic style information */
    print_const("<office:automatic-styles>" \
    "   <style:style style:name=\"MMD-Italic\" style:family=\"text\">\n" \
    "      <style:text-properties fo:font-style=\"italic\" style:font-style-asian=\"italic\"\n" \
    "                             style:font-style-complex=\"italic\"/>\n" \
    "   </style:style>\n" \
    "   <style:style style:name=\"MMD-Bold\" style:family=\"text\">\n" \
    "      <style:text-properties fo:font-weight=\"bold\" style:font-weight-asian=\"bold\"\n" \
    "                             style:font-weight-complex=\"bold\"/>\n" \
    "   </style:style>\n" \
    "   <style:style style:name=\"MMD-Superscript\" style:family=\"text\">\n" \
    "      <style:text-properties style:text-position=\"super 58%\"/>\n" \
    "   </style:style>\n" \
    "   <style:style style:name=\"MMD-Subscript\" style:family=\"text\">\n" \
    "      <style:text-properties style:text-position=\"sub 58%\"/>\n" \
    "   </style:style>\n" \
    "   <style:style style:name=\"Strike\" style:family=\"text\">\n" \
    "      <style:text-properties style:text-line-through-style=\"solid\" />\n" \
    "   </style:style>\n" \
    "   <style:style style:name=\"Underline\" style:family=\"text\">\n" \
    "      <style:text-properties style:text-underline-style=\"solid\" style:text-underline-color=\"font-color\"/>\n" \
    "   </style:style>\n" \
    "   <style:style style:name=\"Highlight\" style:family=\"text\">\n" \
    "      <style:text-properties fo:background-color=\"#FFFF00\" />\n" \
    "   </style:style>\n" \
    "   <style:style style:name=\"Comment\" style:family=\"text\">\n" \
    "      <style:text-properties fo:color=\"#0000BB\" />\n" \
    "   </style:style>\n" \
    "<style:style style:name=\"MMD-Table\" style:family=\"paragraph\" style:parent-style-name=\"Standard\">\n" \
    "   <style:paragraph-properties fo:margin-top=\"0in\" fo:margin-bottom=\"0.05in\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"MMD-Table-Center\" style:family=\"paragraph\" style:parent-style-name=\"MMD-Table\">\n" \
    "   <style:paragraph-properties fo:text-align=\"center\" style:justify-single-word=\"false\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"MMD-Table-Right\" style:family=\"paragraph\" style:parent-style-name=\"MMD-Table\">\n" \
    "   <style:paragraph-properties fo:text-align=\"right\" style:justify-single-word=\"false\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"P2\" style:family=\"paragraph\" style:parent-style-name=\"Standard\"\n" \
    "             style:list-style-name=\"L2\">\n" \
    "<style:paragraph-properties fo:text-align=\"start\" style:justify-single-word=\"false\"/>\n" \
    "</style:style>\n" \
	"<style:style style:name=\"fr1\" style:family=\"graphic\" style:parent-style-name=\"Frame\">\n" \
	"   <style:graphic-properties style:print-content=\"true\" style:vertical-pos=\"top\"\n" \
	"                             style:vertical-rel=\"baseline\"\n" \
	"                             fo:padding=\"0in\"\n" \
	"                             fo:border=\"none\"\n" \
	"                             style:shadow=\"none\"/>\n" \
	"</style:style>\n" \
    "<style:style style:name=\"P1\" style:family=\"paragraph\" style:parent-style-name=\"Standard\"\n" \
    "             style:list-style-name=\"L1\"/>\n" \
	"<text:list-style style:name=\"L1\">\n" \
	"	<text:list-level-style-bullet text:level=\"1\" text:style-name=\"Numbering_20_Symbols\" style:num-suffix=\".\" text:bullet-char=\"•\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"0.5in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"0.5in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-bullet>\n" \
	"	<text:list-level-style-bullet text:level=\"2\" text:style-name=\"Numbering_20_Symbols\" style:num-suffix=\".\" text:bullet-char=\"◦\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"0.75in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"0.75in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-bullet>\n" \
	"	<text:list-level-style-bullet text:level=\"3\" text:style-name=\"Numbering_20_Symbols\" style:num-suffix=\".\" text:bullet-char=\"▪\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-bullet>\n" \
	"	<text:list-level-style-number text:level=\"4\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1.25in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1.25in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"5\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1.5in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1.5in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"6\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1.75in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1.75in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"7\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"8\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2.25in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2.25in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"9\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2.5in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2.5in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"10\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2.75in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2.75in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"</text:list-style>\n" \
	"<text:list-style style:name=\"L2\">\n" \
	"	<text:list-level-style-number text:level=\"1\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"0.5in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"0.5in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"2\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"0.75in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"0.75in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"3\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"4\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1.25in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1.25in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"5\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1.5in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1.5in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"6\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1.75in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1.75in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"7\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"8\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2.25in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2.25in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"9\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2.5in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2.5in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"10\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2.75in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2.75in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"</text:list-style>\n" \
    "</office:automatic-styles>\n" \
	" <office:master-styles>\n" \
	"  <style:master-page style:name=\"Endnote\" >\n" \
	"    <style:header><text:h text:outline-level=\"2\">Bibliography</text:h></style:header></style:master-page>\n" \
	"  <style:master-page style:name=\"Footnote\" style:page-layout-name=\"pm2\"/>\n" \
	" </office:master-styles>\n");

	char * result = out->str;
	d_string_free(out, false);
	return result;
}


/// Create full style file for OpenDocument
char * opendocument_style_file(int format) {
	DString * out = d_string_new("");

	char * style = opendocument_style(format);

	// Open
	d_string_append(out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

	print_const("<office:document-styles xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"\n" \
"xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\"\n" \
"xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\"\n" \
"xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\"\n" \
"xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\"\n" \
"xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"\n" \
"xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n" \
"xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n" \
"xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\"\n" \
"xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\"\n" \
"xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\"\n" \
"xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\"\n" \
"xmlns:dr3d=\"urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0\"\n" \
"xmlns:math=\"http://www.w3.org/1998/Math/MathML\"\n" \
"xmlns:form=\"urn:oasis:names:tc:opendocument:xmlns:form:1.0\"\n" \
"xmlns:script=\"urn:oasis:names:tc:opendocument:xmlns:script:1.0\"\n" \
"xmlns:ooo=\"http://openoffice.org/2004/office\"\n" \
"xmlns:ooow=\"http://openoffice.org/2004/writer\"\n" \
"xmlns:oooc=\"http://openoffice.org/2004/calc\"\n" \
"xmlns:dom=\"http://www.w3.org/2001/xml-events\"\n" \
"xmlns:xforms=\"http://www.w3.org/2002/xforms\"\n" \
"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"\n" \
"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n" \
"xmlns:rpt=\"http://openoffice.org/2005/report\"\n" \
"xmlns:of=\"urn:oasis:names:tc:opendocument:xmlns:of:1.2\"\n" \
"xmlns:xhtml=\"http://www.w3.org/1999/xhtml\"\n" \
"xmlns:grddl=\"http://www.w3.org/2003/g/data-view#\"\n" \
"xmlns:officeooo=\"http://openoffice.org/2009/office\"\n" \
"xmlns:tableooo=\"http://openoffice.org/2009/table\"\n" \
"xmlns:drawooo=\"http://openoffice.org/2010/draw\"\n" \
"xmlns:calcext=\"urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0\"\n" \
"xmlns:loext=\"urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0\"\n" \
"xmlns:field=\"urn:openoffice:names:experimental:ooo-ms-interop:xmlns:field:1.0\"\n" \
"xmlns:formx=\"urn:openoffice:names:experimental:ooxml-odf-interop:xmlns:form:1.0\"\n" \
"xmlns:css3t=\"http://www.w3.org/TR/css3-text/\"\n" \
"office:version=\"1.2\">\n");

	// Styles
	d_string_append(out, style);

	// Close
	d_string_append(out, "\n</office:document-styles>");

	// Cleanup
	free(style);
	char * result = out->str;
	d_string_free(out, false);
	return result;
}


/// Create full settings file for OpenDocument
char * opendocument_settings_file(int format) {
	return my_strdup("<?xml version=\"1.0\" encoding=\"utf-8\"?> <office:document-settings xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:config=\"urn:oasis:names:tc:opendocument:xmlns:config:1.0\" xmlns:ooo=\"http://openoffice.org/2004/office\" office:version=\"1.2\"> <office:settings></office:settings></office:document-settings>");
}


#ifdef USE_CURL
// Dynamic buffer for downloading files in memory
// Based on https://curl.haxx.se/libcurl/c/getinmemory.html

struct MemoryStruct {
	char * memory;
	size_t size;
};


static size_t write_memory(void * contents, size_t size, size_t nmemb, void * userp) {
	size_t realsize = size * nmemb;
	struct MemoryStruct * mem = (struct MemoryStruct *)userp;

	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory == NULL) {
		// Out of memory
		fprintf(stderr, "Out of memory\n");
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

// Add assets to zipfile using libcurl
static void add_assets(mz_zip_archive * pZip, mmd_engine * e, const char * directory) {
	asset * a, * a_tmp;

	if (e->asset_hash){
		CURL * curl;
		CURLcode res;
		
		struct MemoryStruct chunk;
		chunk.memory = malloc(1);
		chunk.size = 0;

		char destination[100] = "Pictures/";
		destination[45] = '\0';
		
		mz_bool status;

		curl_global_init(CURL_GLOBAL_ALL);
		curl = curl_easy_init();

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

		HASH_ITER(hh, e->asset_hash, a, a_tmp) {
			curl_easy_setopt(curl, CURLOPT_URL, a->url);
			res = curl_easy_perform(curl);

			memcpy(&destination[9], a->asset_path, 36);

			if (res != CURLE_OK) {
				// Attempt to add asset from local file
				if (!add_asset_from_file(pZip, a, destination, directory)) {
					fprintf(stderr, "Unable to store '%s' in EPUB\n", a->url);
				}
			} else {
				// Store downloaded file in zip
				status = mz_zip_writer_add_mem(pZip, destination, chunk.memory, chunk.size, MZ_BEST_COMPRESSION);

				if (!status) {
					fprintf(stderr, "Error adding asset '%s' to zip as '%s'.\n", a->asset_path, destination);
				}
			}
		}
	}
}

#else
// Add local assets only (libcurl not available)
static void add_assets(mz_zip_archive * pZip, mmd_engine * e, const char * directory) {
	asset * a, * a_tmp;

	if (e->asset_hash){

		char destination[100] = "Pictures/";
		destination[45] = '\0';
		
		mz_bool status;

		HASH_ITER(hh, e->asset_hash, a, a_tmp) {

			memcpy(&destination[9], a->asset_path, 36);

			// Attempt to add asset from local file
			if (!add_asset_from_file(pZip, a, destination, directory)) {
				fprintf(stderr, "Unable to store '%s' in EPUB\n", a->url);
			}
		}
	}
}
#endif


/// Create manifest file for OpenDocument
char * opendocument_manifest_file(mmd_engine * e, int format) {
	DString * out = d_string_new("");

	char * style = opendocument_style(format);

	// Open
	print_const("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	print_const("<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\" manifest:version=\"1.2\">\n");

	// Files
	switch (format) {
		case FORMAT_ODT:
			print_const("\t<manifest:file-entry manifest:full-path=\"/\" manifest:version=\"1.2\" manifest:media-type=\"application/vnd.oasis.opendocument.text\"/>\n");
			break;
	}

	print_const("\t<manifest:file-entry manifest:full-path=\"content.xml\" manifest:media-type=\"text/xml\"/>\n");
	print_const("\t<manifest:file-entry manifest:full-path=\"styles.xml\" manifest:media-type=\"text/xml\"/>\n");
	print_const("\t<manifest:file-entry manifest:full-path=\"settings.xml\" manifest:media-type=\"text/xml\"/>\n");
	print_const("\t<manifest:file-entry manifest:full-path=\"meta.xml\" manifest:media-type=\"text/xml\"/>\n");

	// Add assets
	if (e->asset_hash) {
		asset * a, * a_tmp;

		print_const("\t<manifest:file-entry manifest:full-path=\"Pictures/\" manifest:media-type=\"\"/>\n");

		HASH_ITER(hh, e->asset_hash, a, a_tmp) {
			printf("\t<manifest:file-entry manifest:full-path=\"Pictures/%s\" manifest:media-type=\"image/png\"/>\n", a->asset_path);
		}
	}

	// Close
	print_const("\n</manifest:manifest>");

	// Cleanup
	free(style);
	char * result = out->str;
	d_string_free(out, false);
	return result;
}


/// Create common elements of an OpenDocument zip file
mz_zip_archive * opendocument_core_zip(mmd_engine * e, int format) {
	// Initialize zip archive
	mz_zip_archive * zip = malloc(sizeof(mz_zip_archive));
	zip_new_archive(zip);

	mz_bool status;
	char * data;
	size_t len;

	scratch_pad * scratch = scratch_pad_new(e, format);


	// Add mimetype
	char mime[100];

	switch (format) {
		case FORMAT_ODT:
			strcpy(mime, "application/vnd.oasis.opendocument.text");
			break;
	}

	len = strlen(mime);
	status = mz_zip_writer_add_mem(zip, "mimetype", mime, len, MZ_NO_COMPRESSION);
	if (!status) {
		fprintf(stderr, "Error adding mimetype to zip.\n");
	}


	// Create metadata file
	data = opendocument_metadata_file(e, scratch);
	len = strlen(data);
	status = mz_zip_writer_add_mem(zip, "meta.xml", data, len, MZ_BEST_COMPRESSION);
	free(data);
	if (!status) {
		fprintf(stderr, "Error adding metadata to zip.\n");
	}


	// Create styles file
	data = opendocument_style_file(format);
	len = strlen(data);
	status = mz_zip_writer_add_mem(zip, "styles.xml", data, len, MZ_BEST_COMPRESSION);
	free(data);
	if (!status) {
		fprintf(stderr, "Error adding styles to zip.\n");
	}


	// Create settings file
	data = opendocument_settings_file(format);
	len = strlen(data);
	status = mz_zip_writer_add_mem(zip, "settings.xml", data, len, MZ_BEST_COMPRESSION);
	free(data);
	if (!status) {
		fprintf(stderr, "Error adding settings to zip.\n");
	}


	// Create directories
	status = mz_zip_writer_add_mem(zip, "META-INF/", NULL, 0, MZ_BEST_COMPRESSION);
	if (!status) {
		fprintf(stderr, "Error adding directory to zip.\n");
	}

	status = mz_zip_writer_add_mem(zip, "Pictures/", NULL, 0, MZ_BEST_COMPRESSION);
	if (!status) {
		fprintf(stderr, "Error adding directory to zip.\n");
	}


	// Create manifest file
	data = opendocument_manifest_file(e, format);
	len = strlen(data);
	status = mz_zip_writer_add_mem(zip, "META-INF/manifest.xml", data, len, MZ_BEST_COMPRESSION);
	free(data);
	if (!status) {
		fprintf(stderr, "Error adding manifest to zip.\n");
	}


	// Clean up
	scratch_pad_free(scratch);

	return zip;
}


/// Add shared office:document config
void opendocument_document_attr(DString * out) {
	print_const("xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"\n" \
	"xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\"\n" \
	"xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\"\n" \
	"xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\"\n" \
	"xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\"\n" \
	"xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"\n" \
	"xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n" \
	"xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n" \
	"xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\"\n" \
	"xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\"\n" \
	"xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\"\n" \
	"xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\"\n" \
	"xmlns:dr3d=\"urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0\"\n" \
	"xmlns:math=\"http://www.w3.org/1998/Math/MathML\"\n" \
	"xmlns:form=\"urn:oasis:names:tc:opendocument:xmlns:form:1.0\"\n" \
	"xmlns:script=\"urn:oasis:names:tc:opendocument:xmlns:script:1.0\"\n" \
	"xmlns:ooo=\"http://openoffice.org/2004/office\"\n" \
	"xmlns:ooow=\"http://openoffice.org/2004/writer\"\n" \
	"xmlns:oooc=\"http://openoffice.org/2004/calc\"\n" \
	"xmlns:dom=\"http://www.w3.org/2001/xml-events\"\n" \
	"xmlns:xforms=\"http://www.w3.org/2002/xforms\"\n" \
	"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"\n" \
	"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n" \
	"xmlns:rpt=\"http://openoffice.org/2005/report\"\n" \
	"xmlns:of=\"urn:oasis:names:tc:opendocument:xmlns:of:1.2\"\n" \
	"xmlns:xhtml=\"http://www.w3.org/1999/xhtml\"\n" \
	"xmlns:grddl=\"http://www.w3.org/2003/g/data-view#\"\n" \
	"xmlns:officeooo=\"http://openoffice.org/2009/office\"\n" \
	"xmlns:tableooo=\"http://openoffice.org/2009/table\"\n" \
	"xmlns:drawooo=\"http://openoffice.org/2010/draw\"\n" \
	"xmlns:calcext=\"urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0\"\n" \
	"xmlns:loext=\"urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0\"\n" \
	"xmlns:field=\"urn:openoffice:names:experimental:ooo-ms-interop:xmlns:field:1.0\"\n" \
	"xmlns:formx=\"urn:openoffice:names:experimental:ooxml-odf-interop:xmlns:form:1.0\"\n" \
	"xmlns:css3t=\"http://www.w3.org/TR/css3-text/\"\n" \
	"office:version=\"1.2\"");
}


/// Create full content file
char * opendocument_content_file(const char * body, int format) {
	DString * out = d_string_new("");

	// Open
	print_const("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	print_const("<office:document-content ");
	opendocument_document_attr(out);
	print_const(">\n<office:body>\n");

	switch (format) {
		case FORMAT_ODT:
			print_const("<office:text>\n");
			break;
	}

	// Body
	d_string_append(out, body);

	// Close
	switch (format) {
		case FORMAT_ODT:
			print_const("\n</office:text>");
			break;
	}

	d_string_append(out, "\n</office:body>\n</office:document-content>\n");

	// Cleanup
	char * result = out->str;
	d_string_free(out, false);
	return result;
}


/// Create OpenDocument text file
DString * opendocument_core_flat_create(const char * body, mmd_engine * e, int format) {
	DString * out = d_string_new("");
	char * text;

	print_const("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	print_const("<office:document ");
	opendocument_document_attr(out);

	// Add mimetype
	switch (format) {
		case FORMAT_FODT:
			print_const("\noffice:mimetype=\"application/vnd.oasis.opendocument.text\">\n");
			break;
	}

	scratch_pad * scratch = scratch_pad_new(e, format);

	// Add styles
	text = opendocument_style(format);
	d_string_append(out, text);
	free(text);

	// Add metadata
	text = opendocument_metadata(e, scratch);
	d_string_append(out, text);
	free(text);


	// Add body
	print_const("\n<office:body>\n<office:text>\n");
	d_string_append(out, body);
	print_const("\n</office:text>\n</office:body>\n</office:document>\n");


	// Cleanup
	scratch_pad_free(scratch);

	return out;
}


/// Create OpenDocument zip file version
DString * opendocument_core_file_create(const char * body, mmd_engine * e, const char * directory, int format) {
	DString * result = d_string_new("");

	// Add common core elements
	mz_zip_archive * zip = opendocument_core_zip(e, format);

	mz_bool status;
	char * data;
	size_t len;


	// Create content file
	data = opendocument_content_file(body, format);
	len = strlen(data);
	status = mz_zip_writer_add_mem(zip, "content.xml", data, len, MZ_BEST_COMPRESSION);
	free(data);
	if (!status) {
		fprintf(stderr, "Error adding content.xml to zip.\n");
	}


	// Add image assets
	add_assets(zip, e, directory);


	// Clean up
	free(result->str);

	status = mz_zip_writer_finalize_heap_archive(zip, (void **) &(result->str), (size_t *) &(result->currentStringLength));
	if (!status) {
		fprintf(stderr, "Error finalizing zip archive.\n");
	}

	return result;
}


/// Create OpenDocument flat text file (single xml file)
DString * opendocument_flat_text_create(const char * body, mmd_engine * e, const char * directory) {
	return opendocument_core_flat_create(body, e, FORMAT_FODT);
}


/// Create OpenDocument text file (zipped package)
DString * opendocument_text_create(const char * body, mmd_engine * e, const char * directory) {
	return opendocument_core_file_create(body, e, directory, FORMAT_ODT);
}

