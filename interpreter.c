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
#include "specialForms.h"
Frame *globalFrame;

// Adds bindings to the global frame
Value *defineBinding(Frame *frame, Value *bindingPair){
    frame->bindings = cons(eval(car(cdr(bindingPair)), frame), frame->bindings);
    frame->bindings = cons(car(bindingPair), frame->bindings);

    Value *define = makeNull();
    define->type = VOID_TYPE;
    return define;
}

//This creates the global frame
Frame *globalFrameCreator(){
  globalFrame = talloc(sizeof(Frame));
  globalFrame->parent = NULL;
  Value *bindings = makeNull();
  globalFrame->bindings = bindings;
  return globalFrame;
}

// Helper function for printing improper lists
void print(Value *tree){
  if(tree->type == CONS_TYPE && tree->c.cdr->type != CONS_TYPE && tree->c.car->type != CONS_TYPE){
    display(tree->c.car);
    printf(".");
    display(tree->c.cdr);
  }
  else{
    printTree(tree);
  }
}

// Takes a parse tree and prints the appropriate output
void interpret(Value *tree){
  globalFrame = globalFrameCreator();
  bind("+", primitiveAdd, globalFrame);
  bind("-", primitiveSubtract, globalFrame);
  bind("*", primitiveMult, globalFrame);
  bind("/", primitiveDivide, globalFrame);
  bind("modulo", primitiveMod, globalFrame);
  bind("=", primitiveEqual, globalFrame);
  bind(">", primitiveGreaterThan, globalFrame);
  bind("<", primitiveLessThan, globalFrame);
  bind(">=", primitiveGeq, globalFrame);
  bind("<=", primitiveLeq, globalFrame);
  bind("null?", primitiveNull, globalFrame);
  bind("boolean?", primitiveBool, globalFrame);
  bind("not", primitiveNot, globalFrame);
  bind("cons", primitiveCons, globalFrame);
  bind("car", primitiveCar, globalFrame);
  bind("cdr", primitiveCdr, globalFrame);
  bind("print", primitivePrint, globalFrame);
  Value *evaluate;
  while(tree->type != NULL_TYPE){
	evaluate = eval(car(tree), globalFrame);
	print(evaluate);
    if(evaluate->type != VOID_TYPE){
      printf("\n");
    }
    tree = cdr(tree);
  }
}

// Evaluates the Value tree within the given frame.
Value *eval(Value *tree, Frame *frame) {
  switch (tree->type)  {
  case INT_TYPE: {
    return tree;
  }
  case DOUBLE_TYPE: {
    return tree;
  }
  case STR_TYPE: {
    return tree;
  }
  case NULL_TYPE: {
    return tree;
  }
  case BOOL_TYPE: {
    return tree;
  }
  case SYMBOL_TYPE: {
    return lookUpSymbol(tree, frame);
  }
  case CLOSURE_TYPE:{
    return tree;
  }
  case CONS_TYPE: {

    Value *first = car(tree);
    if(first->type == CONS_TYPE){
      first = eval(first, frame);
    }
    if(first->type == NULL_TYPE){
      return first;
    }
    //makes sure that the first thing is actually a function
    if(first->type != SYMBOL_TYPE && first->type != CLOSURE_TYPE &&
      first->type != PRIMITIVE_TYPE){
        printf("Input Error: ");
        display(first);
        printf(" is not a valid function\n");
        texit(1);

      }
    Value *args = cdr(tree);

    if (!strcmp(first->s,"if")) {
      return evalIf(args,frame);
    }
    else if (!strcmp(first->s,"and")) {
      return evalAnd(args,frame);
    }
    else if (!strcmp(first->s,"or")) {
      return evalOr(args,frame);
    }
    else if (!strcmp(first->s,"let")) {
      return evalLet(args,frame);
    }
    else if (!strcmp(first->s,"let*")) {
      return evalLetStar(args,frame);
    }
    else if (!strcmp(first->s,"letrec")) {
      return evalLetRec(args, frame);
    }
    else if (!strcmp(first->s,"cond")) {
      return evalCond(args, frame);
    }
    else if (!strcmp(first->s,"begin")) {
      return evalBegin(args, frame);
    }
    else if (!strcmp(first->s,"set!")) {
      return evalSetBang(args, frame);
    }
    else if (!strcmp(first->s, "quote")){
      if(length(args) != 1){
	printf("quote needs to have 2 arguments, you have %i\n", length(args));
	texit(1);
      }
      return car(args);
    }
    else if (!strcmp(first->s, "define")){
      if(length(args) != 2){
	       printf("define needs to have 2 arguments, you have %i\n", length(args));
	       texit(1);
      }
      return defineBinding(frame, args);
    }
    else if (!strcmp(first->s, "lambda")){
      Value *closure = makeNull();
      closure->type = CLOSURE_TYPE;
      closure->cl.paramNames = car(args);
      closure->cl.functionCode = cdr(args);
      closure->cl.frame = frame;

      return closure;
    }
    else {
      Value *evaledOperator = eval(first, frame);
      Value *evaledArgs = evalEach(args, frame);
      return apply(evaledOperator, evaledArgs);

    }
  }
  default:{
    printf("ERROR: YOU TRIED TO EVALUATE A TYPE: %i\n THAT THIS INTERPRETER IS NOT DESIGNED TO EVALUATE\n", tree->type);
    texit(1);
    return tree;
  }
  }
}
