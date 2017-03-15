## About ##

|            |                           |  
| ---------- | ------------------------- |  
| Title:     | MultiMarkdown        |  
| Author:    | Fletcher T. Penney       |  
| Date:      | 2017-03-13 |  
| Copyright: | Copyright © 2016 - 2017 Fletcher T. Penney.    |  
| Version:   | 6.0.0-b2      |  


## Updates ##

* 2017-03-13 -- v 6.0.0-b2:

	*    ADDED: Add CriticMarkup preprocessor that works across empty lines when accepting/rejecting markup
	*    ADDED: Add back the mmd6 latex title file
	*    ADDED: Basic EPUB 3 support -- uses 'miniz' library to zip creation
	*    ADDED: Update QuickStart and EPUB code
	*    CHANGED: Update QuickStart guide
	*    CHANGED: Update test suite
	*    FIXED: Don't duplicate LaTeX glossary definitions
	*    FIXED: Fix abbreviations in ODF; Improve test suite
	*    FIXED: Improve glossaries and abbreviations; Update QuickStart
	*    FIXED: Tidy up some compiler warnings in code
	*    FIXED: Use custom UUID code to minimize external dependencies


* 2017-03-09 -- v 6.0.0-b1:

	*    ADDED: Add French translations; fix typo in German
	*    ADDED: Add Quick Start guide
	*    ADDED: Add functionality to automatically identify abbreviations and glossary terms in source
	*    ADDED: Improve LaTeX configuration files
	*    ADDED: Update German translations
	*    ADDED: Use native ODF table of contents instead of a manual list
	*    ADDED: Use native command for table of contents in LaTeX
	*    CHANGED: Bring HTML and ODF into line with LaTeX as to output of abbreviatinos on first and subsequent uses
	*    CHANGED: Slight performance tweak
	*    CHANGED: Update German test suite
	*    FIXED: Allow {{TOC}} in latex verbatim
	*    FIXED: Don't free token_pool if never initialized
	*    FIXED: Fix German typo
	*    FIXED: Fix missing token type
	*    FIXED: Improve performance of checking document for metadata, which improves performance when checking for possible transclusion
	*    FIXED: Update test suite for abbreviation changes


* 2017-03-05 -- v 0.4.2-b:

	*    ADDED: Add  and  utility functions; fix memory leak
	*    ADDED: Initial abbreviation support
	*    ADDED: Keep working on Abbreviations/Glossaries
	*    ADDED: Refactor abbreviation code; Add inline abbreviations; Fix abbreviations in ODF
	*    ADDED: Update Inline Footnote test
	*    CHANGED: Add comments to i18n.h
	*    CHANGED: Finish refactoring note-related code
	*    CHANGED: Refactor footnotes
	*    CHANGED: Refactor glossary code
	*    CHANGED: Remove offset from html export functions
	*    FIXED: latex list items need to block optional argument to allow '[' as first character
	*    Merge branch 'release/0.4.1-b' into develop


* 2017-03-04 -- v 0.4.1-b:

	*    FIXED: Add glossary localization


* 2017-03-04 -- v 0.4.0-b:

	*    ADDED: Add TOC support to ODF
	*    ADDED: Add glossary support to ODF
	*    ADDED: Add prelim code for handling abbreviations
	*    ADDED: Add support for Swift Package Maker; CHANGED: Restructure source directory
	*    ADDED: Added LaTeX support for escaped characters, fenced code blocks, images, links
	*    ADDED: Basic ODF Support
	*    ADDED: Better document strong/emph algorithm
	*    ADDED: Continue ODF progress
	*    ADDED: Continue to work on ODF export
	*    ADDED: Continue work on ODF
	*    ADDED: Finish ODF support for lists
	*    ADDED: Improve performance when exporting
	*    ADDED: Improve token_pool memory handling
	*    ADDED: Prototype support for Glossaries
	*    ADDED: Support 'latexconfig' metadata
	*    CHANGED: Use multiple cases in glossary tests
	*    FIXED: Don't force glossary terms into lowercase
	*    FIXED: Fix Makefile for new source file location
	*    FIXED: Fix algorithm for creating TOC to properly handle 'incorrect' levels
	*    FIXED: Fix linebreaks in LaTeX; ADDED: Add Linebreaks test file
	*    FIXED: Fix new_source script for new directory structure
	*    FIXED: Fix non-breaking space in ODF
	*    FIXED: Fix padding at end of document body in ODF
	*    FIXED: Fix underscores in raw latex
	*    FIXED: Potential bug
	*    NOTE: Add shared library build option


* 2017-02-17 -- v 0.3.1.a:

	*    ADDED: 'finalize' beamer support
	*    ADDED: Add escaped newline as linebreak; start on beamer/memoir support
	*    ADDED: CriticMarkup test for LaTeX
	*    ADDED: Custom LaTeX output for CriticMarkup comments
	*    ADDED: Support mmd export format
	*    ADDED: Work on cpack installer -- change project name for compatibility
	*    CHANGED: Adjust latex metadata configuration for consistency
	*    CHANGED: Configure cmake to use C99
	*    FIXED: Add custom  implementation for cross-platform support
	*    FIXED: Fix German HTML tests
	*    FIXED: Fix cpack destination directory issue
	*    FIXED: Fix memory leaks etc
	*    FIXED: Fix warning in custom vasprintf
	*    FIXED: Modify CMakeLists.txt to test for use of clang compiler
	*    FIXED: Work on memory leaks
	*    NOTE: Adjust license width to improve display on smaller terminal windows


* 2017-02-14 -- v 0.3.0a:

	*    ADDED: Add basic image support to LaTeX
	*    ADDED: Add file transclusion
	*    ADDED: Add support for citation 'locators'
	*    ADDED: Add support for manual labels on ATX Headers
	*    ADDED: Add support for manual labels on Setext Headers
	*    ADDED: Add support for tables in LaTeX
	*    ADDED: HTML Comments appear as raw LaTeX
	*    ADDED: Improved citation support in LaTeX
	*    ADDED: Support \autoref{} in LaTeX
	*    ADDED: Support combined options in LaTeX citations that use the '\]\[' syntax
	*    ADDED: Support language specifier in fenced code blocks
	*    ADDED: Support metadata in LaTeX
	*    ADDED: Update Citations test suite
	*    FIXED: Escaped LaTeX characters
	*    FIXED: Fix bug in URL parsing
	*    FIXED: Fix bug in citation links
	*    FIXED: Fix bug when no closing divider or newline at end of last table cell
	*    FIXED: Fix issue printing '-'
	*    FIXED: Fix scan_url test suite
	*    FIXED: Get Math working in LaTeX
	*    FIXED: Improve reliability or link scanner
	*    FIXED: Properly add id attribute to new instances of citation only
	*    FIXED: Properly handle manual labels with TOC
	*    FIXED: Properly print hash characters in LaTeX
	*    FIXED: Separate LaTeX verbatim and texttt character handling
	*    FIXED: Update Escapes test LaTeX result
	*    FIXED: Work on escaping LaTeX characters


* 2017-02-08 -- v 0.1.4a:

	*    ADDED: Add smart quote support for other languages (resolves #15)


* 2017-02-08 -- v 0.1.3a:

	*    ADDED: Add support for reference image id attributes
	*    ADDED: Add support for table captions
	*    ADDED: Metadata support for base header level
	*    ADDED: Support distinction between 3 and 5 backticks in fenced code blocks
	*    ADDED: Support Setext headers
	*    FIXED: Fix issue with metadata disrupting smart quotes

* 2017-02-07 --  v 0.1.2a:

	* "pathologic" test suite -- fix handling of nested brackets, e.g.
	`[[[[foo]]]]` to avoid bogging down checking for reference links that
	don't exist.  
	* Table support -- a single blank line separates sections of tables, so
	at least two blank lines are needed between adjacent tables.
	* Definition list support
	* "fuzz testing" -- stress test the parser for unexpected failures
	* Table of Contents support
	* Improved compatibility mode parsing

* 2017-01-28 -- v 0.1.1a includes a few updates:

	* Metadata support
	* Metadata variables support
	* Extended ASCII range character checking
	* Rudimentary language translations, including German
	* Improved performance
	* Additional testing:
		* CriticMarkup
		* HTML Blokcs
		* Metadata/Variables
		* "pathologic" test cases from CommonMark


## An Announcement! ##

I would like to officially announce that MultiMarkdown version 6 is in public
alpha.  It's finally at a point where it is usable, but there are quite a few
caveats.

This post is a way for me to organize some of my thoughts, provide some
history for those who are interested, and to provide some tips and tricks from
my experiences for those who are working on their own products.

But first, some background...



## Differences in MultiMarkdown Itself ##

MultiMarkdown v6 is mostly about making a better MMD parser, but it will
likely involve a few changes to the MultiMarkdown language itself.


1. {--I am thinking about removing Setext headers from the language.  I almost
never use them, much preferring to use ATX style headers (`# foo #`).
Additionally, I have never liked the fact that Setext headers allow the
meaning of a line to be completely changed by the following line.  It makes
the parsing slightly more difficult on a technical level (requiring some
backtracking at times).  I'm not 100% certain on this, but right now I believe
it's the only Markdown feature that doesn't exist in MMD 6 yet.--}{++I decided
to go ahead and implement Setext headers, as it can be done with the new
parser without backtracking.  One difference with older versions of MMD, as
well as Markdown itself, is that a setext header can consist of more than one
line to be included in the header.++}

2. Whitespace is not allowed between the text brackets and label brackets in
reference links, images, footnotes, etc.  For example `[foo] [bar]` will no
longer be the same as `[foo][bar]`.

3. Link and image titles can be quoted with `'foo'`, `"foo"`, or `(foo)`.

4. HTML elements are handled slightly differently.  There is no longer a
`markdown="1"` feature.  Instead, HTML elements that are on a line by
themselves will open an HTML block that will cause the rest of the "paragraph"
to be treated as HTML such that Markdown will not be parsed in side of it.
HTML block-level tags are even "stronger" at starting an HTML block.  It is
not quite as complex as the approach used in CommonMark, but is similar under
most circumstances.

	For example, this would not be parsed:

		<div>
		*foo*
		</div>

	But this would be:

		<div>

		*foo*

		</div>

5. "Malformed" reference link definitions are handled slightly differently.
For example, `Reference Footnotes.text` is parsed differently in compatibility
mode than MMD-5.  This started as a side-effect of the parsing algorithm, but
I actually think it makes sense.  This may or may not change in the future.

6. Table captions in MMD-6 must come immediately *after* the table, not
before it.

7. Escaped linebreaks (`\` preceding a line break) will be interpreted as
`<br />` (even in compatibility mode).  This was previously an optional
feature in MMD, but I don't see a problem with just making it default 
behavior.


## Where Does MultiMarkdown 6 Stand? ##


### Features ###

I *think* that all basic Markdown features have been implemented.
Additionally, the following MultiMarkdown features have been implemented:

* Automatic cross-reference targets
* Basic Citation support
* CriticMarkup support
* Definition lists
* Figures
* Footnotes
* Inline and reference footnotes
* Image and Link attributes (attributes can now be used with inline links as
	well as reference links)
* Math support
* Smart quotes (support for languages other than english is not fully
	implemented yet)
* Superscripts/subscripts
* Table of Contents
* Tables


Things that are partially completed:

* Citations -- still need:
	* Syntax for "not cited" entries
	* Output format
	* HTML --> separate footnotes and citations?
	* Locators required?
* CriticMarkup -- need to decide:
	* How to handle CM stretches that include blank lines
* Fenced code blocks
* Headers -- need support for manual labels
* Metadata
* Full/Snippet modes


Things yet to be completed:

* Abbreviations
* Glossaries
* File Transclusion


### Accuracy ###

MultiMarkdown v6 successfully parses the Markdown [syntax page], except for
the Setext header at the top.  It passes the 29 test files currently in place.
There are a few at

[syntax page]: https://daringfireball.net/projects/markdown/syntax



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
	
