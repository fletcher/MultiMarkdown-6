## About ##

|            |                           |  
| ---------- | ------------------------- |  
| Title:     | C-Template        |  
| Author:    | Fletcher T. Penney       |  
| Date:      | 2017-01-16 |  
| Copyright: | Copyright © 2015-2017 Fletcher T. Penney.    |  
| Version:   | 1.0.5      |  


## Introduction ##

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

4.	Documentation -- some default setup to allow for [Doxygen]-generated
	documentation.  The generated `README.md` file is used as the main 
	page, and the source c/header files are included.  Naturally, Doxygen
	is a complex system, so you're responsible for figuring out how to 
	properly document your code.

5.	Simplify `git` a touch -- In my larger projects, I make heavy use of
	git modules.  One project may make use of 20-30 modules, which are
	designed to be re-usable across other projects.  I found that I was
	spending too much time making sure that I had the latest version
	of a module checked out, so I created two scripts to help me keep
	my modules in line: `link_git_modules` and `update_git_modules`.
	You run the `link` script once to ensure that your modules are properly
	set up, and can then run the `update` script at any time to be sure
	you've pulled the latest version.  One advantage of this is that your
	modules are set to a branch, rather than just a detached commit. It
	may or may not work for your needs, but it saves me a bunch of time
	and headache.


[tdd]:	https://en.wikipedia.org/wiki/Test-driven_development
[cmake]:	http://www.cmake.org/
[CuTest]:	http://cutest.sourceforge.net
[Doxygen]:	http://www.stack.nl/~dimitri/doxygen/


## How do I use it? ##

You can download the source from [github] and get to work. The file "IMPORTANT"
contains instructions on the various build commands you can use.


I recommend using the following script to automatically create a new git repo,
pull in the default project template, and configure git-flow.  You simply have
to rename your project directory from `new-project` to whatever you desire:


	#!/bin/sh

	git init new-project

	cd new-project

	git remote add "template" https://github.com/fletcher/c-template.git

	git pull template master

	git flow init -d

	git checkout develop


Using this approach, you can define your own `origin` remote if you like, but
the `template` remote can be used to update the core project files should any
improvements come about:

	git checkout develop
	git merge template master

**NOTE**: `cmake` is a complex suite of utilities, and if you have trouble you
will need to get support elsewhere.  If you find errors in this template, by
all means I want to hear about them and fix them, but this is just a basic 
framework to get you started.  In all likelihood, all but the most basic
projects will need some customization.


[github]:	https://github.com/fletcher/c-template


## Configuration ##


### CMakeLists.txt File ###

First, you should update the project information under the "Define Our Project"
section, including the title, description, etc.  This information will be used
to update the README, as well as to create the `version.h` file so that the 
project can have access to its own version number.

You will then need to update the various groups in the "Source Files" section
so that Cmake will be able to determine which files are used to build your
project.  For reasons that will become clear later, try to follow the
suggestions for the different groups of files.

You then need to define your targets, such as a library, or executable, etc.
Obviously, this will depend on the needs of your project.  You can also add
custom steps based on the Target OS (OS X, Windows, *nix, etc.).

You can use CPack to generate installers for your software.  This can be
complex, and you will need to modify this section heavily.

CuTest is used by default to provide unit testing (see below), but you
can also use CMake/CTest to provide integration testing.  Again, this will
be up to you to configure.


### CuTest ###

[CuTest] provides a means to integrate unit testing with your C source code.
Once you get the hang of it, it's easy to use.


### Doxygen ###

[Doxygen] is used to generate documentation from the source code itself. 
Properly configuring your source for this is up to you.  You can modify the
`doxygen.conf.in` template with your desired settings as desired, but most
of the basics are handled for you based on your CMake configuration.


### GitHub Pages Support ###

The `configure-gh-pages` script sets up a `documentation` directory that is 
linked to a `gh-pages` branch of the project.  You can then run `make gh-pages` 
to update the documentation in this directory.  Commit and push to your origin,
and your projects gh-page is updated.


### Makefile ###

The overall build process is controlled by the master `Makefile`.  It provides
the following commands:

	make
	make release

Generate the CMake build files for use or distribution.  Once complete you will
need to change to the `build` directory and run `make`, `make test`, and
`cpack` as desired.

	make zip

Direct CPack to create a zip installer rather than a graphical installer.

	make debug

Generate build files for [CuTest] unit testing.  In the `build` directory, 
run `make`, then `make test`.

	make analyze

If you have `clang` installed, this will generate debug build files with the
`scan-build` command.  In the `build` directory, run `scan-build -V make`
to compile the software and view the static analysis results.

	make xcode

Build a project file for Xcode on OS X.

	make windows
	make windows-zip
	make windows-32
	make windows-zip-32

Use the MinGW software to cross-compile for Windows on a *nix machine.  You can
specify the 32 bit option, and also the zip option as indicated.

	make documentation

Build the [Doxygen]-generated documentation.

	make clean

Clean out the `build` directory.  Be sure to run this before running another
command.


## Git Submodules ##

Apparently, submodules are a rather controversial feature in git.  For me,
however, they have proven invaluable.  My most active projects depend on each
other, and the submodule feature allows me to easily keep everything up to
date.  That said, however, I quickly realized that submodules don't work very
well using default commands.

The problem is that I want to always use the latest version of my submodules.
This is more easily accomplished when the submodule is set to the `master`
branch of the original repository, rather than a detached commit as happens
by default.  In order to easily keep all submodules updated, there are two 
scripts:

1. `link_git_modules` -- this script is generally only run when the master
repository is first cloned, but can also be run after a new submodule is 
added.  It causes the submodules to automatically track the master branch.
If you need to modify this, there are instructions in the script itself 
explaining how to modify it on a per submodule basis.  Running this script 
more than one time will not hurt anything.

2. `update_git_modules` -- this script simply causes each submodule to be
updated to the latest commit in the original repository.  Again, running it
multiple times doesn't hurt anything.


## Source File Templates ##

In the `templates` directory are two files, `template.c.in` and
`template.h.in`.  These are used to create default source files that include
the project title, copyright, license, etc. They are also set up to include
some example information for [Doxygen] and [CuTest].


## License ##

The `c-template` project is released under the MIT License.

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
