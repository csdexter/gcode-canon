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
(polar coordinates)
G90
G15 G01 X0.000 Y0.000 Z0.000 F600
X10
Y15
G16 X5 Y45
X10 Y135

(machine mirroring)
G15 X10 Y10 Z10
X15
Y12 X8
X10 Y10
(mirror X axis)
M21
X15
Y12 X8
X10 Y10
(no mirror)
M23

M02