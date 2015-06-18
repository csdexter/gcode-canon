G-Code Canonical Interpreter ![Travis CI build status](https://travis-ci.org/csdexter/gcode-canon.svg?branch=master "Build status at HEAD, as reported by Travis CI")
============================

Copyright (C) 2012-2015, Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>

All files in this repository are licensed under the
**GNU Generic Public License v3**, a copy of whose text can be accessed
 [here](http://www.gnu.org/licenses/gpl.html).

* * *

An exception is hereby granted to the maintainer(s) of the
[grbl](https://github.com/grbl/grbl) project to, at their own discretion, pick
either **GPLv3** (as stated above) or the **BSD 3-clause license** (reproduced in the
file [COPYING.grbl](https://github.com/csdexter/gcode-canon/blob/master/COPYING.grbl))
as their licensing terms for using any file in this repository.

For the purposes of GPLv3 requirements only *and* in a situation in which the
`grbl` maintainer(s) use any file from this repository *and* they choose GPLv3 as
the licensing terms under which such use is to occur, I'm hereby assigning
copyright for the used files (as used and while used) to them.

* * *

This project is an attempt at 100% coverage of the existing G-Code standard.
It aims to build a parser that can handle the entirety of RS724-NGC plus the
most common extensions and additions to it, as encountered in the wild.

To make testing (and further reuse, the secondary aim of the project) straight
forward, the interpreter comes with a virtual debugging CNC so that the whole
implementation behaves as if there were an actual machine to control.
