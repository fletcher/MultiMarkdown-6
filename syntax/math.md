Title:	Math  
Author:	Fletcher T. Penney  
Revised:	2018-06-27  
Base Header Level:	2  
Transclude Base:	.
CSS:	https://fletcher.github.io/MultiMarkdown-6/css/document.css  
MMD Header:	{{TOC}}  
MMD Footer:	{{../links.txt}}  
HTML Footer:	foo {{../highlighting.txt}}
LaTeX Config:	tufte-handout  

## Math ##


MultiMarkdown 2.0 used [ASCIIMathML] to typeset mathematical equations. There
were benefits to using ASCIIMathML, but also some disadvantages.

When rewriting for MultiMarkdown 3.0, there was no straightforward way to
implement ASCIIMathML which lead me to look for alternatives. I settled on
using [MathJax]. The advantage here is that the same syntax is supported by
MathJax in browsers, and in native LaTeX syntax when creating LaTeX documents. 

To enable MathJax support in web pages, you have to include a link to an
active MathJax installation --- setting this up is beyond the scope of this
document, but it's not too hard.

Here's an example of the metadata setup, and some math:

	latex input:	mmd-article-header  
	Title:			MultiMarkdown Math Example  
	latex input:	mmd-article-begin-doc  
	latex footer:	mmd-memoir-footer  
	HTML header:	<script type="text/javascript" src="https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.2/MathJax.js?config=TeX-AMS-MML_HTMLorMML"></script>
	
			
	An example of math within a paragraph --- \\({e}^{i\pi }+1=0\\)
	--- easy enough.

	And an equation on it's own:

	\\[ {x}_{1,2}=\frac{-b\pm \sqrt{{b}^{2}-4ac}}{2a} \\]

	That's it.


Here's what it looks like in action (if you're viewing this document in a
supported format):

> An example of math within a paragraph --- \\({e}^{i\pi }+1=0\\)
--- easy enough.
>
> And an equation on it's own:
>
>\\[ {x}_{1,2}=\frac{-b\pm \sqrt{{b}^{2}-4ac}}{2a} \\]
>
> That's it.

In addition to the `\\[ \\]` and `\\( \\)` syntax, you can use LaTeX-style "dollar sign" delimiters:
	
	An example of math within a paragraph --- ${e}^{i\pi }+1=0$
	--- easy enough.

	And an equation on it's own:

	$${x}_{1,2}=\frac{-b\pm \sqrt{{b}^{2}-4ac}}{2a}$$

	That's it.

In order to be correctly parsed as math, there *must* not be any space between the `$` and the actual math on the inside of the delimiter, and there *must* be space on the outside.  ASCII punctuation can also serve as "space" outside of the math.


### Superscripts and Subscripts ###

You can easily include superscripts and subscripts in MultiMarkdown as well:

	This apartment has an area of 100m^2
	One must consider the value of x~z

becomes
> This apartment has an area of 100m^2  
> One must consider the value of x~z

The subscript must not contain any whitespace or punctuation.

More complicated exponents and subscripts can be delimited like this:

	y^(a+b)^
	x~y,z~

> y^(a+b)^  
> x~y,z~
