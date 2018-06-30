Title:	Fenced Code Blocks  
Author:	Fletcher T. Penney  
Revised:	2018-06-30   
Base Header Level:	2  
CSS:	https://fletcher.github.io/MultiMarkdown-6/css/document.css  
MMD Header:	{{TOC}}  
MMD Footer:	{{../links.txt}}  [File Transclusion]: https://fletcher.github.io/MultiMarkdown-6/syntax/transclusion.html
LaTeX Config:	tufte-handout  
HTML header:	<script type="text/javascript" src="https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.2/MathJax.js?config=TeX-AMS-MML_HTMLorMML"></script>
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.12.0/styles/default.min.css">
<script type="text/javascript" src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.12.0/highlight.min.js"></script>
<script>hljs.initHighlightingOnLoad();</script>


## Fenced Code Blocks ##

In addition to the regular indented code block that Markdown uses, you can use "fenced" code blocks in MultiMarkdown.  These code blocks do not have to be indented, and can also be configured to be compatible with a third party syntax highlighter.  These code blocks should begin with 3 to 5 backticks, an optional language specifier (if using a syntax highlighter), and should end with the same number of backticks you started with:

````
```perl
# Demonstrate Syntax Highlighting if you link to highlight.js #
# http://softwaremaniacs.org/soft/highlight/en/
print "Hello, world!\n";
$a = 0;
while ($a < 10) {
print "$a...\n";
$a++;
}
```
````

```perl
# Demonstrate Syntax Highlighting if you link to highlight.js #
# http://softwaremaniacs.org/soft/highlight/en/
print "Hello, world!\n";
$a = 0;
while ($a < 10) {
print "$a...\n";
$a++;
}
```

I don't recommend any specific syntax highlighter, but have used the following metadata to set things up.  It may or may not work for you:

```
html header:	<link rel="stylesheet" href="http://yandex.st/highlightjs/7.3/styles/default.min.css">
	<script src="http://yandex.st/highlightjs/7.3/highlight.min.js"></script>
	<script>hljs.initHighlightingOnLoad();</script>
```

Fenced code blocks are particularly useful when including another file ([File Transclusion]), and you want to show the *source* of the file, not what the file looks like when processed by MultiMarkdown.

**NOTE**: In MultiMarkdown  v6, if there is no closing fence, then the code block continues until the end of the document.