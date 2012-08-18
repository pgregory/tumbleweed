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

Work is underway to update and improve the system as
a whole, focusing in the following areas.

 * Add a foreign function interface using libffi.
   This allows direct access to shared library
   functionality without having to extend the VM. 
 * Extend the basic language functionality to include
   MetaClass support. The MetaClass support follows the
   standard Smalltalk approach, all classes have a unique
   MetaClass, automatically created when the Class is
   created. MetaClasses support class methods and data.
 * Extend the basic language class hierarchy to support
   method categories.
 * Replace the memory management with a simple mark/sweep
   garbage collection scheme.
 * Various optimisations to the language interpreter to
   improve message throughput.


Building and Getting Started
----------------------------

Tumbleweed currently has very little in the way of 
dependencies. Build is via CMake, you should build out
of source. The following instructions are for Posix
based systems, Windows is similar, the required changes
should be obvious.

The CMake configuration exposes some options to control
the build.

 * TW_BUILD_TESTS
   
   Enable the Google Test based unit testing. You will
   need to make sure the GTEST_* options are also correctly
   setup.
 
 * TW_ENABLE_FFI
   
   Enable the support for dynamically binding shared
   libraries via the libffi library. You'll need to make
   sure that LIBFFI_* options are also correctly setup.
 
 * TW_SMALLINTEGER_AS_OBJECT
   
   By default Tumbleweed treats small integers as a 
   special type for efficiency. This flag will switch
   to treating them as normal objects. There is a 
   significant performance hit with this approach, it is
   only really to be used while the new approach is being
   finalised.

Get the source from github:

    git clone git://github.com/pgregory/tumbleweed.git

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

    > 'Hello, World!' print
    'Hello, World!'
    'Hello, World!'

The second 'Hello, World!' is because Tumbleweed by default
echos the result of an expression.