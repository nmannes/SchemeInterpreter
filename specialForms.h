#ifndef _SPECIALFORMS
#define _SPECIALFORMS

//Looks through the frame structure for a given symbol value
Value *lookUpSymbol(Value *symbol, Frame *frame);

//Evaluates the car of the tree and short circuits based on true or false
Value *evalIf(Value *tree, Frame *frame);

//Places the bindings that are in car(tree) into a frame,
//and then evaluates the code from that follows
Value *evalLet(Value *tree, Frame *frame);

//Places each binding into its own frame, and points the frame at the previous binding's frame
//and then evaluates the code that follows
Value *evalLetStar(Value *args, Frame *frame);

//Initializes each variable binding in a frame, and then evaluates each
//expression and puts it in the appropriate frame
Value *evalLetRec(Value *args, Frame *frame);

//Evaluates each expression inputted.
//Short circuits and returns #f if anything evaluates to #f
//Expressions that return VOID_TYPEs are also evaluated
Value *evalAnd(Value *args, Frame *frame);

//Evaluates each expression inputted.
//Short circuits and returns #t if anything evaluates to #t
//Expressions that return VOID_TYPEs are also evaluated
Value *evalOr(Value *args, Frame *frame);

//Looks through the frame for the first instance
//of the symbol type that is the car(args)
//And then replaces it with the evaluated second arg-
//-ument
Value *evalSetBang(Value *args, Frame *frame);

//Evaluates each scheme expression inputted in the frame
//and then returns the evaluated last argument
Value *evalBegin(Value *args, Frame *frame);

//Expects a list of the form
//(cond ((<boolean expression>) (<expression>))...)
//If a boolean expression evalutates to true, it short
// circuits and return the appropriate expression
Value *evalCond(Value *args, Frame *frame);

//Given a list of expressions,
//Evaluates every list with the inputted frame
//It returns the eval of the last expression
Value *evalEach(Value *args, Frame *frame);

//Given a primitive function pointer, it calls that
//function via a function pointer in prim
Value *evalPrimitive(Value *prim, Value *args);

//Binds the inputted args to the given arguments for
//the closure
//Then it evaluates the closure from the frame
//it was initialized with
Value *evalClosure(Value *closure, Value *args);

//Checks whether or not the first arg is a PRIMITIVE_TYPE
//or a lambda expression, and then calls the appropriate function
Value *apply(Value *first, Value *args);

#endif
