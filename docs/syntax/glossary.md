Title:	Glossaries  
Author:	Fletcher T. Penney  
Revised:	2018-06-27  
Base Header Level:	2  
CSS:	https://fletcher.github.io/MultiMarkdown-6/css/document.css  
MMD Header:	{{TOC}}  
MMD Footer:	{{../links.txt}}  
LaTeX Config:	tufte-handout  


## Glossaries ##

MultiMarkdown has a feature that allows footnotes to be specified as glossary terms. It doesn't do much for XHTML documents, but the XSLT file that converts the document into LaTeX is designed to convert these special footnotes into glossary entries. 

**NOTE**: The syntax for glossary terms changed in MMD v6. 


If there are terms in your document you wish to define in a [?(glossary) The glossary collects information about important terms used in your document] at the end of your document, you can define them using the glossary syntax. 

Glossary terms can be specified using inline or reference syntax. The inline variant requires that the abbreviation be wrapped in parentheses and immediately follows the `?`. 

    [?(glossary) The glossary collects information about important
    terms used in your document] is a glossary term.

	[?glossary] is also a glossary term.

	[?glossary]: The glossary collects information about important
	terms used in your document

Much like abbreviations, there is also a "shortcut" method that is similar to the approach used in prior versions of MMD.  You specify the definition for the glossary term in the usual manner, but MMD will automatically identify each instance where the term is used and substitute it automatically.  In this case, the term is limited to a more basic character set which includes letters, numbers, periods, and hyphens, but not much else.  For more complex glossary terms, you must explicitly mark uses of the term. 


### LaTeX Glossaries ###

**Note**: *Getting glossaries to work is a slightly more advanced LaTeX
feature, and might take some trial and error the first few times.*

Unfortunately, it takes an extra step to generate the glossary when creating a
pdf from a latex file:

1.	You need to have the `basic.gst` file installed, which comes with the
memoir class.

2.	You need to run a special makeindex command to generate the `.glo` file:
		``makeindex -s `kpsewhich basic.gst` -o "filename.gls" "filename.glo"``

3.	Then you run the usual pdflatex command again a few times.

Alternatively, you can use the code below to create an engine file for TeXShop (it belongs in `~/Library/TeXShop/Engines`). You can name it something like `MemoirGlossary.engine`. Then, when processing a file that needs a glossary, you typeset your document once with this engine, and then continue to process it normally with the usual LaTeX engine. Your glossary should be compiled appropriately. If you use [TeXShop][], this is the way to go. 



	#!/bin/	
	
	set path = ($path /usr/local/teTeX/bin/powerpc-apple-darwin-current 
		/usr/local/bin) # This is actually a continuation of the line above
	
	set basefile = `basename "$1" .tex`
	
	makeindex -s `kpsewhich basic.gst` -o "${basefile}.gls" "${basefile}.glo"
