G-Code Canonical Interpreter
============================

This project is an attempt at 100% coverage of the existing G-Code standard. It aims to build a parser that can handle the entirety of RS724-NGC plus the most common extensions and additions to it, as encountered in the wild.

To make testing (and further reuse, the secondary aim of the project) straight forward, the interpreter comes with a virtual debugging CNC so that the whole implementation behaves as if there were an actual machine to control.