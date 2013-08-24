/*
 ============================================================================
 Name        : gcode-expression.c
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2013-08-22)
 Copyright   : (C) 2013 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : G-Code Expression Evaluator Code
 ============================================================================
 */

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "gcode-commons.h"
#include "gcode-expression.h"
#include "gcode-debugcon.h"
#include "gcode-state.h"
#include "gcode-parameters.h"


static TGCodeExpressionPrecedence operatorPrecedence[] = {GCODE_EOP_THIRD,
                                                          GCODE_EOP_THIRD,
                                                          GCODE_EOP_THIRD,
                                                          GCODE_EOP_THIRD,
                                                          GCODE_EOP_THIRD,
                                                          GCODE_EOP_SECOND,
                                                          GCODE_EOP_SECOND,
                                                          GCODE_EOP_SECOND,
                                                          GCODE_EOP_FIRST};


static double _do_expression(char **expression); /* Forward declaration */

static double _do_function(char *fname, double arg1, double arg2) {
  if(!strcasecmp(fname, "ABS"))
    return fabs(arg1);
  if(!strcasecmp(fname, "ASIN"))
    return asin(arg1) * GCODE_RAD2DEG;
  if(!strcasecmp(fname, "ACOS"))
    return acos(arg1) * GCODE_RAD2DEG;
  if(!strcasecmp(fname, "ATAN"))
    return atan2(arg1, arg2) * GCODE_RAD2DEG;
  if(!strcasecmp(fname, "COS"))
    return cos(arg1 * GCODE_DEG2RAD);
  if(!strcasecmp(fname, "EXP"))
    return exp(arg1);
  if(!strcasecmp(fname, "FIX"))
    return floor(arg1);
  if(!strcasecmp(fname, "FUP"))
    return ceil(arg1);
  if(!strcasecmp(fname, "LN"))
    return log(arg1);
  if(!strcasecmp(fname, "ROUND"))
    return round(arg1);
  if(!strcasecmp(fname, "SIN"))
    return sin(arg1 * GCODE_DEG2RAD);
  if(!strcasecmp(fname, "SQRT"))
    return sqrt(arg1);
  if(!strcasecmp(fname, "TAN"))
    return tan(arg1 * GCODE_DEG2RAD);

  return 0;
}

static char *_next_token(char *expression, TGCodeExpressionToken *token) {
  double arg1, arg2;

  if(!*expression) {
    token->tType = GCODE_ETT_CLOSE;
    return expression;
  }

  switch(*(expression++)) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
      token->tType = GCODE_ETT_VALUE;
      token->tValue = read_gcode_real(expression - 1);
      expression = skip_gcode_digits(expression - 1);
      break;
    case '#':
      token->tType = GCODE_ETT_VALUE;
      if(*expression == '[') {
        expression++; /* skip the open bracket */

        token->tValue = fetch_parameter((uint16_t)_do_expression(&expression));
      } else {
        /* read_gcode_real handles parameter references on its own */
        token->tValue = read_gcode_real(expression - 1);
        expression = skip_gcode_digits(expression - 1);
      }
      break;
    case '[':
      token->tType = GCODE_ETT_OPEN;
      break;
    case ']':
      token->tType = GCODE_ETT_CLOSE;
      break;
    case '-':
      token->tType = GCODE_ETT_OPERATOR;
      token->tOperator.oType = GCODE_EO_MINUS;
      break;
    case '+':
      token->tType = GCODE_ETT_OPERATOR;
      token->tOperator.oType = GCODE_EO_PLUS;
      break;
    case '*':
      token->tType = GCODE_ETT_OPERATOR;
      if(*expression == '*') {
        token->tOperator.oType = GCODE_EO_POWER;
        expression++;
      }
      token->tOperator.oType = GCODE_EO_STAR;
      break;
    case '/':
      token->tType = GCODE_ETT_OPERATOR;
      token->tOperator.oType = GCODE_EO_SLASH;
      break;
    case 'A':
      if(strncasecmp(expression - 1, "ABS", strlen("ABS"))) {
        token->tType = GCODE_ETT_VALUE;
        expression += strlen("ABS"); /* Eat the opening bracket as well */
        token->tValue = _do_function("ABS", _do_expression(&expression), 0.0);
      }
      else if(strncasecmp(expression - 1, "ACOS", strlen("ACOS"))) {
        token->tType = GCODE_ETT_VALUE;
        expression += strlen("ACOS"); /* Eat the opening bracket as well */
        token->tValue = _do_function("ACOS", _do_expression(&expression), 0.0);
      }
      else if(strncasecmp(expression - 1, "AND", strlen("AND"))) {
        token->tType = GCODE_ETT_OPERATOR;
        token->tOperator.oType = GCODE_EO_AND;
        expression += strlen("AND") - 1;
      }
      else if(strncasecmp(expression - 1, "ASIN", strlen("ASIN"))) {
        token->tType = GCODE_ETT_VALUE;
        expression += strlen("ASIN"); /* Eat the opening bracket as well */
        token->tValue = _do_function("ACOS", _do_expression(&expression), 0.0);
      }
      else if(strncasecmp(expression - 1, "ATAN", strlen("ATAN"))) {
        token->tType = GCODE_ETT_VALUE;
        expression += strlen("ATAN"); /* Eat the opening bracket as well */
        arg1 = _do_expression(&expression); /* Enforce order of evaluation */
        /* Skip the slash between the arguments and the following opening
         * bracket. */
        expression += 2;
        arg2 = _do_expression(&expression);
        token->tValue = _do_function("ATAN", arg1, arg2);
      }
      break;
    case 'C':
      if(strncasecmp(expression - 1, "COS", strlen("COS"))) {
        token->tType = GCODE_ETT_VALUE;
        expression += strlen("COS"); /* Eat the opening bracket as well */
        token->tValue = _do_function("COS", _do_expression(&expression), 0.0);
      }
      break;
    case 'E':
      if(strncasecmp(expression - 1, "EXP", strlen("EXP"))) {
        token->tType = GCODE_ETT_VALUE;
        expression += strlen("EXP"); /* Eat the opening bracket as well */
        token->tValue = _do_function("EXP", _do_expression(&expression), 0.0);
      }
      break;
    case 'F':
      if(strncasecmp(expression - 1, "FIX", strlen("FIX"))) {
        token->tType = GCODE_ETT_VALUE;
        expression += strlen("FIX"); /* Eat the opening bracket as well */
        token->tValue = _do_function("FIX", _do_expression(&expression), 0.0);
      }
      else if(strncasecmp(expression - 1, "FUP", strlen("FUP"))) {
        token->tType = GCODE_ETT_VALUE;
        expression += strlen("FUP"); /* Eat the opening bracket as well */
        token->tValue = _do_function("FUP", _do_expression(&expression), 0.0);
      }
      break;
    case 'L':
      if(strncasecmp(expression - 1, "LN", strlen("LN"))) {
        token->tType = GCODE_ETT_VALUE;
        expression += strlen("LN"); /* Eat the opening bracket as well */
        token->tValue = _do_function("LN", _do_expression(&expression), 0.0);
      }
      break;
    case 'M':
      if(strncasecmp(expression - 1, "MOD", strlen("MOD"))) {
        token->tType = GCODE_ETT_OPERATOR;
        token->tOperator.oType = GCODE_EO_MOD;
        expression += strlen("MOD") - 1;
      }
      break;
    case 'O':
      if(strncasecmp(expression - 1, "OR", strlen("OR"))) {
        token->tType = GCODE_ETT_OPERATOR;
        token->tOperator.oType = GCODE_EO_OR;
        expression += strlen("OR") - 1;
      }
      break;
    case 'R':
      if(strncasecmp(expression - 1, "ROUND", strlen("ROUND"))) {
        token->tType = GCODE_ETT_VALUE;
        expression += strlen("ROUND"); /* Eat the opening bracket as well */
        token->tValue = _do_function("ROUND", _do_expression(&expression), 0.0);
      }
      break;
    case 'S':
      if(strncasecmp(expression - 1, "SIN", strlen("SIN"))) {
        token->tType = GCODE_ETT_VALUE;
        expression += strlen("SIN"); /* Eat the opening bracket as well */
        token->tValue = _do_function("SIN", _do_expression(&expression), 0.0);
      }
      else if(strncasecmp(expression - 1, "SQRT", strlen("SQRT"))) {
        token->tType = GCODE_ETT_VALUE;
        expression += strlen("SQRT"); /* Eat the opening bracket as well */
        token->tValue = _do_function("SQRT", _do_expression(&expression), 0.0);
      }
      break;
    case 'T':
      if(strncasecmp(expression - 1, "TAN", strlen("TAN"))) {
        token->tType = GCODE_ETT_VALUE;
        expression += strlen("TAN"); /* Eat the opening bracket as well */
        token->tValue = _do_function("TAN", _do_expression(&expression), 0.0);
      }
      break;
    case 'X':
      if(strncasecmp(expression - 1, "XOR", strlen("XOR"))) {
        token->tType = GCODE_ETT_OPERATOR;
        token->tOperator.oType = GCODE_EO_XOR;
        expression += strlen("XOR") - 1;
      }
      break;
  }

  if(token->tType == GCODE_ETT_OPERATOR)
    token->tOperator.oPrec = operatorPrecedence[token->tOperator.oType];

  return expression;
}

static double _do_operation(TGCodeExpressionOperator op, double left,
    double right) {
  switch(op.oType) {
    case GCODE_EO_PLUS:
      return left + right;
    case GCODE_EO_MINUS:
      return left - right;
    case GCODE_EO_AND:
      if(fabs(left) > 0.0001 && fabs(right) > 0.0001) return +1.0E+0;
      else return +0.0E+0;
    case GCODE_EO_OR:
      if(fabs(left) > 0.0001 || fabs(right) > 0.0001) return +1.0E+0;
      else return +0.0E+0;
    case GCODE_EO_XOR:
      if((fabs(left) > 0.0001 || fabs(right) > 0.0001) &&
        !(fabs(left) > 0.0001 && fabs(right) > 0.0001)) return +1.0E+0;
      else return +0.0E+0;
    case GCODE_EO_STAR:
      return left * right;
    case GCODE_EO_SLASH:
      return left / right;
    case GCODE_EO_MOD:
      return fmod(left, right);
    case GCODE_EO_POWER:
      return pow(left, right);
  }

  /* We should never get here */
  return NAN;
}

static double _do_expression(char **expression) {
  TGCodeExpressionToken token;
  double left = NAN, right = +0.0E+0;
  TGCodeExpressionOperator operator = {GCODE_EO_PLUS,
                                       operatorPrecedence[GCODE_EO_PLUS]};
  bool skipOp = false, skipRight = false;
  char *subexp = NULL;

  /* First argument */
  *expression = _next_token(*expression, &token);
  switch(token.tType) {
    case GCODE_ETT_OPEN:
      left = _do_expression(expression);
      break;
    case GCODE_ETT_OPERATOR:
      if(token.tOperator.oType == GCODE_EO_MINUS ||
         token.tOperator.oType == GCODE_EO_PLUS) {
        skipOp = true; /* We already know what the operator is */
        left = +0.0E+0;
        operator = token.tOperator;
      }
      break;
    case GCODE_ETT_VALUE:
      left = token.tValue;
      break;
  }

  while(**expression) {
    /* Save where right began, we may need to yield it */
    subexp = *expression;

    if(!skipOp) {
      /* Operator */
      *expression = _next_token(*expression, &token);
      switch(token.tType) {
        case GCODE_ETT_CLOSE:
          return left; /* Special case of a single value in brackets */
        case GCODE_ETT_OPERATOR:
          operator = token.tOperator;
          subexp = *expression; /* Normal case, update bookmark for right */
          break;
        case GCODE_ETT_OPEN:
        case GCODE_ETT_VALUE:
          /* Standard extension: math-like implicit multiplication */
          skipRight = true; /* We already know what right is */
          right = (token.tType == GCODE_ETT_OPEN ?
              _do_expression(expression) : token.tValue);
          operator.oType = GCODE_EO_STAR;
          operator.oPrec = operatorPrecedence[GCODE_EO_STAR];
          break;
      }
    } else skipOp = false;

    if(!skipRight) {
      /* Second argument */
      *expression = _next_token(*expression, &token);
      switch(token.tType) {
        case GCODE_ETT_OPEN:
        right = _do_expression(expression);
        break;
      case GCODE_ETT_VALUE:
        right = token.tValue;
        break;
      }
    } else skipRight = false;

    /* By this time, we have left, operator and right set properly. We should
     * now peek at the next token and decide what to do next. */
    _next_token(*expression, &token);
    /* If this is the end of the expression, we're done */
    if(token.tType == GCODE_ETT_CLOSE)
      return _do_operation(operator, left, right);
    /* If we're followed by an operation of a higher precedence, yield */
    if((token.tType == GCODE_ETT_OPERATOR &&
        token.tOperator.oPrec > operator.oPrec) ||
       ((token.tType == GCODE_ETT_OPEN || token.tType == GCODE_ETT_VALUE) &&
        operatorPrecedence[GCODE_EO_STAR] > operator.oPrec))
      return _do_operation(operator, left, _do_expression(&subexp));
    /* If we got to here, we're followed by an operation of equal or lower
     * precedence: reduce and repeat */
    left = _do_operation(operator, left, right);
    right = +0.0E+0;
    operator.oType = GCODE_EO_PLUS;
    operator.oPrec = operatorPrecedence[GCODE_EO_PLUS];
  }

  /* We should never get here */
  return NAN;
}

double evaluate_expression(char *expression) {
  return _do_expression(&expression);
}

void evaluate_unary_expression(char *line) {
  bool seenWord = false;
  char fname[strlen("ROUND") + 1], *sptr, *buf; /* longest named function */
  uint8_t fni;
  double arg, sar;

  while(*(line++)) {
    if(isdigit(*line) && seenWord) {
      line = skip_gcode_digits(line);
      seenWord = false;
    }
    if(isalpha(*line)) {
      if(seenWord) {
        sptr = line; /* Remember where it started */
        fni = 0;
        do { fname[fni++] = *line; } while (isalpha(*(++line)));
        fname[fni] = '\0'; /* Function name at fname */
        arg = read_gcode_real(line); /* Function (1st) argument in arg */
        line = skip_gcode_digits(line);
        if(*line == '/') {/* Special case of ATAN */
          sar = read_gcode_real(++line);
          line = skip_gcode_digits(line);
        }

        buf = (char *)calloc(1, strlen(line) + strlen("4.2f") + 1);
        strcpy(buf, "4.2f");
        strcat(buf, line); /* Save the rest of the line */
        line = sptr; /* Rewind to where the function started */
        /* Overwrite with numeric result */
        snprintf(line, strlen(line), buf, _do_function(fname, arg, sar));
        /* And go past it */
        line = skip_gcode_digits(line);
        free((void *)buf);
      }
      else seenWord = true;
    }
  }

  return;
}
