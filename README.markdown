Tumbleweed
==========

A lightweight, image based, encapsulated development
environment based around Smalltalk, specifically,
"A Little Smalltalk" version 3.0 by Timothy A. Budd.

Why Version 3?
--------------

The reason for basing on the 3.0 version of the LST
system, and not the more recent 4.x version, is that 
firstly, 3.0 is public domain, and secondly, the 3.0
codebase is much clearer and simpler to follow and 
understand. Tumbleweed began as a learning exercise,
so having a clear and relatively simple codebase to 
start from was more important than the optmisations 
and other improvements that came about with the
rewrite to version 4.

Status
------

So far, the majority of the work has been on updating
the VM, and enhancing the basic language support.

 * Added a foreign function interface using libffi.
   This allows direct access to shared library
   functionality without having to extend the VM. 
   Primary initial use is to enable the IUP based
   GUI library.
 * Extend the basic language functionality to include
   MetaClass support. The MetaClass support follows the
   standard Smalltalk approach, all classes have a unique
   MetaClass, automatically created when the Class is
   created. MetaClasses support class methods and data.
 * Extend the basic language class hierarchy to support
   method categories.

Building and Getting Started
----------------------------

Tumbleweed currently has very little in the way of 
dependencies. Build is via CMake, you should build out
of source. The following instructions are for Posix
based systems, Windows is similar, the required changes
should be obvious.

Get the source from github:

  git clone https://pgregory@github.com/pgregory/tumbleweed.git

Make a build directory, and configure with cmake:

  mkdir build
  cd build
  cmake ../tumbleweed
  make

You should then end up with an executable 'tw' in the build
folder. To work with the GUI, you'll need a copy of the IUP
shared library (.so or .dll) in the same foler. Tumbleweed
will work fine without it, but you'll only be able to work
from the command line.

Run Tumbleweed:

  ./tw

You can then run standard Smalltalk like commands from the
command prompt.

  > 'Hello, World!' printstring
  'Hello, World!'
  object count 6942

You can launch a class Browser to examine the class hierarchy
in the current running image.

  > Browser open

From within the browser you can view the class hierarchy by
selecting the class from the class list, you'll see a list
of protocols for that class, select a protocol to see a list
of methods. Seleting a method in the list shows the source
to the method in the lower panel, you can edit the source and 
hit accept to change the method implementation.

