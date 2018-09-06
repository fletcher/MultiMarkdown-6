/**

	MultiMarkdown -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file opml-lexer.c

	@brief Tokenize OPML file for parsing


	@author	Fletcher T. Penney
	@bug	

**/

/*

	Copyright © 2016 - 2018 Fletcher T. Penney.


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

#include <stdlib.h>

#include "opml-lexer.h"
#include "opml-parser.h"


// Basic scanner struct

#define YYCTYPE		unsigned char
#define YYCURSOR	s->cur
#define YYMARKER	s->ptr
#define YYCTXMARKER	s->ctx

int opml_scan(Scanner * s, const char * stop) {

	scan:

	if (s->cur >= stop) {
		return 0;
	}

	s->start = s->cur;

	/*!re2c
		re2c:yyfill:enable = 0;

		NL														= "\r\n" | '\n' | '\r';
		WS														= [ \t]+;
		WSNL													= (NL | WS)+;

		EQUAL													= '=';
		
		double_quoted											= '"' [^"\x00]* '"';

		text_attribute											= WSNL* 'text' WSNL* EQUAL WSNL*;
		note_attribute											= WSNL* '_note' WSNL* EQUAL WSNL*;

		contains_newline										= "&#10;" | "&#13;";

		'<?xml' [^>\x00]* '>'									{ return OPML_XML; }

		'<opml' [^>\x00]* '>'									{ return OPML_OPML_OPEN; }
		'</opml>'												{ return OPML_OPML_CLOSE; }

		'<head' [^>\x00]* '>'									{ return OPML_HEAD_OPEN; }
		'</head>'												{ return OPML_HEAD_CLOSE; }

		'<title' [^>\x00]* '>'									{ return OPML_TITLE_OPEN; }
		'</title>'												{ return OPML_TITLE_CLOSE; }

		'<body' [^>\x00]* '>'									{ return OPML_BODY_OPEN; }
		'</body>'												{ return OPML_BODY_CLOSE; }

		'<outline' text_attribute '"&gt;&gt;Preamble&lt;&lt;"' note_attribute double_quoted WSNL* '>'		{ return OPML_OUTLINE_PREAMBLE; }
		'<outline' text_attribute '"&gt;&gt;Metadata&lt;&lt;"' WSNL* '>'	{ return OPML_OUTLINE_METADATA; }


		'<outline' [^>\x00]* '/>'								{ return OPML_OUTLINE_SELF_CLOSE; }
		'<outline' [^>\x00]* '>'								{ return OPML_OUTLINE_OPEN; }
		'</outline>'											{ return OPML_OUTLINE_CLOSE; }

		WSNL													{ return OPML_WSNL; }

		// Skip over anything else - '.' does not include '\n'
		*														{ goto scan; }
	*/
}
