/*
 ============================================================================
 Name        : gcode-expression.h
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2013-08-22)
 Copyright   : (C) 2013 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : G-Code Expression Evaluator API Header
 ============================================================================
 */

#ifndef GCODE_EXPRESSION_H_
#define GCODE_EXPRESSION_H_


/* Evaluates expression according to G-Code expression grammar and returns the
 * numeric result. */
double evaluate_expression(const char *expression);
/* Scan line for function names, evaluate and replace them and their arguments
 * in line with the numeric result. Used for unary expressions without brackets
 * i.e. "G01 XSIN10 YATAN9/14" */
void evaluate_unary_expression(char *line);

#endif /* GCODE_EXPRESSION_H_ */
