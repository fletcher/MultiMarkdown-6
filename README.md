
# Introduction #

This template was created out of a desire to simplify some of the setup and
configuration that I was doing over and over each time I started a new project.
Additionally, I wanted to try to start encouraging some "better practices"
(though not necessarily "best practices"):

1. [Test-driven development][tdd] -- My development of MultiMarkdown
	focused on integration testing, but really had no unit testing to
	speak of.  Some newer projects I began working on were a bit math-
	heavy, and ensuring that each piece works properly became even more
	important.  It was also nice to be able to actually develop code that
	could do *something* (via the test suite), even though the project as
	a whole was nowhere near complete.)  To accomplish this, I include the
	[CuTest] project to support writing tests for your code.

2.  Use of the [cmake] build system.  `cmake` is not perfect by any
	means, but it does offer some very useful features and a means for
	better integrating the compilation and packaging/installation aspects
	of development.  Rather than reinventing the wheel each time, this
	setup incorporates basic `cmake` functionality to make it easy to 
	control how your project is compiled, and includes automated generation
	of the test command.

3.	Templates -- `cmake` has a reasonable templating system, so that you
	can define basic variables (e.g. author, project name, etc.) and allow
	`cmake` to combine those elements to ensure consistency across source
	code and README files.

4.	Documentation -- some default setup to allow for [doxygen]-generated
	documentation.  The generated `README.md` file is used as the main 
	page, and the source c/header files are included.  Naturally, doxygen
	is a complex system, so you're responsible for figuring out how to 
	properly document your code.


[tdd]:	https://en.wikipedia.org/wiki/Test-driven_development
[cmake]:	http://www.cmake.org/
[CuTest]:	http://cutest.sourceforge.net
[doxygen]:	http://www.stack.nl/~dimitri/doxygen/


# How do I use it? #

You can download the source from [github] and get to work. The file "IMPORTANT"
contains instructions on the various build commands you can use.


I recommend using the following script to automatically create a new git repo,
pull in the default project template, and configure git-flow.  You simply have
to rename your project from `new-project` to whatever you desire:


	#!/bin/sh

	git init new-project

	cd new-project

	git remote add "template" https://github.com/fletcher/c-template.git

	git pull template master

	git flow init -d

	git checkout develop


Using this approach, you can define your own "origin" remote if you like, but
the "template" remote can be used to update the core project files should any
improvements come about:

	git checkout develop
	git merge template master

**NOTE**: `cmake` is a complex suite of utilities, and if you have trouble you
will need to get support elsewhere.  If you find errors in this template, by
all means I want to hear about them and fix them, but this is just a basic 
framework to get you started.  In all likelihood, all but the most basic
projects will need some customization.


[github]:	https://github.com/fletcher/c-template


# License #

The `new-project` project is released under the MIT License.

GLibFacade.c and GLibFacade.h are from the MultiMarkdown v4 project:

	https://github.com/fletcher/MultiMarkdown-4/

MMD 4 is released under both the MIT License and GPL.


CuTest is released under the zlib/libpng license. See CuTest.c for the text
of the license.


## The MIT License ##

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.