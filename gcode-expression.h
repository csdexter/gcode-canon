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

#endif /* GCODE_EXPRESSION_H_ */
