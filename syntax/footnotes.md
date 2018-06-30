Title:	Footnotes  
Author:	Fletcher T. Penney  
Revised:	2018-06-30  
Base Header Level:	2  
CSS:	https://fletcher.github.io/MultiMarkdown-6/css/document.css  
MMD Header:	{{TOC}}  
MMD Footer:	{{../links.txt}}  
LaTeX Config:	tufte-handout  
HTML header:	<script type="text/javascript" src="https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.2/MathJax.js?config=TeX-AMS-MML_HTMLorMML"></script>
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.12.0/styles/default.min.css">
<script type="text/javascript" src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.12.0/highlight.min.js"></script>
<script>hljs.initHighlightingOnLoad();</script>


## Footnotes ##

I have added support for footnotes to MultiMarkdown, using the syntax proposed by John Gruber.  Unfortunately, he never implemented footnotes in Markdown. 

To create a footnote, enter something like the following:

    Here is some text containing a footnote.[^somesamplefootnote]
	
    [^somesamplefootnote]: Here is the text of the footnote itself.
	
    [somelink]:http://somelink.com


The footnote itself must be at the start of a line, just like links by reference. If you want a footnote to have multiple paragraphs, lists, etc., then the subsequent paragraphs need an extra tab preceding them. You may have to experiment to get this just right, and please let me know of any issues you find. 

This is what the final result looks like:

> Here is some text containing a footnote.[^somesamplefootnote]

[^somesamplefootnote]: Here is the text of the footnote itself.

You can also use "inline footnotes":

	Here is another footnote.[^This is the footnote itself]
