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

(scaling)
G01 X10 Y10 Z10 F600
X15
Y12 X8
X10 Y10
(scale by 2 around 11,11 and do it again)
G51 X11 Y11 Z10 I2 J2 K1
X15
Y12 X8
X10 Y10
(scale by 0.5 around 11,11 and do it again)
G51 X11 Y11 Z10 I0.5 J0.5 K1
X15
Y12 X8
X10 Y10
(disable scaling)
G50

M02