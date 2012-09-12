/*
 ============================================================================
 Name        : gcode-canon.c
 Author      : Radu - Eosif Mihailescu
 Version     :
 Copyright   : Copyright (C) 2012 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include "gcode-commons.h"
#include "gcode-parameters.h"
#include "gcode-tools.h"
#include "gcode-input.h"
#include "gcode-machine.h"

int main(void) {
	FILE *parFile = fopen(GCODE_PARAMETER_STORE, "rm");
	char line[0xFF];

	init_machine(NULL);
	init_parameters(parFile);
	init_tools(NULL);
	init_input(stdin);
	init_gcode_state(NULL);

	while(fetch_line_input(line)) update_gcode_state(line);

	done_input();
	done_tools();
	done_parameters();
	done_machine();

	return 0;
}
