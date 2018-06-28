Title:	Abbreviations  
Author:	Fletcher T. Penney  
Revised:	2018-06-27  
Base Header Level:	2  
CSS:	https://fletcher.github.io/MultiMarkdown-6/css/document.css  
MMD Header:	{{TOC}}  
MMD Footer:	{{../links.txt}}  
LaTeX Config:	tufte-handout  


## Abbreviations (or Acronyms)##

**NOTE**: The syntax for abbreviations changed in MMD v6. 

Abbreviations can be specified using inline or reference syntax.  The inline variant requires that the abbreviation be wrapped in parentheses and immediately follows the `>`. 

	[>MMD] is an abbreviation.  So is [>(MD) Markdown].

	[>MMD]: MultiMarkdown


There is also a "shortcut" method for abbreviations that is similar to the approach used in prior versions of MMD.  You specify the definition for the abbreviation in the usual manner, but MMD will automatically identify each instance where the abbreviation is used and substitute it automatically.  In this case, the abbreviation is limited to a more basic character set which includes letters, numbers, periods, and hyphens, but not much else.  For more complex abbreviations, you must explicitly mark uses of the abbreviation. 

There are a few limitations:

*	The full name of the abbreviation is plain text only -- no MultiMarkdown markup will be processed.

*	When exporting to LaTeX, the `acronym` package is used; this means that the first usage will result in `full text (short)`, and subsequent uses will result in `short`.