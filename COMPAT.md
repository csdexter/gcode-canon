# Compatibility in terms of implemented functionality

* * *

## Abstract

`gcode-canon` aims to be the most complete RS274NGC a.k.a. G-Code interpreter.
The problem is, apart from a small subset of the language that even jury-rigged
implementations support, there is no single standard encompassing all the
advanced functionality. Instead, there are various *de facto* standards created
by the multiple controller vendors out there.

Therefore, this document aims to describe the complete vocabulary and syntax
supported by `gcode-canon`, allowing you to draw your own conclusions as to its
compatibility and indeed suitability for your use.

## Basic Language Features

* empty lines are ignored and stripped
* the character set is ASCII (7-bit, that is)
* the maximum line length is 256 characters including the terminator
* comments are delimited by `(` and `)`, must be completely contained within
one line and are ignored and stripped
* case does not matter, all characters are coerced to upper case on being read
* a comment can be prefixed with `MSG,` to get a message comment that will be
displayed verbatim on the machine console
* whitespace is ignored and stripped, with the sole exception of whitespace
inside a message comment
* lines starting with `/` (a.k.a. block delete) can be programatically ignored
and stripped; if they are not ignored and stripped, then the block delete
character shall be
* the character `%` (a.k.a. the program separator) may appear alone on a line
* the G-Code word `N` (a.k.a. line number) may appear first on the line, it and
its literal numeric argument are parsed and ignored
* the G-Code word `O` (a.k.a. program number) may appear first on the line or
after `N`, it and its numeric argument will be parsed and an
`(argument, file_offset)` tuple shall be memorized for further use with `M98`
* the numeric arguments of `G` and `M` words are read and interpreted as
positive integers
* the numeric arguments of any other G-Code word are read and interpreted as
floating point values. Omitting the decimal point is permitted and interpreted
as that floating point number having no fractional part. Scientific notation is
not permitted.
* for the purpose of comparison, any two floating point numbers that are within
`0.0001` of each other are equal; this applies throughout (i.e. `0.00005` is
`false` for boolean operators)
* whenever a numerical argument denotes an angular measurement, it will be
interpreted as degrees (i.e. not radians, nor grads) and measured following
trigonometric conventions (i.e. 0 degrees is at 3 o'clock and positive is
counter clock-wise)
* most words behave like sticky registers which means not specifying a
particular word in the current block causes the interpreter to re-use the last
value given for that word in previous blocks

## Advanced Language Features

* parameter references via `#` can take the place of any numeric argument. This
includes the numeric argument of the `#` word itself (i.e. double indirection)
* parameter assignments are supported via the `=` operator; all assignments
encountered on one line (a.k.a. block) take effect on the conceptual boundary
between that line and the next one; in the case of multiple assignments to the
same parameter on the same line, the last one wins
* expressions enclosed in `[` and `]` can take the place of any numeric
argument; they are parsed left-to-right and evaluated recursively according to
the order of precedence of operators; all arithmetic and boolean operators in
the standard are supported as well as all mathematical functions defined
therein

## Parameter Behaviour

Apart from language-level support for reading and writing parameters, the
standard also defines specific behaviour for some parameters or parameter
ranges:

* parameter `#0` is read-only and always set to `0.0E+0`
* parameters `#1` to `#499` are volatile and will be reset to the value of `#0`
upon a machine restart
* parameters `#500` to `#5400` are persistent and will be saved and restored
across machine restarts

Furthermore, certain parameters are changed by the machine (i.e. independent of
any language constructs) when its state changes:

* (to be written)

Some parameters automatically mirror the arguments of recent words and are
updated at the conceptual boundary between two blocks:

* (to be written)

Finally, some parameters store machine configuration data that influences its
behaviour in addition to the G-Code words parsed:

* (to be written)

## G Word Commands

### G00 (rapid movement)

Moves the machine at the machine maximum speed to the target point given by the
axis words (i.e. `X`, `Y` and `Z`). The movement will be linearly interpolated
(i.e. all axes start and stop at the same time).

### G01 (linear interpolation)

The same as `G00` except the machine is moved at a speed given by the `F` word
and the current feed mode (i.e. `M93`, `M94` or `M95`).

### G02/03 (circular interpolation)

Moves the machine clock-wise for `G02` or counter clock-wise for `G03` in an arc
contained in the plane selected by `G17`, `G18` or `G19` to the target point
given by the axis words. Either the radius of the desired arc can be given via
the `R` word **or** the center of the arc can be given relative to the current
position using the center words (i.e. `I`, `J` and `K`). If the axis word for
the axis perpendicular to the currently selected plane is given, the resulting
machine move is helical.

### G04 (dwell and sequence point)

Blocks G-Code processing until all buffers run dry and the machine comes to a
complete stop, then waits for a number of seconds given by the `P` word.
Processing then resumes with the next block, all look-ahead and buffering logic
in the machine behaving as if it were the first one in the program.

### G09 (non-modal exact stop check)

Temporarily switches the machine's motion control strategy for the current block
to exact stop. This means the machine will come to a halt between moves,
effectively behaving as if it were executing separate single-move programs.

### G10/11 (data input/data cancel)

Starts reading in various configuration data (selected via the `L` and `P`
words) with `G10` and ends the stream with `G11`.

#### G10 L1 (set tool radius)

Updates the tool given by the `P` word to have the radius given by the `R` word.

#### G10 L2 (set work coordinate system)

Updates the WCS given by the `P` word to have the origin equal to the target
given by the axis words.

#### G10 L3 (set tool height and diameter)

Updates the tool given by the `P` word to have the diameter given by the `D`
word and the height given by the `H` word.

### G12/13 (full circle)

`G12` and `G13` are parsed but the corresponding machine movement functionality
is not implemented yet.

### G15/16 (cartesian/polar coordinates)

Axis words are to be interpreted as cartesian coordinates with `G15` or as polar
coordinates with `G16`. In the latter case, `X` is the radius relative to the
current position and `Y` is the angle.

### G17/18/19 (plane selection)

`G17` selects `XY` as the current plane, `G18` `ZX` and `G19` `YZ`.

### G20/21 (measurement units)

All numeric arguments that represent units of length are to be interpreted as
inches with `G20` and as millimeters with `G21`. Bear in mind the **mm** in
**mm/min** is a unit of length.

### G22/23 (coordinate-wise mirroring)

The axes corresponding to the given axis words are mirrored with `G22` around an
imaginary axis situated at the target given by the axis words.

For example (and assuming relative mode), `G22 X10` will enable mirroring of the
X axis with the mirroring origin at 10 units of length away from current
position on that axis. This further means that a subsequent commanded move of
`G01 X2` will result in a physical machine movement of 2 units of length
**away** from the mirroring axis (and towards the physical origin of X).