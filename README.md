## About ##

|            |                           |  
| ---------- | ------------------------- |  
| Title:     | MultiMarkdown        |  
| Author:    | Fletcher T. Penney       |  
| Date:      | 2017-03-15 |  
| Copyright: | Copyright © 2016 - 2017 Fletcher T. Penney.    |  
| Version:   | 6.0.0-rc1      |  


## An Announcement! ##

MultiMarkdown v6 is finally here!  It's technically still in "beta" as I would
like to hear back from a few more users to make sure I'm not missing anything,
but it has been subjected to much more rigorous testing than any previous
versions of MultiMarkdown in the past.  If you want more information about
testing, see `DevelopmentNotes`.  It's basically feature complete as a
replacement for MMD v5, and included additional features beyond that.


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

12. EPUB 3 output is supported without need of any external tools.

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
	
		Licensed under BSD Revised license
	
	miniz library:
		Copyright 2013-2014 RAD Game Tools and Valve Software
		Copyright 2010-2014 Rich Geldreich and Tenacious Software LLC
	
		Licensed under the MIT license
	
	
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
	
