Title:	LaTeX Support Files  
Author:	Fletcher T. Penney  
Date:	2017-03-27  


## Introduction ##

This folder includes the LaTeX support files for MMD.  They have been renamed
for MMD v6.

The files are organized into directories using a naming convention compatible
with the MMD 6 `LaTeX Config` metadata key:

	TItle:	Some Title
	LaTeX Config:	article

Each configuration would have three files, named:

1.	mmd6-foo-leader.tex
2.	mmd6-foo-begin.tex
3.	mmd6-foo-footer.tex

(where `foo` is the name of the configuration)

Each of these files can reference other files, and the common configuration
files used in the default setups are stored in the `shared` directory.


## Installation ##

These files need to go in your `texmf` folder, wherever that may be.

With MacTeX on Mac OS X:

	~/Library/texmf/tex/latex/mmd

On most *nix accounts, you can use:

	~/texmf/tex/latex/mmd

I don't  remember off the top  of my head  where your texmf folder  belongs in
Windows.
