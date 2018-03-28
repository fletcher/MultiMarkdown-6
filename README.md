## About ##

|            |                           |  
| ---------- | ------------------------- |  
| Title:     | MultiMarkdown        |  
| Author:    | Fletcher T. Penney       |  
| Date:      | 2018-03-27 |  
| Copyright: | Copyright © 2016 - 2018 Fletcher T. Penney.    |  
| Version:   | 6.3.1      |  

master branch: [![Build Status](https://travis-ci.org/fletcher/MultiMarkdown-6.svg?branch=master)](https://travis-ci.org/fletcher/MultiMarkdown-6)  
develop branch: [![Build Status](https://travis-ci.org/fletcher/MultiMarkdown-6.svg?branch=develop)](https://travis-ci.org/fletcher/MultiMarkdown-6)

## An Announcement! ##

MultiMarkdown v6 is finally here!  If you want more information about
testing, see `DevelopmentNotes`.  

If you want to more know about the differences from v5, see the `QuickStart`
guide.


## Obtaining MultiMarkdown ##

You can download the latest installer for MacOS or Windows at Github:

<https://github.com/fletcher/MultiMarkdown-6/releases>

To build from source, download from Github.  Then:

	make release
	(OR)
	make debug

	cd build
	make

You can optionally test using the test suite:

	ctest


## Differences in the MultiMarkdown Syntax ##

MultiMarkdown v6 is mostly about making a better MMD parser, but it involves a
few changes to the MultiMarkdown syntax itself.

1. Setext headers can consist of more than one line to be included in the
header:

		This is
		a header
		========

2. Whitespace is not allowed between the text brackets and label brackets in
reference links, images, footnotes, etc.  For example `[foo] [bar]` will no
longer be the same as `[foo][bar]`.

3. Link and image titles can be quoted with `'foo'`, `"foo"`, or `(foo)`.
Link attributes can be used in both reference and inline links/images.

4. HTML elements are handled slightly differently.  There is no longer a
`markdown="1"` feature.  Instead, HTML elements that are on a line by
themselves will open an HTML block that will cause the rest of the "paragraph"
to be treated as HTML such that Markdown will not be parsed in side of it.
HTML block-level tags are even "stronger" at starting an HTML block.  It is
not quite as complex as the approach used in CommonMark, but is similar under
most circumstances.  Leaving a blank line after the opening tag will allow
MultiMarkdown parsing inside of the HTML block.

	For example, this would not be parsed:

		<div>
		*foo*
		</div>

	But this would be:

		<div>

		*foo*

		</div>

5. "Malformed" reference link definitions are handled slightly differently.
For example, the test suite file `Reference Footnotes.text` is parsed
differently in compatibility mode than MMD-5.  This started as a side-effect
of the parsing algorithm, but I actually think it makes sense.  This may or
may not change in the future.

6. Table captions in MMD-6 must come immediately *after* the table, not
before it.

7. Escaped linebreaks (`\` preceding a line break) will be interpreted as
`<br />` (even in compatibility mode).  This was previously an optional
feature in MMD, but I don't see a problem with just making it default 
behavior.

8. Escaped spaces (`\ `) will be interpreted as a non-breaking space, if the
output format supports it.

9. CriticMarkup, Abbreviations, Glossary Terms, and Citations are handled
slightly differently.  See the QuickStart guide for more information.

10. Fenced code blocks can use leading/trailing "fences" of 3, 4, or 5
backticks in length.  That should be sufficient for complex documents without
requiring a more complex parser.  If there is no trailing fence, then the
fenced block is considered to go through the end of the document.

11. Emph and Strong parsing is conceptually the same, but the implementation
is different.  It is designed for speed, accuracy, and consistency.  In
general, it seems to handle edge cases much more reliably, but there are still
a couple of situations that I would like to take into account, if possible.
These are not situations that should occur often in "real life."

12. EPUB 3 output is supported without need of any external tools (unless you
want to support downloading remote images/files in which case `libcurl` is
used.

13. Internationalization support for HTML phrases, such as "see footnote". See
[Github](https://github.com/fletcher/MultiMarkdown-6/issues/37) for more
information.



## License ##

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
	
