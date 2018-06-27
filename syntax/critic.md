Title:	CriticMarkup  
Author:	Fletcher T. Penney  
Revised:	2018-06-27  
Base Header Level:	2  
CSS:	https://fletcher.github.io/MultiMarkdown-6/css/document.css  
MMD Header:	{{TOC}}  
MMD Footer:	{{../links.txt}}  
LaTeX Config:	tufte-handout  


## CriticMarkup ##

### What Is CriticMarkup? ###

> CriticMarkup is a way for authors and editors to track changes to documents in plain text. As with Markdown, small groups of distinctive characters allow you to highlight insertions, deletions, substitutions and comments, all without the overhead of heavy, proprietary office suites. <http://criticmarkup.com/> 

CriticMarkup is integrated with MultiMarkdown itself, as well as [MultiMarkdown Composer].  I encourage you to check out the [CriticMarkup] web site to learn more as it can be a very useful tool.  There is also a great video showing CriticMarkup in use while editing a document in MultiMarkdown Composer. 


### The CriticMarkup Syntax ###

The CriticMarkup syntax is fairly straightforward.  The key thing to remember is that CriticMarkup is processed *before* any other MultiMarkdown is handled.  It's almost like a separate layer on top of the MultiMarkdown syntax. 

When editing in MultiMarkdown Composer, you can have CriticMarkup syntax flagged in the both the editor pane and the preview window.  This will allow you to see changes in the HTML preview. 

When using CriticMarkup with MultiMarkdown itself, you have four choices: 

*	Leave the CriticMarkup syntax in place (`multimarkdown foo.txt`). MultiMarkdown will attempt to show the changes as highlights in the exported document, where possible.

*	Accept all changes, giving you the "new" document (`multimarkdown -a foo.txt` or `multimarkdown --accept foo.txt`)

*	Reject all changes, giving you the "original" document (`multimarkdown -r foo.txt` or `multimarkdown --reject foo.txt`)


CriticMarkup comments and highlighting are ignored when processing. 

Deletions from the original text: 

	This is {--is --}a test.

Additions: 

	This {++is ++}a test.

Substitutions: 

	This {~~isn't~>is~~} a test.

Highlighting: 

	This is a {==test==}.

Comments: 

	This is a test{>>What is it a test of?<<}.


### CriticMarkup Limitations ###

If you `--accept` or `--reject` CriticMarkup changes, then it should work properly in just about any document.

If you want to try to include your changes as "notes" in the final document, then certain situations will lead to results that were probably not what you intended.

1.	CriticMarkup must be contained within a single block (e.g. paragraph, list item, etc.)  CM that spans multiple blocks will not be recognized.

2.	CriticMarkup that crosses multiple MMD spans (e.g. `{++** foo} bar**`) will not properly manage the intended MultiMarkdown markup.  This example would not result in bold being applied to `foo bar`.


### My philosophy on CriticMarkup

I view CriticMarkup as two things (in addition to the actual tools that implement these concepts): 

1.	A syntax for documenting editing notes and changes, and for collaborating amongst coauthors. 

2.	A means to display those notes/changes in the HTML output. 

I believe that #1 is a really great idea, and well implemented.  #2 is not so well implemented, largely due to the "orthogonal" nature of CriticMarkup and the underlying Markdown syntax. 

CM is designed as a separate layer on top of Markdown/MultiMarkdown.  This means that a Markdown span could, for example, start in the middle of a CriticMarkup structure, but end outside of it.  This means that an algorithm to properly convert a CM/Markdown document to HTML would be quite complex, with a huge number of edge cases to consider.  I've tried a few (fairly creative, in my opinion) approaches, but they didn't work.  Perhaps someone else will come up with a better solution, or will be so interested that they put the work in to create the complex algorithm.  I have no current plans to do so. 

Additionally, there is a philosophical distinction between documenting editing notes, and using those notes to produce a "finished" document (e.g. HTML or PDF) that keeps those editing notes intact (e.g. strikethroughs, highlighting, etc.) I believe that CM is incredibly useful for the editing process, but am less convinced for the output process (I know many others disagree with me, and that's ok.  And to be clear, I think that what Gabe and Erik have done with CriticMarkup is fantastic!) 

There are other CriticMarkup tools besides MultiMarkdown and [MultiMarkdown Composer](http://multimarkdown.com/), and you are more than welcome to use them. 

For now, the *official* MultiMarkdown support for CriticMarkup consists of: 

1.	CriticMarkup syntax is "understood" by the MultiMarkdown parser, and by MultiMarkdown Composer syntax highlighting.

2.	When converting from MultiMarkdown text to an output format, you can ignore CM formatting with `compatibility` mode (probably not what you want to do), accept all changes, or reject all changes (as above).  These are the preferred choices.

3.	The secondary choice, is to *attempt* to show the changes in the exported document.  Because the syntaxes are orthogonal, this will not always work properly, and will not always give valid output files.

