Title:	Images  
Author:	Fletcher T. Penney  
Revised:	2018-06-30  
Base Header Level:	2  
CSS:	https://fletcher.github.io/MultiMarkdown-6/css/document.css  
MMD Header:	{{TOC}}  
MMD Footer:	{{../links.txt}}
	[Link and Image Attributes]: https://fletcher.github.io/MultiMarkdown-6/syntax/attributes.html
LaTeX Config:	tufte-handout  


## Images ##

The basic syntax for images in Markdown is:


	![Alt text](/path/to/img.jpg)

	![Alt text](/path/to/img.jpg "Optional title")


	![Alt text][id]

	[id]: url/to/image  "Optional title attribute"


In addition to the attributes you can use with links and images (described in [another section][Link and Image Attributes]), MultiMarkdown also adds a few additional features.  If an image is the only thing in a paragraph, it is treated as a block level element:

	This image (![Alt text](/path/to/img.jpg))
	is different than the following image:

	![Alt text](/path/to/img.jpg)

The resulting HTML is:

	<p>This image (<img src="/path/to/img.jpg" alt="Alt text" />)
	is different than the following image:</p>
	
	<figure>
	<img src="/path/to/img.jpg" alt="Alt text" />
	<figcaption>Alt text</figcaption>
	</figure>

The first one would be an inline image.  The second one (in HTML) would be wrapped in an HTML `figure` element.  In this case, the `alt` text is also used as a figure caption, and can contain MultiMarkdown syntax (e.g. bold, emph, etc.).  The alt text is not specifically designed to limit which MultiMarkdown is supported, but there will be limits and block level elements aren't supported.
