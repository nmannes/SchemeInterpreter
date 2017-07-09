#ifndef _PRIMITIVES
#define _PRIMITIVES

// Function that binds scheme symbols to functions.
void bind(char *name, Value *(*function)(struct Value *), Frame *frame);

// Function that evaluates the first (and only) argument of args to test if it is
// of NULL_TYPE.
Value *primitiveNull(Value *args);

// Function that evaluates whether the argument is a BOOL_TYPE
Value *primitiveBool(Value *args);

// Function that takes a bool and flips its value
Value *primitiveNot(Value *args);

// Function which create a cons cell, with the first arg of args in the car and the
// second arg of args in the cdr.
Value *primitiveCons(Value *args);

// Function which returns the car of the cons cell contained in the first element of
// args
Value *primitiveCar(Value *args);

// Function which returns the cdr of the cons cell contained in the first element of
// args
Value *primitiveCdr(Value *args);

// Function that performs the modulo operator between the first and second dargs
Value *primitiveMod(Value *dargs);

// Function that adds an undetermined number of arguments
Value *primitiveAdd(Value *args);

// Function which takes the int located at the first arg of args and subtracts
// from it all ints in proceeding args.
Value *primitiveSubtract(Value *args);

// Function which returns the product of all args in args
Value *primitiveMult(Value *args);

// Function which returns the quotient of the first and second args
Value *primitiveDivide(Value *args);

// Function which takes two numeric args and tests if they are equal
Value *primitiveEqual(Value *args);

// Function which takes two numeric args and tests if the first is greater than the
// second
Value *primitiveGreaterThan(Value *args);

// Function which takes two numeric args and tests if the first is less than the second
Value *primitiveLessThan(Value *args);

// Function which takes two numeric args and tests if the first is greater than or equal
// to the second
Value *primitiveGeq(Value *args);

// Function which takes two numeric args and test if the first is less than or equal to
// the second
Value *primitiveLeq(Value *args);

// Prints args
Value *primitivePrint(Value *args);

#endif
