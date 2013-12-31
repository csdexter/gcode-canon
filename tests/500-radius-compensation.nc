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

(we need a tool selected for radius compensation to make sense)
M06 T1 (test output will vary by D of T1)
(initial move to prime buffer)
G01 X10 Y10 F100

(activate left and initial move to set direction)
G41 Y50
(keep going)
X50
Y10
(stub move to clear the corner in a compensated way)
X45
(deactivate and return)
G40 X5 Y5

(initial move)
X10 Y10
(activate right and initial move to set direction)
G42 Y50
(keep going)
X50
Y10
(stub move to clear the corner in a compensated way)
X45
(deactivate and return)
G40 X5 Y5

M2 (THE END)
