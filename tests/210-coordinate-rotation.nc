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


M02