Title:	File Transclusion  
Author:	Fletcher T. Penney  
Revised:	2018-06-30  
Base Header Level:	2  
CSS:	https://fletcher.github.io/MultiMarkdown-6/css/document.css  
MMD Header:	{{TOC}}  
MMD Footer:	{{../links.txt}}  
LaTeX Config:	tufte-handout  
HTML header:	<script src="https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.2/MathJax.js?config=TeX-AMS-MML_HTMLorMML"></script>
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.12.0/styles/default.min.css">
<script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.12.0/highlight.min.js"></script>
<script>hljs.initHighlightingOnLoad();</script>


## File Transclusion ##

File transclusion is the ability to tell MultiMarkdown to insert the contents of another file inside the current file being processed.  For example:

	This is some text.
	
	{{some_other_file.txt}}
	
	Another paragraph

If a file named `some_other_file.txt` exists, its contents will be inserted inside of this document *before* being processed by MultiMarkdown.  This means that the contents of the file can also contain MultiMarkdown formatted text.

If you want to display the *contents* of the file without processing it, you can include it in a code block (you may need to remove trailing newlines at the end of the document to be included):

	This is some text

	```
	{{relative/path/to/some_other_file.txt}}
	```

	Another paragraph

Transclusion is recursive, so the file being inserted will be scanned to see if it references any other files.

Metadata in the file being inserted will be ignored.  This means that the file can contain certain metadata when viewed alone that will not be included when the file is transcluded by another file.

You can use the `[Transclude Base]` metadata to specify where MultiMarkdown should look for the files to be included.  All files must be in this folder.  If this folder is not specified, then MultiMarkdown will look in the same folder as the parent file.

**Note:**  Thanks to David Richards for his ideas in developing support for this feature.

### Search Paths ###


When you process a file with MMD, it uses that file's directory as the search
path for included files. For example:

| Directory	| Transcluded Filename	| Resolved Path 	|  
| ------------------	| -----------------------------	| ------------------------------	|  
| `/foo/bar/`	| `bat`	| `/foo/bar/bat`	|  
| `/foo/bar/`	| `baz/bat`	| `/foo/bar/baz/bat`	|  
| `/foo/bar/`	| `../bat` 	| `/foo/bat`	|  

This is the same as MMD v 5.  What's different is that when you transclude a
file, the search path stays the same as the "parent" file, **UNLESS** you use
the `transclude base` metadata to override it.  The simplest override is:

	transclude base: .

This means that any transclusions within the file will be calculated relative
to the file, regardless of the original search path.  

Alternatively you could specify that any transclusion happens inside a
subfolder:

	transclude base: folder/

Or you can specify an absolute path:

	transclude base: /some/path

This flexibility means that you can transclude different files based on
whether a file is being processed by itself or as part of a "parent" file.
This can be useful when a particular file can either be a standalone document,
or a chapter inside a larger document.


### Wildcard Extensions ###

Sometimes you may wish to transclude alternate versions of a file depending on your output format.  Simply use the extension ".*" to have MMD choose the proper version of the file (e.g. `foo.tex`, `foo.fodt`, `foo.html`, etc.)

	Insert a different version of a file here based on export format:
	{{foo.*}}


### Transclusion Preprocessing ###

If you want to perform transclusion, *without* converting to another format, you can use `mmd` as the output format:

	multimarkdown -t mmd foo.txt

This will only perform "basic" transclusion --it does not support wildcard extensions, since the final output format is not known.
