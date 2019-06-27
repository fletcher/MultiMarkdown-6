Title:	Escaped Newlines  
Author:	Fletcher T. Penney  
Revised:	2018-06-30  
Base Header Level:	2  
CSS:	https://fletcher.github.io/MultiMarkdown-6/css/document.css  
MMD Header:	{{TOC}}  
MMD Footer:	{{../links.txt}}  
LaTeX Config:	tufte-handout  
HTML header:	<script src="https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.2/MathJax.js?config=TeX-AMS-MML_HTMLorMML"></script>
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.12.0/styles/default.min.css">
<script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.12.0/highlight.min.js"></script>
<script>hljs.initHighlightingOnLoad();</script>


## Escaped newlines ##

Thanks to a contribution from [Nicolas](https://github.com/njmsdk), MultiMarkdown has an additional syntax to indicate a line break.  The usual approach for Markdown is "space-space-newline" --- two spaces at the end of the line.  For some users, this causes problems:

*	the trailing spaces are typically invisible when glancing at the source, making it easy to overlook them

*	some users' text editors modify trailing space (IMHO, the proper fix for this is a new text editor...)

Nicolas submitted a patch that enables a new option that interprets "\\" before a newline as a marker that a line break should be used:

	This is a line.\
	This is a new line.

