#include "../lib/error.h"
#include "operator.h"

#define T Operator_t

String_t Operator_toString (T t)
{
  switch (t){
  case OP_ADD:
    return "+";
  case OP_SUB:
    return "-";
  case OP_TIMES:
    return "*";
  case OP_DIVIDE:
    return "/";
  case OP_MODUS:
    return "%";
  case OP_OR:
    return "||";
  case OP_AND:
    return "&&";
  case OP_EQ:
    return "==";
  case OP_NE:
    return "!=";
  case OP_LT:
    return "<";
  case OP_LE:
    return "<=";
  case OP_GT:
    return ">";
  case OP_GE:
    return ">=";
  case OP_NOT:
    return "!";
  case OP_NEG:
    return "-";
  default:
    fprintf (stderr, "%d\n", t);
    Error_impossible ();
    return 0;
  }
  Error_impossible ();
  return 0;
}

int Operator_binary (int left, T t, int right)
{
  switch (t){
  case OP_ADD:
    return left + right;
  case OP_SUB:
    return left - right;
  case OP_TIMES:
    return left * right;
  case OP_DIVIDE:
    return left / right;
  case OP_MODUS:
    return left % right;
  case OP_OR:
    if (left)
      return 1;
    return right;
  case OP_AND:
    if (left)
      return right;
    return 0;
  case OP_EQ:
    return left==right;
  case OP_NE:
    return left != right;
  case OP_LT:
    return left < right;
  case OP_LE:
    return left <= right;
  case OP_GT:
    return left > right;
  case OP_GE:
    return left >= right;
  case OP_NOT:
    Error_impossible ();
    return 0;
  case OP_NEG:
    Error_impossible ();
    return 0;
  default:
    Error_impossible ();
    return 0;
  }
  Error_impossible ();
  return 0;
}

int Operator_unary (T t, int x)
{
  switch (t){
  case OP_NOT:
    return !x;
  case OP_NEG:
    return -x;
  default:
    fprintf (stderr, "%d", t);
    Error_impossible ();
    return 0;
  }
  Error_impossible ();
  return 0;
}

#undef T
