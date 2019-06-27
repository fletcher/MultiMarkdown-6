Title:	Tables  
Author:	Fletcher T. Penney  
Revised:	2018-06-30   
Base Header Level:	2  
CSS:	https://fletcher.github.io/MultiMarkdown-6/css/document.css  
MMD Header:	{{TOC}}  
MMD Footer:	{{../links.txt}}  
transcluyde base:	.
LaTeX Config:	tufte-handout  
HTML header:	<script src="https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.2/MathJax.js?config=TeX-AMS-MML_HTMLorMML"></script>
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.12.0/styles/default.min.css">
<script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.12.0/highlight.min.js"></script>
<script>hljs.initHighlightingOnLoad();</script>


## Tables ##

### Table Basics ###

MultiMarkdown has a special syntax for creating tables.  It is generally compatible with the syntax used by Michael Fortin for [PHP Markdown Extra](http://www.michelf.com/projects/php-markdown/extra/)

Basically, it allows you to turn:

``` 
{{../examples/table.text}}
```

into the following [table][Prototype table].

{{../examples/table.text}}


### Table Rules ###

The requirements are:

*	There must be at least one `|` per line

*	The "separator" line between  headers and table content must contain only `|`,`-`, `=`, `:`,`.`, `+`, or spaces

*	Cell content must be on one line only

*	Columns are separated by `|`

*	The first line of the table, and the alignment/divider line, must start at 
	the beginning of the line

Other notes:

*	It is optional whether you have `|` characters at the beginning and end of lines. 

*	The "separator" line uses `----` or `====` to indicate the line between a header and cell.  The length of the line doesn't matter, but must have at least one character per cell. 

*	To set alignment, you can use a colon to designate left or right alignment, or a colon at each end to designate center alignment, as above. If no colon is present, the default alignment of your system is selected (left in most cases).  If the separator line ends with `+`, then cells in that column will be wrapped when exporting to LaTeX if they are long enough. 

*	To indicate that a cell should span multiple columns, then simply add additional pipes (`|`) at the end of the cell, as shown in the example. If the cell in question is at the end of the row, then of course that means that pipes are not optional at the end of that row....  The number of pipes equals the number of columns the cell should span. 

*	You can use normal Markdown markup within the table cells. 

*	Captions are optional, but if present must be at the beginning of the line immediately following the table, start with `[`, and end with `]`. If you have a caption before and after the table, only the first match will be used. 

*	If you have a caption, you can also have a label, allowing you to create  anchors pointing to the table. If there is no label, then the caption acts  as the label 

*	Cells can be empty. 

*	You can create multiple `<tbody>` tags (for HTML) within a table by having a **single** empty line between rows of the table. This allows your CSS to place horizontal borders to emphasize different sections of the table.  This feature doesn't work in all output formats (e.g. RTF and OpenDocument). 



### Limitations of Tables ###

* MultiMarkdown table support is designed to handle *most* tables for *most* people; it doesn't cover *all* tables for *all* people.  If you need complex tables you will need to create them by hand or with a tool specifically designed for your output format.  At some point, however, you should consider whether a table is really the best approach if you find MultiMarkdown tables too limiting.

* Native RTF support for tables is *very* limited.  If you need more complex tables, I recommend using the OpenDocument format, and then using [LibreOffice] to convert your document to RTF.
