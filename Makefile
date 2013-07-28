# Part of gcode-canon
#
# Copyright (C) 2012 Radu - Eosif Mihailescu
#
# gcode-canon is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# gcode-canon is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with gcode-canon. If not, see <http://www.gnu.org/licenses/>.

SOURCES:=$(wildcard *.c)
OBJECTS:=$(patsubst %.c,%.o,$(SOURCES))
HEADERS:=$(wildcard *.h)

.PHONY:	all clean

all:	gcode-canon

clean:
	rm -f *.o gcode-canon

%.c:	$(HEADERS)

gcode-canon: $(OBJECTS)
	$(CC) $(OBJECTS) -lm -o gcode-canon
