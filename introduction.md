Title:	Introduction
Author:	Fletcher T. Penney  
Revised:	2018-10-08  
Base Header Level:	2  
CSS:	https://fletcher.github.io/MultiMarkdown-6/css/document.css  
MMD Header:	{{TOC}}  
MMD Footer:	{{links.txt}}
LaTeX Config:	tufte-book  
HTML header:	<script src="https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.2/MathJax.js?config=TeX-AMS-MML_HTMLorMML"></script>
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.12.0/styles/default.min.css">
<script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.12.0/highlight.min.js"></script>
<script>hljs.initHighlightingOnLoad();</script>


# Introduction #

MultiMarkdown is a superset of the [Markdown] lightweight markup syntax with support for additional output formats and features.


## What is Markdown? ##

To understand what MultiMarkdown is, you first should be familiar with
[Markdown]. The best description of what Markdown is comes from John Gruber's
Markdown web site:

> Markdown is a text-to-HTML conversion tool for web writers. Markdown
> allows you to write using an easy-to-read, easy-to-write plain text
> format, then convert it to structurally valid XHTML (or HTML).

> Thus, "Markdown" is two things: (1) a plain text formatting
> syntax; and (2) a software tool, written in Perl, that converts
> the plain text formatting to HTML. See the Syntax page for details
> pertaining to Markdown's formatting syntax. You can try it out,
> right now, using the online Dingus.

> The overriding design goal for Markdown's formatting syntax is to
> make it as readable as possible. The idea is that a Markdown-formatted
> document should be publishable as-is, as plain text, without looking
> like it's been marked up with tags or formatting instructions. While
> Markdown's syntax has been influenced by several existing
> text-to-HTML filters, the single biggest source of inspiration for
> Markdown's syntax is the format of plain text email. --- [John Gruber][Markdown]


## What is MultiMarkdown? ##

Markdown is great, but it lacked a few features that would allow it to work with entire documents, rather than just pieces of a web page. 

I wrote MultiMarkdown in order to leverage Markdown's syntax, but to extend it to work with complete documents that could ultimately be converted from text into other formats, including complete HTML documents, LaTeX, PDF, and ODF. 

In addition to the ability to work with complete documents and conversion to formats beyond HTML, the Markdown syntax was lacking a few other things. Michel Fortin added a few additional syntax features when writing [PHP Markdown Extra][]. Some of his ideas were implemented and expanded on in MultiMarkdown, including tables, footnotes, citation support, image and link attributes, cross-references, math support, and more. 

John Gruber may disagree with me, but I really did try to stick with his proclaimed vision whenever I added a new syntax format to MultiMarkdown. The quality that attracted me to Markdown the most was its clean format. Reading a plain text document written in Markdown is *easy*. It makes sense, and it looks like it was designed for people, not computers. To the extent possible, I tried to keep this same concept in mind when working on MultiMarkdown. 

I may or may not have succeeded in this....

In the vein of Markdown's multiple definitions, you can think of MultiMarkdown as: 

1.	A program to convert plain text to a fully formatted document. 

2.	The syntax used in the plain text to describe how to convert it to a complete document. 


## Why should I use MultiMarkdown? ##

Writing with MultiMarkdown allows you to separate the content and structure of your document from the formatting. You focus on the actual writing, without having to worry about making the styles of your chapter headers match, or ensuring the proper spacing between paragraphs. And with a little forethought, a single plain text document can easily be converted into multiple output formats without having to rewrite the entire thing or format it by hand. Even better, you don't have to write in "computer-ese" to create well formatted HTML or LaTeX commands. You just write, MultiMarkdown takes care of the rest. 

For example, instead of writing:

	<p>In order to create valid 
	<a href="http://en.wikipedia.org/wiki/HTML">HTML</a>, you 
	need properly coded syntax that can be cumbersome for 
	&#8220;non-programmers&#8221; to write. Sometimes, you
	just want to easily make certain words <strong>bold
	</strong>, and certain words <em>italicized</em> without
	having to remember the syntax. Additionally, for example,
	creating lists:</p>

	<ul>
	<li>should be easy</li>
	<li>should not involve programming</li>
	</ul>

You simply write:

	In order to create valid [HTML], you need properly
	coded syntax that can be cumbersome for 
	"non-programmers" to write. Sometimes, you just want
	to easily make certain words **bold**, and certain 
	words *italicized* without having to remember the 
	syntax. Additionally, for example, creating lists:

	* should be easy
	* should not involve programming

	[HTML]: http://en.wikipedia.org/wiki/HTML


Additionally, you can write a MultiMarkdown document in any text editor, on any operating system, and know that it will be compatible with MultiMarkdown on any other operating system and processed into the same output. As a plain text format, your documents will be safe no matter how many times you switch computers, operating systems, or favorite applications.  You will always be able to open and edit your documents, even when the version of the software you originally wrote them in is long gone. 

These features have prompted several people to use MultiMarkdown in the process of writing their books, theses, and countless other documents. 

There are many other reasons to use MultiMarkdown, but I won't get into all of them here. 

*By the way* --- the MultiMarkdown web site is, of course, created using MultiMarkdown. To view the MMD source for any page, add `.txt` to the end of the URL. If the URL ends with `/`, then add `index.txt` to the end instead. The main MultiMarkdown page, for example, would be <http://fletcherpenney.net/multimarkdown/index.txt>. 


## What Are the Different Versions of MultiMarkdown? ##

The first real version of MultiMarkdown was version 2.  It was a modification of the original `Markdown.pl` script.  It worked fine, but was slow when parsing longer documents.  The plain text was converted to HTML, and then XSLT was used to convert the HTML to other formats (primarily LaTeX).  Over time, maintaining the complicated nest of regular expressions became more difficult, and a better approach was needed.

[MultiMarkdown 3][peg-multimarkdown] (aka `peg-multimarkdown`) was built using John MacFarlane's [peg-markdown] as a base.  It was *much* faster than version 2, and the underlying PEG (parsing expression grammar) made things more reliable.  There were still issues and limitations (some inherited from peg-markdown, but most were my errors), which lead to the development of version 4.

[MultiMarkdown 4][MMD-4] was a complete rewrite, keeping only the PEG and a few utility routines from MMD v3.  This release fixed memory leaks and other problems from earlier MMD releases; it is safe to use in multithreaded applications and adds many new features.

[MultiMarkdown 5][MMD-5] was mostly a restructuring of version 4, followed by further incremental improvements.

[MultiMarkdown 6][MMD-6] was a complete rewrite from the ground up. The primary goals were:

*	Improved performance -- v6 uses a parser that was largely written by hand, combined with a few pieces that are generated by [lemon](http://www.hwaci.com/sw/lemon/).  This is vastly faster than the PEG parser of versions 3-5.  There is probably still room to improve the code, but v6 is now almost as fast as the fastest Markdown parsers out there, *and* provides more features.

*	Improved accuracy and consistency -- v6 uses an entirely new test suite in order to ensure more consistent parsing across various edge cases.

*	New features -- several features were added to v6, and several were completely restructured to provide various improvements.

*	The v6 [QuickStart guide](https://github.com/fletcher/MultiMarkdown-6/tree/master/QuickStart) documents some of the changes in this latest iteration.


## Where is this Guide Kept? ##

This guide has been rewritten with the following changes:

*	The source is now in the `gh_pages` branch of the [MultiMarkdown project][github].  You can submit changes as a pull request, or by writing me.

*	You can access this information on the web at <http://fletcher.github.io/MultiMarkdown-5>

*	The source itself is a collection of MultiMarkdown text documents that use the transclusion features to create a master document from the individual source files.  These documents can be viewed in the browser as HTML, or downloaded as PDF or OpenDocument files.

