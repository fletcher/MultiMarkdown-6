Title:	Link and Image Attributes  
Author:	Fletcher T. Penney  
Revised:	2018-06-30  
Base Header Level:	2  
CSS:	https://fletcher.github.io/MultiMarkdown-6/css/document.css  
MMD Header:	{{TOC}}  
MMD Footer:	{{../links.txt}}  
LaTeX Config:	tufte-handout  


## Link and Image Attributes ##

Adding attributes to links and images has been requested for a long time on
the Markdown discussion list. I was fairly opposed to this, as most of the
proposals really disrupted the readability of the syntax. I consider myself a
"Markdown purist", meaning that I took John's introduction to heart:

> The overriding design goal for Markdown's formatting syntax is to make
> it as readable as possible. The idea is that a Markdown-formatted
> document should be publishable as-is, as plain text, without looking
> like it's been marked up with tags or formatting instructions. While
> Markdown's syntax has been influenced by several existing text-to-HTML
> filters, the single biggest source of inspiration for Markdown's
> syntax is the format of plain text email.

Because there was not a syntax proposal that I felt fit this goal, I was generally opposed to the idea.

Then, Choan C. Gálvez [proposed][galvez] a brilliantly simple syntax that
stayed out of the way. By simply appending the attributes to the link
reference information, which is already removed from the text itself, it
doesn't disturb the readability.

[galvez]: http://six.pairlist.net/pipermail/markdown-discuss/2005-October/001578.html

For example:

    This is a formatted ![image][] and a [link][] with attributes.
	
    [image]: http://path.to/image "Image title" width=40px height=400px
    [link]:  http://path.to/link.html "Some Link" class=external
	         style="border: solid black 1px;"

This will generate width and height attributes for the image, and a border
around the link. And while it can be argued that it does look "like it's been
marked up with tags [and] formatting instructions", even I can't argue too
strongly against it. The link and the title in quotes already look like some
form of markup, and the the additional tags are hardly that intrusive, and
they offer a great deal of functionality. They might even be useful in further
functions (citations?).

The attributes must continue after the other link/image data, and may contain
newlines, but must start at the beginning of the line. The format is
`attribute=value` or `attribute="multi word value"`. Currently, MultiMarkdown
does not attempt to interpret or make any use of any of these attributes.
Also, you can't have a multiword attribute span a newline.

**NOTE:** As of version 6, MultiMarkdown will also allow attributes in inline links as well:

	Colored [link](http://example.net "Title" style="color:red")