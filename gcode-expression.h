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

#include <stdint.h>


#define GCODE_ETT_VALUE 0x01
#define GCODE_ETT_OPERATOR 0x02
#define GCODE_ETT_OPEN 0x03
#define GCODE_ETT_CLOSE 0x04

typedef enum {
  GCODE_EO_PLUS = 0,
  GCODE_EO_MINUS,
  GCODE_EO_AND,
  GCODE_EO_OR,
  GCODE_EO_XOR,
  GCODE_EO_STAR,
  GCODE_EO_SLASH,
  GCODE_EO_MOD,
  GCODE_EO_POWER,
} TGCodeExpressionOperators;

typedef enum {
  GCODE_EOP_THIRD,
  GCODE_EOP_SECOND,
  GCODE_EOP_FIRST
} TGCodeExpressionPrecedence;

typedef struct {
  TGCodeExpressionOperators oType;
  TGCodeExpressionPrecedence oPrec;
} TGCodeExpressionOperator;

typedef struct {
  uint8_t tType;
  union {
    TGCodeExpressionOperator tOperator;
    double tValue;
  };
} TGCodeExpressionToken;

/* Evaluates expression according to G-Code expression grammar and returns the
 * numeric result. */
double evaluate_expression(char *expression);
/* Scan line for function names, evaluate and replace them and their arguments
 * in line with the numeric result. Used for unary expressions without brackets
 * i.e. "G01 XSIN10 YATAN9/14" */
void evaluate_unary_expression(char *line);

#endif /* GCODE_EXPRESSION_H_ */
