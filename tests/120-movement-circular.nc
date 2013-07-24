(testing rapid movements)
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
G90 F600
G01 X20 Y20 Z20
G02 R5 X30 Y20
G02 R10 X10 Y20

G01 X10 Y10 Z10
G03 R-5 Z5 X15 Y15

