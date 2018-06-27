Title:	Raw Source  
Author:	Fletcher T. Penney  
Revised:	2018-06-27  
Base Header Level:	2  
CSS:	https://fletcher.github.io/MultiMarkdown-6/css/document.css  
MMD Footer:	{{../links.txt}}  
LaTeX Config:	tufte-handout  


## Raw Source ##

In older versions of MultiMarkdown you could use an HTML comment to pass raw LaTeX or other content to the final document.  This worked reasonably well, but was limited and didn't work well when exporting to multiple formats.  It was time for something new.

MMD  v6 offers a new feature to handle this.  Code spans and code blocks can be flagged as representing raw source:


	foo `*bar*`{=html}
	
	```{=latex}
	*foo*
	```


The contents of the span/block will be passed through unchanged.

You can specify which output format is compatible with the specified source:

*	`html`
*	`odt`
*	`epub`
*	`latex`
*	`*` -- wildcard matches any output format

