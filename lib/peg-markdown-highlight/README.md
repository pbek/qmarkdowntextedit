
PEG Markdown Highlight
========================
_Copyright 2011-2016 Ali Rantakari_ -- <http://hasseg.org>

This is a syntax highlighter for the [Markdown] language, designed to be integrated into GUI text editor programs. It uses a recursive-descent parser for interpreting the input (instead of e.g. regular expressions), and this parser is based on the PEG grammar from John MacFarlane's [peg-markdown] project.

[Markdown]: http://daringfireball.net/projects/markdown
[peg-markdown]: https://github.com/jgm/peg-markdown

PEG Markdown Highlight…

- is written in __ANSI/ISO C89 with GNU extensions__
- has __no external dependencies__
- is __re-entrant__
- is probably __slower__ than many simpler (but less correct) highlighting mechanisms but __faster__ than most Markdown compilers
- works (at least) on __OS X, Linux and Windows__
- is used in __shipping software__ ([Mou], [MacDown], [LightPaper], [Erato], [CuteMarkEd])
- is dual-licensed under the __MIT and GPL2+__ licenses.

[Mou]: http://mouapp.com/
[MacDown]: http://macdown.uranusjr.com/
[LightPaper]: http://clockworkengine.com/lightpaper-mac
[Erato]: http://9muses.se/erato
[CuteMarkEd]: http://cloose.github.io/CuteMarkEd

This program uses the PEG grammar from John MacFarlane's `peg-markdown` project,
and the `greg` parser generator by Why The Lucky Stiff and Amos Wenger (`greg`
is heavily based on `peg/leg` by Ian Piumarta). It also contains an
implementation of the merge sort algorithm for linked lists by Simon Tatham.

Thanks to these gentlemen (and everyone who contributed to their projects) for
making this one possible.

_See the `LICENSE` file for licensing information._


Why This is Useful
-------------------------------------------------------------------------------

Existing syntax highlighting solutions in (programming) editors are too simple to be able to handle the context sensitivity of the Markdown language, and the fact that it is not well defined. They usually work well for simple cases but fail for many nontrivial inputs that existing Markdown compilers handle correctly. __This project is an attempt to bring Markdown syntax highlighting to the same level of “correctness” as the existing compilers.__


Quick Code Examples
-------------------------------------------------------------------------------

Here are some quick, simple examples of what it might look like to use this highlighter in your project.

__Using the Cocoa highlighter classes to highlight an NSTextView with default settings:__

```objective-c
#import "HGMarkdownHighlighter.h"

@interface MyClass : NSObject {
    HGMarkdownHighlighter *highlighter;
}

- (void) awakeFromNib {
    highlighter = [[HGMarkdownHighlighter alloc]
                   initWithTextView:myTextView];
    [highlighter activate];
}
```

__Manually highlighting a TextWidget in some hypothetical GUI framework:__

```c
#include "pmh_parser.h"

void highlight(TextWidget *textWidget)
{
    pmh_element **results;
    pmh_markdown_to_elements(textWidget->containedText, pmh_EXT_NONE, &results);
    
    for (int i = 0; i < pmh_NUM_LANG_TYPES; i++) {
        TextStyle style;
        switch (i) {
            case pmh_EMPH: style = ItalicStyle; break;
            case pmh_STRONG: style = BoldStyle; break;
            case pmh_H1: style = LargeFontStyle; break;
            default: style = FunkyStyle; break;
        }
        pmh_element *element_cursor = results[i];
        while (element_cursor != NULL) {
            textWidget->setStyleForSpan(element_cursor->pos,
                                        element_cursor->end,
                                        style);
            element_cursor = element_cursor->next;
        }
    }

    pmh_free_elements(results);
}
```


Repo Contents
-------------------------------------------------------------------------------

This project contains:

- A Markdown parser for syntax highlighting, written in C. The parser itself
  should compile as is on OS X, Linux and Windows (at least).
- Helper classes for syntax highlighting `NSTextView`s in Cocoa applications.
- A simple example on how to highlight a `GtkTextView` in a GTK+ application.
- A simple example on how to highlight a `QTextEdit` in a Qt application.
- A parser for stylesheets that define syntax highlighting styles


API Documentation
-------------------------------------------------------------------------------

The public APIs are documented using [Doxygen][dox]. If you have it installed,
just run `make docs` and they should be available under the `docs/` directory.

[dox]: http://doxygen.org



Using the Parser in Your Application
-------------------------------------------------------------------------------

The parser has been written in ANSI/ISO C89 with GNU extensions, which means
that you need a _GCC-compatible compiler_ (see section on MSVC below, though).
You also need _Bourne Shell_ and some common Unix utilities due to a utility
shell script that is used to combine some files in a `make` step.


### Files You Need

You need to add the following files into your project:

- `pmh_definitions.h`
- `pmh_parser.h`
- `pmh_parser.c`

`pmh_parser.c` implements the parser and must be generated with `make`.
`pmh_parser.h` contains the parser's public interface and
`pmh_definitions.h` some public definitions you might want to use in files
where you don't wish to import the parser interface itself.


### Compiling in Microsoft Visual C++

First you need to generate `pmh_parser.c` somehow. There are two main
ways to do this:

- Use a Linux or OS X machine to generate it
- Generate it on Windows using MinGW (you'd run `make` in the MinGW shell)

Whichever way you go, the command you run is `make pmh_parser.c`.

MSVC does not support some of the GNU extensions the code uses, but it should
compile it nicely as C++ (just change the extensions to `.cpp` or set some
magic switch in the project settings to get the same effect). You may need to
insert the following to the beginning of `pmh_parser.c`:

    #include "stdafx.h"



Stylesheet Parser
-------------------------------------------------------------------------------

The `pmh_styleparser.h` file contains the style parser's public interface and
the `pmh_styleparser.c` file implements the parser. The style parser depends
on the main parser.

The stylesheet syntax is documented in `/styleparser/stylesheet_syntax.md`.
This file is compiled into an HTML format upon running `make docs`.



