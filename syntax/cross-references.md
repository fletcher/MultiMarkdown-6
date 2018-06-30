Title:	Cross-References  
Author:	Fletcher T. Penney  
Revised:	2018-06-30  
Base Header Level:	2  
CSS:	https://fletcher.github.io/MultiMarkdown-6/css/document.css  
MMD Header:	{{TOC}}  
MMD Footer:	{{../links.txt}}  
LaTeX Config:	tufte-handout  


## Cross-References ##

An oft-requested feature was the ability to have Markdown automatically handle
within-document links as easily as it handled external links. To this aim, I
added the ability to interpret `[Some Text][]` as a cross-link, if a header
named "Some Text" exists.

As an example, `[Metadata][]` will take you to the
[section describing metadata][Metadata].

Alternatively, you can include an optional label of your choosing to help
disambiguate cases where multiple headers have the same title:

	### Overview [MultiMarkdownOverview] ##

This allows you to use `[MultiMarkdownOverview]` to refer to this section
specifically, and not another section named `Overview`. This works with atx-
or settext-style headers.

If you have already defined an anchor using the same id that is used by a
header, then the defined anchor takes precedence.

In addition to headers within the document, you can provide labels for images
and tables which can then be used for cross-references as well.
