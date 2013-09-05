# Linux Agent Project Standards

In order for a component to get the point where it can be considered 'coding complete', a component needs to meet a certain set of standards. This document describes them.

## Code architecture and style
### Google's C++ Style Guide
http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml

This guide tends to be the guiding document for most C++ code. However, this project differs from the projects Google's guide references in a couple of ways.

#### Exceptions
Google doesn't use exceptions because their existing code doesn't use exceptions. This isn't the case for us, exceptions allow us to perform clean-up (critical!) on error. Smart pointers help a lot here.

Removing the restriction on exceptions means we are free to do work in constructors.

#### Boost
Google restricts a lot of the Boost libraries. We don't worry about this - Boost contains high quality code, and contains a lot of classes that are perfect for our purposes (e.g. [interval containers](http://www.boost.org/doc/libs/1_54_0/libs/icl/doc/html/index.html) for block trace data).

#### C++11
In addition to restricting Boost, Google restricts many C++ features. In general we follow these restrictions, but [the concurrency classes](http://en.cppreference.com/w/cpp/thread) are too tempting to resist - we use these without hesitation.

***

### C
#### Function scope
As there is no notion of public/private, use static for functions not intended for external use. See Red Hat's [libdm-string.c](https://git.fedorahosted.org/cgit/lvm2.git/tree/libdm/libdm-string.c) for a good example.

#### Headers
Headers should [provide a C++ interface](http://stackoverflow.com/questions/3789340/combining-c-and-c-how-does-ifdef-cplusplus-work).

Headers should be self-contained. Someone using functions in a .h file shouldn't need to look at the .c file. [libdevmapper.h](https://git.fedorahosted.org/cgit/lvm2.git/tree/libdm/libdevmapper.h) is a good example.

### Code Formatting
Having a consistent [coding style is important](http://stackoverflow.com/a/1325617/965648) for any reasonably sized project. More important than what rules are chosen is consistency with those rules.

#### C++
* [Follow Google's guide for formatting](http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml#Formatting)

#### C
C is generally very close to C++ with a few exceptions
* 8 width tabs
* [K&R Braces](http://en.wikipedia.org/wiki/Indent_style#K.26R_style)
    * Braces on next line for functions, same line for everything else 
* *simple* gotos for error handling

#### Python
* [PEP8](http://www.python.org/dev/peps/pep-0008/)

## Testing

**TODO**