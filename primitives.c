#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "value.h"
#include "interpreter.h"
#include "talloc.h"
#include "linkedlist.h"
#include "parser.h"
#include "tokenizer.h"
#include "primitives.h"

// Function that binds scheme symbols to functions
void bind(char *name, Value *(*function)(struct Value *), Frame *frame) {
  // Add primitive functions to top-level bindings list
  Value *value = talloc(sizeof(Value));
  value->type = PRIMITIVE_TYPE;
  value->pf = function;
  Value *nameVal = talloc(sizeof(Value));
  nameVal->type = SYMBOL_TYPE;
  nameVal->s = name;
  frame->bindings = cons(nameVal, cons(value, frame->bindings));
}

// Function that evaluates the first (and only) argument of args to test if it is
// of NULL_TYPE
Value *primitiveNull(Value *args){
  if(length(args) != 1){
    printf("SYNTAX ERROR: Function null? takes one argument!\n");
    texit(1);
  }
  Value *boo = makeNull();
  boo->type = BOOL_TYPE;
  if(car(args)->type == NULL_TYPE){
    boo->i = 1;
  } else {
    boo->i = 0;
  }
  return boo;
}

// Function that evaluates whether the argument is a BOOL_TYPE
Value *primitiveBool(Value *args){
  Value *isBool = makeNull();
  isBool->type = BOOL_TYPE;
  if(length(args) != 1){
    printf("Syntax Error: boolean? needs 1 arguments but you had %i\n",
	   length(args));
    texit(1);
  }
  isBool->i = (car(args)->type == BOOL_TYPE);
  return isBool;
}

// Function that takes a bool and flips its value
Value *primitiveNot(Value *args){
  Value *newBool;
  if(car(args)->type == BOOL_TYPE){
    newBool = makeNull();
    newBool->type = BOOL_TYPE;
    newBool->i = !(car(args)->i);
  } else {
    printf("Syntax Error: not takes a boolean as an argument.\n");
    texit(1);
  }
  return newBool;
}

// Function which create a cons cell, with the first arg of args in the car and the
// second arg of args in the cdr.
Value *primitiveCons(Value *args){
  if(length(args) != 2){
    printf("Syntax Error: Cons needs 2 arguments but you had %i\n",
	   length(args));
    texit(1);
  }

  return cons(car(args), car(cdr(args)));
}

// Function which returns the car of the cons cell contained in the first element of
// args
Value *primitiveCar(Value *args){
  if(length(args) != 1){
    printf("Syntax Error: Car needs 1 argument but you had %i\n",
	   length(args));
    texit(1);
  }
  return car(car(args));
}

// Function which returns the cdr of the cons cell contained in the first element of
// args
Value *primitiveCdr(Value *args){
  if(length(args) != 1){
    printf("Syntax Error: Car needs 1 argument but you had %i\n",
	   length(args));
    texit(1);
  }
  return cdr(car(args));
}

// Function that performs the modulo operator between the first and second dargs
Value *primitiveMod(Value *dargs){
  if(length(dargs) != 2){
    printf("Syntax Error: Modulo needs 2 arguments but you had %i\n"
	   , length(dargs));
    texit(1);
  }
  if(car(dargs)->type != INT_TYPE && car(cdr(dargs))->type != INT_TYPE){
    printf("Syntax Error: Modulo only accepts integers as arguments\n");
    texit(1);
  }
  Value *val = makeNull();
  val->type = INT_TYPE;
  val->i = car(dargs)->i % car(cdr(dargs))->i;
  return val;
}

// Function that adds an undetermined number of arguments
Value *primitiveAdd(Value *args){
  double sum = 0;
  int isDouble = 0;
  while(args->type != NULL_TYPE){
    if(car(args)->type == INT_TYPE){
      sum += (double) car(args)->i;
    }
    else if(car(args)->type == DOUBLE_TYPE){
      sum += car(args)->d;
      isDouble = 1;
    }
    else{
      printf("SYNTAX ERROR: you trying to add things that aren't numbers\n");
      texit(1);
    }
    args = cdr(args);
  }
  Value *val = makeNull();
  if(isDouble){
    val->type = DOUBLE_TYPE;
    val->d = sum;
    return val;
  }
  val->type = INT_TYPE;
  val->i = (int) sum;
  return val;
}

// Function which takes the int located at the first arg of args and subtracts
// from it all ints in proceeding args.
Value *primitiveSubtract(Value *args){
  if(length(args) != 2 ) {
    printf("need 2 args for subtraction to work\n");
    texit(1);
  }

  double diff;
  if(car(args)->type == DOUBLE_TYPE){
    diff = car(args)->d;
  } else if(car(args)->type == INT_TYPE){
    diff = (double) car(args)->i;
  } else {
    printf("SYNTAX ERROR: you trying to subtract things that aren't numbers :/\n");
    texit(1);
  }

  int isDouble = 0;
  if(car(cdr(args))->type == INT_TYPE){
    diff -= car(cdr(args))->i;
  }
  else if(car(cdr(args))->type == DOUBLE_TYPE){
    diff -= car(cdr(args))->d;
    isDouble = 1;
  }
  else{
    printf("SYNTAX ERROR: you be trying to subtract things that aren't numbers\n");
    texit(1);
  }
  Value *valhalla = makeNull();
  if(isDouble){
    valhalla->type = DOUBLE_TYPE;
    valhalla->d = diff;
  }else{
    valhalla->type = INT_TYPE;
    valhalla->i = (int) diff;
  }
  return valhalla;
}

// Function which returns the product of all args in args
Value *primitiveMult(Value *args){
  double product = 1;
  int isDouble = 0;
  while(args->type != NULL_TYPE){
    if(car(args)->type == INT_TYPE){
      product = product * car(args)->i;
    }
    else if(car(args)->type == DOUBLE_TYPE){
      product = product * car(args)->d;
      isDouble = 1;
    }
    else{
      printf("SYNTAX ERROR: you trying to multiply things that aren't numbers\n");
      texit(1);
    }
    args = cdr(args);
  }
  Value *val = makeNull();
  if(isDouble){
    val->type = DOUBLE_TYPE;
    val->d = product;
    return val;
  }
  val->type = INT_TYPE;
  val->i = (int) product;
  return val;
}

// Function which returns the quotient of the first and second args
Value *primitiveDivide(Value *args){
  if(length(args) != 2){
    printf("SYNTAX ERROR: Divide only takes two args, sorry.\n");
    texit(1);
  }
  double quotient = 0;
  int isDouble = 0;
  if(car(args)->type == DOUBLE_TYPE){
    quotient = car(args)->d;
    isDouble = 1;
  } else if(car(args)->type == INT_TYPE){
    quotient = car(args)->i;
  } else {
    printf("SYNTAX ERROR: Divide only accepts numbers.\n");
    texit(1);
  }

  if(car(cdr(args))->type == DOUBLE_TYPE){
    quotient /= car(cdr(args))->d;
    isDouble = 1;
  } else if(car(cdr(args))->type == INT_TYPE){
    quotient /= car(cdr(args))->i;
  } else {
    printf("SYNTAX ERROR: Divide only accepts numbers.\n");
    texit(1);
  }

  int modOkay = 0;
  if(!isDouble){
    if(car(args)->i % car(cdr(args))->i == 0){
      quotient = (int) quotient;
      modOkay = 1;
    }
  }

  Value *valhalla = makeNull();
  if(isDouble || !modOkay){
    valhalla->type = DOUBLE_TYPE;
    valhalla->d = quotient;
  } else {
    valhalla->type = INT_TYPE;
    valhalla->i = quotient;
  }
  return valhalla;

}


// Function which takes two numeric args and tests if they are equal
Value *primitiveEqual(Value *args){
  Value *val;
  if(length(args) != 2){
    printf("Syntax Error: = needs 2 arguments but you had %i\n"
	   , length(args));
    texit(1);
  }
  if(
     !((car(args)->type == INT_TYPE ||
	car(args)->type == DOUBLE_TYPE)
     &&
       (car(cdr(args))->type == INT_TYPE ||
	car(cdr(args))->type == DOUBLE_TYPE)
      )
     )
    {
     printf("Syntax Error: = only accepts integers and doubles as arguments\n");
    texit(1);
  }
  else{
    double arg1, arg2;
    if(car(args)->type == INT_TYPE){
      arg1 = (double) car(args)->i;
    }else{
      arg1 = car(args)->d;
    }
    if(car(cdr(args))->type == INT_TYPE){
      arg2 = (double) car(cdr(args))->i;
    }else{
      arg2 = car(cdr(args))->d;
    }

    val = makeNull();
    val->type = BOOL_TYPE;
    val->i = (arg1 == arg2);
  }
  return val;
}

// Function which takes two numeric args and tests if the first is greater than the
// second
Value *primitiveGreaterThan(Value *args){
  Value *val;
  if(length(args) != 2){
    printf("Syntax Error: > needs 2 arguments but you had %i\n"
	   , length(args));
    texit(1);
  }
  if(
     !((car(args)->type == INT_TYPE ||
	car(args)->type == DOUBLE_TYPE)
     &&
       (car(cdr(args))->type == INT_TYPE ||
	car(cdr(args))->type == DOUBLE_TYPE)
      )
     )
    {
     printf("Syntax Error: > only accepts integers and doubles as arguments\n");
    texit(1);
  }
  else{
    double arg1, arg2;
    if(car(args)->type == INT_TYPE){
      arg1 = (double) car(args)->i;
    }else{
      arg1 = car(args)->d;
    }
    if(car(cdr(args))->type == INT_TYPE){
      arg2 = (double) car(cdr(args))->i;
    }else{
      arg2 = car(cdr(args))->d;
    }

    val = makeNull();
    val->type = BOOL_TYPE;
    val->i = (arg1 > arg2);
  }
  return val;
}

// Function which takes two numeric args and tests if the first is less than the second
Value *primitiveLessThan(Value *args){
  Value *val;
  if(length(args) != 2){
    printf("Syntax Error: = needs 2 arguments but you had %i\n"
	   , length(args));
    texit(1);
  }
  if(
     !((car(args)->type == INT_TYPE ||
	car(args)->type == DOUBLE_TYPE)
     &&
       (car(cdr(args))->type == INT_TYPE ||
	car(cdr(args))->type == DOUBLE_TYPE)
      )
     )
    {
     printf("Syntax Error: < only accepts integers and doubles as arguments\n");
    texit(1);
  }
  else{
    double arg1, arg2;
    if(car(args)->type == INT_TYPE){
      arg1 = (double) car(args)->i;
    }else{
      arg1 = car(args)->d;
    }
    if(car(cdr(args))->type == INT_TYPE){
      arg2 = (double) car(cdr(args))->i;
    }else{
      arg2 = car(cdr(args))->d;
    }

    val = makeNull();
    val->type = BOOL_TYPE;
    val->i = (arg1 < arg2);
  }
  return val;
}

// Function which takes two numeric args and tests if the first is greater than or equal
// to the second
Value *primitiveGeq(Value *args){
  Value *val;
  if(length(args) != 2){
    printf("Syntax Error: >= needs 2 arguments but you had %i\n"
	   , length(args));
    texit(1);
  }
  if(
     !((car(args)->type == INT_TYPE ||
	car(args)->type == DOUBLE_TYPE)
     &&
       (car(cdr(args))->type == INT_TYPE ||
	car(cdr(args))->type == DOUBLE_TYPE)
      )
     )
    {
     printf("Syntax Error: >= only accepts integers and doubles as arguments\n");
    texit(1);
  }
  else{
    double arg1, arg2;
    if(car(args)->type == INT_TYPE){
      arg1 = (double) car(args)->i;
    }else{
      arg1 = car(args)->d;
    }
    if(car(cdr(args))->type == INT_TYPE){
      arg2 = (double) car(cdr(args))->i;
    }else{
      arg2 = car(cdr(args))->d;
    }

    val = makeNull();
    val->type = BOOL_TYPE;
    val->i = (arg1 >= arg2);
  }
  return val;
}

// Function which takes two numeric args and test if the first is less than or equal to
// the second
Value *primitiveLeq(Value *args){
  Value *val;
  if(length(args) != 2){
    printf("Syntax Error: <= needs 2 arguments but you had %i\n"
	   , length(args));
    texit(1);
  }
  if(
     !((car(args)->type == INT_TYPE ||
	car(args)->type == DOUBLE_TYPE)
     &&
       (car(cdr(args))->type == INT_TYPE ||
	car(cdr(args))->type == DOUBLE_TYPE)
      )
     )
    {
     printf("Syntax Error: <= only accepts integers and doubles as arguments\n");
    texit(1);
  }
  else{
    double arg1, arg2;
    if(car(args)->type == INT_TYPE){
      arg1 = (double) car(args)->i;
    }else{
      arg1 = car(args)->d;
    }
    if(car(cdr(args))->type == INT_TYPE){
      arg2 = (double) car(cdr(args))->i;
    }else{
      arg2 = car(cdr(args))->d;
    }

    val = makeNull();
    val->type = BOOL_TYPE;
    val->i = (arg1 <= arg2);
  }
  return val;
}


Value *primitivePrint(Value *args){
  if(length(args) != 1){
    printf("print takes 1 arg\n");
    texit(1);
  }
  print(car(args));
  Value *voidVal = makeNull();
  voidVal->type = VOID_TYPE;
  return voidVal;
}
