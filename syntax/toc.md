Title:	Table of Contents  
Author:	Fletcher T. Penney  
Revised:	2018-06-27  
Base Header Level:	2  
CSS:	https://fletcher.github.io/MultiMarkdown-6/css/document.css  
MMD Header:	{{TOC}}  
MMD Footer:	{{../links.txt}}  
LaTeX Config:	tufte-handout  


## Table of Contents ##

As of version 4.7, MultiMarkdown supports the use of `{{TOC}}` to insert a Table of Contents in the document.  This is automatically generated from the headers included in the document.

When possible, MultiMarkdown uses the "native" TOC for a given output format.  For example, `\tableofcontents` when exporting to LaTeX.

As of version 6.5, MultiMarkdown also supports two additional versions:

*	`{{TOC:2}}` -- this limits the TOC to "level 2" entries
*	`{{TOC:2-3}}` -- this limts the TOC to levels 2 and 3
