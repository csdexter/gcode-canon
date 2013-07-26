(testing coordinate systems)
(machine setup)
M05 S600
M09
M23
M49
M69

(turn servos ON)
M17

(control setup)
G15
G17
G23
G40
G49
G50
G69

(in mm)
G21

(absolute coordinates)
G90

(rotation)
G01 X10 Y10 Z10 F600
X15
Y12 X8
X10 Y10
(rotate 90 degrees and do it again)
G68 R90
X15
Y12 X8
X10 Y10
(disable rotation)
G69

(and now for something more exotic)
(setup)
G01 X20 Y10 Z10
(first one)
G00 X25
G01 Y12.5 F600
X15
Y7.5
X25
Y10
G00 X20
(rotate 45 degrees around 10,10)
G68 X10 Y10 R45
(and again)
G00 X25
G01 Y12.5 F600
X15
Y7.5
X25
Y10
G00 X20
(rotate 120 degrees around 10,10)
G68 X10 Y10 R120
(and again)
G00 X25
G01 Y12.5 F600
X15
Y7.5
X25
Y10
G00 X20
(cancel rotation)
G69

M02