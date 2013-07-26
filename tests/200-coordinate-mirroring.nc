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

(and now for something more exotic)
X10 Y10 Z10
X20
G03 R10 X10 Y20
G01 Y10
(mirror in X and do it again)
M21
X20
G03 R10 X10 Y20
G01 Y10
(mirror in X and Y and do it again)
M22
X20
G03 R10 X10 Y20
G01 Y10
(mirror in Y and do it again)
M23
M22
X20
G03 R10 X10 Y20
G01 Y10
(mirror off)
M23


M02