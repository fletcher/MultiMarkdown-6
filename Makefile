# Compile the MultiMarkdown User's Guide into documentation or test suite

srcfiles := $(wildcard *.md */*.md )

htmlfiles := $(patsubst %.md, %.html, $(srcfiles))

epubfiles := $(patsubst %.md, %.epub, $(srcfiles))

rtffiles := $(patsubst %.md, %.rtf, $(srcfiles))

texfiles := $(patsubst %.md, %.tex, $(srcfiles))

odffiles := $(patsubst %.md, %.fodt, $(srcfiles))

pdffiles := $(patsubst %.md, %.pdf, $(srcfiles))

examples := $(wildcard examples/*.text)

ex-html := $(patsubst %.text, %.html, $(examples))

ex-rtf := $(patsubst %.text, %.rtf, $(examples))

ex-tex := $(patsubst %.text, %.tex, $(examples))

ex-odf := $(patsubst %.text, %.fodt, $(examples))



all: $(htmlfiles) $(texfiles) # $(rtffiles) $(odffiles)

guide: MMD_Users_Guide.html

html: $(htmlfiles)

epub: MMD_Users_Guide.epub

pdf: MMD_Users_Guide.tex
	latexmk MMD_Users_Guide
	makeglossaries MMD_Users_Guide
	latexmk MMD_Users_Guide
	latexmk MMD_Users_Guide
	latexmk -c MMD_Users_Guide
	rm MMD_Users_Guide.tex MMD_Users_Guide.a* MMD_Users_Guide.gl*

odf: MMD_Users_Guide.fodt

%.html: %.md
	../build/multimarkdown -b -t html $*.md

%.html: %.text
	../build/multimarkdown -b -t html $*.text

%.epub: %.md
	../build/multimarkdown -b -t epub $*.md

%.epub: %.text
	../build/multimarkdown -b -t epub $*.text

%.rtf: %.md
	../build/multimarkdown -b -t rtf $*.md

%.rtf: %.text
	../build/multimarkdown -b -t rtf $*.text

%.tex: %.md
	../build/multimarkdown -b -t latex $*.md

%.tex: %.text
	../build/multimarkdown -b -t latex $*.text

%.fodt: %.md
	../build/multimarkdown -b -t odf $*.md

%.fodt: %.text
	../build/multimarkdown -b -t odf $*.text


examples: $(ex-html) $(ex-rtf) $(ex-tex) $(ex-odf)


clean:
	@rm $(htmlfiles) $(rtffiles) $(texfiles) $(odffiles) $(pdffiles) $(epubfiles)

clean-examples:
	@rm $(ex-html) $(ex-rtf) $(ex-tex) $(ex-odf)
