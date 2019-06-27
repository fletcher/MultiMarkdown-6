Title:	Citations  
Author:	Fletcher T. Penney  
Revised:	2018-06-27  
Base Header Level:	2  
CSS:	https://fletcher.github.io/MultiMarkdown-6/css/document.css  
MMD Header:	{{TOC}}  
MMD Footer:	{{../links.txt}}  
LaTeX Config:	tufte-handout  


## Citations ##

I have included support for *basic* bibliography features in MultiMarkdown.  I'm open to feedback on ways to improve this but keep the following in mind: 

1.	Bibliography support in MultiMarkdown is rudimentary. The goal is to offer a basic standalone feature, that can be changed using the tool of your choice to a more robust format (e.g. BibTeX, CiteProc). 

2.	Those needing more detailed function sets for their bibliographies may need customized tools to provide those services. This is a basic tool that should work for most people. Reference librarians, for example, will probably not be satisfied. 

To use citations in MultiMarkdown, you use a syntax much like that for links: 

	This is a statement that should be attributed to
	its source[p. 23][#Doe:2006].
	
	And following is the description of the reference to be
	used in the bibliography.
	
	[#Doe:2006]: John Doe. *Some Big Fancy Book*.  Vanity Press, 2006.

You are not required to use a locator (e.g. "p. 23"), and there are no special rules on what can be used as a locator if you choose to use one. If you prefer to omit the locator, just use an empty set of square brackets before the citation: 

	This is a statement that should be attributed to its 
	source[][#Doe:2006].

There are no rules on the citation key format that you use (e.g. `Doe:2006`), but it must be preceded by a `#`, just like footnotes use `^`. 

As for the reference description, you can use Markup code within this section, and I recommend leaving a blank line afterwards to prevent concatenation of several references. Note that there is no way to reformat these references in different bibliography styles; for this you need a program designed for that purpose (e.g. BibTeX). 

If you want to include a source in your bibliography that was not cited, you may use the following: 

	[Not cited][#citekey]

The `Not cited` bit is case insensitive. 

If you are creating a LaTeX document, the citations will be included, and natbib will be used by default. If you are not using BibTeX and are getting errors about your citations not being compatible with 'Author-Year', you can add the following to your documents metadata: 

	latex input:	mmd-natbib-plain

This changes the citation style in natbib to avoid these errors, and is useful when you include your citations in the MultiMarkdown document itself. 

**NOTE**: As of version 6, HTML wraps citation references in parentheses instead of brackets, e.g. `(1)` instead of `[1]`.  Also, citations are now displayed in a separate section from footnotes when outputting to HTML.

### Inline Citations ###

Citations can also be used in an inline syntax, just like inline footnotes:

	As per Doe.[#John Doe. *A Totally Fake Book 1*.  Vanity Press, 2006.]


### BibTeX Citations ###

If you are creating a LaTeX document, and need a bibliography, then you should definitely look into [BibTeX](http://www.bibtex.org/) and [natbib](http://merkel.zoneo.net/Latex/natbib.php). It is beyond the scope of this document to describe how these two packages work, but it is possible to combine them with MultiMarkdown. 

To use BibTeX in a MultiMarkdown document, you *must* use the [`BibTeX` metadata][BibTeX] to specify where your citations are stored.  You may *optionally* use the `biblio style` metadata key.

Since `natbib` is enabled by default, you have a choice between using the `\citep` and `\citet` commands. The following shows how this relates to the MultiMarkdown syntax used. 

	[#citekey]    => ~\citep{citekey}
	[#citekey][]  => ~\citep{citekey}

	[foo][#citekey] => ~\citep[foo]{citekey}

	[foo\]\[bar][#citekey] => ~\citep[foo][bar]{citekey}


	[#citekey;]    => \citet{citekey}
	[#citekey;][]  => \citet{citekey}

	[foo][#citekey;] => \citet[foo]{citekey}

	[foo\]\[bar][#citekey;] => \citet[foo][bar]{citekey}

