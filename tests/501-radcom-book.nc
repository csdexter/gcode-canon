(testing radius compensation)
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
G90

(in mm)
G21

(we need to either have a tool selected or specify one below for radius compensation to make sense)
(test output will vary by D of T2)

(below code taken verbatim from Peter Smid's CNC Programming Handbook)
G17 G40 G80
G90 G54 G00 X-0.625 Y-0.625 S920 M03
G43 Z1.0 H02
G01 Z-0.55 F25.0 M08
G41 X0 D02 F15.0
Y1.125
X2.25 Y1.8561
Y0.625
G02 X1.625 Y0 R0.625
G01 X-0.625
G00 G40 Y-0.625
Z1.0 M09
(end verbatim)
G00 X-0.625 Y-0.625 Z1.0

M2 (THE END)
