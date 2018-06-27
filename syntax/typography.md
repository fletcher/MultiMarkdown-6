Title:	Smart Typography  
Author:	Fletcher T. Penney  
Revised:	2018-06-27  
Base Header Level:	2  
transclude base:	.
CSS:	https://fletcher.github.io/MultiMarkdown-6/css/document.css  
MMD Footer:	{{../links.txt}}  
LaTeX Config:	tufte-handout  


## Smart Typography ##

MultiMarkdown incorporates John Gruber's [SmartyPants] tool in addition to the core Markdown functionality.  This program converts "plain" punctuation into "smarter" typographic punctuation.

Just like the original, MultiMarkdown converts:

* Straight quotes (`"` and `'`) into "curly" quotes 
* Backticks-style quotes (` ``this'' `) into "curly" quotes
* Dashes (`--` and `---`) into en- and em- dashes
* Three dots (`...`) become an ellipsis

MultiMarkdown also includes support for quotes styles other than English (the default).  Use the `quotes language` metadata to choose:

* dutch (`nl`)
* german(`de`)
* germanguillemets
* french(`fr`)
* spanish(`en`)
* swedish(`sv`)

This feature is enabled by default, but is disabled in `compatibility` mode, since it is not part of the original Markdown.  You can also use the `--nosmart` command line option to disable this feature.