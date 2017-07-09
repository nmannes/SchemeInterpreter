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

//This is an error check we use in a lot of let statements and it was getting repetitive
//so we made it a function
void letStarErrorCheck(Value *pair){
  if(pair->type != CONS_TYPE){
        printf("Syntax Error: Not a valid binding ->");
        printTree(pair);
        printf("\n");
        texit(1);
      }
      if(length(pair) != 2){
        printf("Syntax Error: Bindings need to length 2 -> ");
        printTree(pair);
        printf("\n");
        texit(1);
      }
      if(car(pair)->type != SYMBOL_TYPE){
        printf("Interpreter Error: YOU CANNOT ASSIGN A VALUE TO\n SOMETHING THAT IS NOT A SYMBOL");
        texit(1);
      }
}

//Looks through the frame structure for a given symbol value
Value *lookUpSymbol(Value *symbol, Frame *frame){
  Value *bindings = frame->bindings;
  while(bindings->type != NULL_TYPE){
    if(!strcmp(symbol->s, car(bindings)->s)){
      return car(cdr(bindings));
    }
    bindings = cdr(cdr(bindings));
  }
  if(frame->parent == NULL){
    printf("Interpeter Error: Unidentified symbol -\"%s\"\n", symbol->s);
    texit(1);
  }
  return lookUpSymbol(symbol, frame->parent);
}

//Evaluates the car of the tree and short circuits based on true or false
Value *evalIf(Value *tree, Frame *frame){
  assert(length(tree) == 3);
    assert(eval(car(tree), frame)->type == BOOL_TYPE);
  if(eval(car(tree), frame)->i){
    return eval(car(cdr(tree)), frame);
  }
  else{
    return eval(car(cdr(cdr(tree))), frame);
  }
}

//Places the bindings that are in car(tree) into a frame,
//and then evaluates the code from that follows
Value *evalLet(Value *tree, Frame *frame){
  if(length(tree) != 2){
    printf("let needs 2 args, you have %i\n", length(tree));
    texit(1);
  }
  Frame *newFrame = talloc(sizeof(Frame));
  newFrame->bindings = makeNull();
  newFrame->parent = frame;
  Value *bindings = car(tree);
  while(bindings->type != NULL_TYPE){
    Value *pair = car(bindings);
    letStarErrorCheck(pair);
    newFrame->bindings = cons(eval(car(cdr(pair)), frame),
			      newFrame->bindings);
    newFrame->bindings = cons(car(pair), newFrame->bindings);
    bindings = cdr(bindings);
  }
  return eval(car(cdr(tree)), newFrame);
}

//Places each binding into its own frame, and points the frame at the previous binding's frame
//and then evaluates the code that follows
Value *evalLetStar(Value *tree, Frame *frame){
  if(length(tree) != 2){
    printf("let needs 2 args, you have %i\n", length(tree));
    texit(1);
  }
  Frame *newFrame = talloc(sizeof(Frame));
  newFrame->bindings = makeNull();
  newFrame->parent = frame;
  Value *bindings = car(tree);
  while(bindings->type != NULL_TYPE){
    Value *pair = car(bindings);
    letStarErrorCheck(pair);
    newFrame->bindings = cons(eval(car(cdr(pair)), newFrame),
			      newFrame->bindings);
    newFrame->bindings = cons(car(pair), newFrame->bindings);
    bindings = cdr(bindings);
  }
  return evalBegin(cdr(tree), newFrame);
}

//Returns a boolean Value that evaluates to #f
Value *createFalse(){
  Value *val = makeNull();
  val->type = BOOL_TYPE;
  val->i = 0;
  return val;
}

//Takes the input for letrec, and then
//creates a dummy frame where every binding in letrec
//points to a boolean false value
Frame *createVarFrame(Value *args, Frame *frame){
  Frame *newFrame = talloc(sizeof(Frame));
  newFrame->parent = frame;
  newFrame->bindings = makeNull();
  Value *bindings = car(args);
  Value *pair;
  while(bindings->type != NULL_TYPE){
    pair = car(bindings);
    letStarErrorCheck(pair);
    newFrame->bindings = cons(createFalse(), newFrame->bindings);
    newFrame->bindings = cons(car(pair), newFrame->bindings);
    bindings = cdr(bindings);
  }
  return newFrame;
}

//Initializes each variable binding in a frame, and then evaluates each
//expression and puts it in the appropriate frame
Value *evalLetRec(Value *tree, Frame *frame){
  if(length(tree) != 2){
    printf("let needs 2 args, you have %i\n", length(tree));
    texit(1);
  }
  //First we make a dummy frame that has all the right variable names
  Frame *varFrame = createVarFrame(tree, frame);
  Value *bindings = car(tree);
  Value *pair;
  Value *bang;
  while(bindings->type != NULL_TYPE){
    pair = car(bindings);
    bang = makeNull();
    bang = cons(eval(car(cdr(pair)), varFrame), bang);
    bang = cons(car(pair), bang);
    evalSetBang(bang, varFrame);
    bindings = cdr(bindings);
  }
  return evalBegin(cdr(tree), varFrame);

}

//Evaluates each expression inputted.
//Short circuits and returns #f if anything evaluates to #f
//Expressions that return VOID_TYPEs are also evaluated
Value *evalAnd(Value *args, Frame *frame){
  Value *temp;
  for(int i = length(args); i > 0; i--){
    temp = eval(car(args), frame);
    if(temp->type != BOOL_TYPE && temp->type != VOID_TYPE){
      printf("Arguments of and need to be boolean expressions\n");
      texit(1);
    }
    if(temp->type == BOOL_TYPE && !temp->i){
      return temp;
    }
    args = cdr(args);
  }
  temp = makeNull();
  temp->type = BOOL_TYPE;
  temp->i = 1;
  return temp;
}

//Evaluates each expression inputted.
//Short circuits and returns #t if anything evaluates to #t
//Expressions that return VOID_TYPEs are also evaluated
Value *evalOr(Value *args, Frame *frame){
  Value *temp;
  for(int i = length(args); i > 0; i--){
    temp = eval(car(args), frame);
    if(temp->type != BOOL_TYPE && temp->type != VOID_TYPE){
      printf("Arguments of or need to be boolean expressions\n");
      texit(1);
    }
    if(temp->type == BOOL_TYPE && temp->i){
      return temp;
    }
    args = cdr(args);
  }
  temp = makeNull();
  temp->type = BOOL_TYPE;
  temp->i = 0;
  return temp;
}

//Given a list of expressions,
//Evaluates every list with the inputted frame
//It returns the eval of the last expression
Value *evalEach(Value *args, Frame *frame){
  Value *evaledArgs = makeNull();
  while(args->type != NULL_TYPE){
    evaledArgs = cons(eval(car(args), frame), evaledArgs);
    args = cdr(args);
  }
  return reverse(evaledArgs);
}

//Given a primitive function pointer, it calls that
//function via a function pointer in prim
Value *evalPrimitive(Value *prim, Value *args){
  return prim->pf(args);
}

//Binds the inputted args to the given arguments for
//the closure
//Then it evaluates the closure from the frame
//it was initialized with
Value *evalClosure(Value *closure, Value *args){
  if(length(closure->cl.paramNames) != length(args)){
    printf("Expected %i parameters, got %i parameters. \n",
	   length(closure->cl.paramNames), length(args));
    texit(1);
  }
  //frame the inputs
  Frame *frame = talloc(sizeof(Frame));
  Value *bindings = makeNull();
  frame->parent = closure->cl.frame;
  Value *iterator = closure->cl.paramNames;
  for(int i = length(closure->cl.paramNames); i>0; i--){
    bindings = cons(car(args), bindings);
    bindings = cons(car(iterator), bindings);
    args = cdr(args);
    iterator = cdr(iterator);
  }
  frame->bindings = bindings;
  return evalBegin(closure->cl.functionCode, frame);
}

//Checks whether or not the first arg is a PRIMITIVE_TYPE
//or a lambda expression, and then calls the appropriate function
Value *apply(Value *first, Value *args){
  assert(first->type == CLOSURE_TYPE || first->type == PRIMITIVE_TYPE);
  if(first->type == PRIMITIVE_TYPE){
    return evalPrimitive(first, args);
  } else {
    return evalClosure(first, args);
  }
}

//Looks through the frame for the first instance
//of the symbol type that is the car(args)
//And then replaces it with the evaluated second arg-
//-ument
Value *evalSetBang(Value *args, Frame *frame){
  if(length(args) != 2){
    printf("Syntax error: set! takes two arguments\n");
    texit(1);
  }
  if(car(args)->type != SYMBOL_TYPE){
    printf("Syntax error: The first argument of set! must be a symbol\n");
    texit(1);
  }
  Frame *framePtr = frame;
  while(framePtr != NULL){
    Value *bindings = framePtr->bindings;
    while(bindings->type != NULL_TYPE){
      if(!strcmp(car(args)->s, car(bindings)->s)){
        cdr(bindings)->c.car = eval(car(cdr(args)), frame);
      }
      bindings = cdr(cdr(bindings));
    }
    framePtr = framePtr->parent;
  }
  Value *val = makeNull();
  val->type = VOID_TYPE;
  return val;
}

//Evaluates each scheme expression inputted in the frame
//and then returns the evaluated last argument
Value *evalBegin(Value *args, Frame *frame){
  Value *evaledArg;
  while(args->type != NULL_TYPE){
    evaledArg = eval(car(args), frame);
    args = cdr(args);
  }
  return evaledArg;
}

//Initializes a frame where else evaluates to #t
Frame *createCondFrame(Frame *frame){
  Frame *condFrame = talloc(sizeof(Frame));
  condFrame->parent = frame;

  Value *elseVal = makeNull();
  elseVal->type = SYMBOL_TYPE;
  elseVal->s = "else";

  Value *trueVal = makeNull();
  trueVal->type = BOOL_TYPE;
  trueVal->i = 1;

  condFrame->bindings = makeNull();
  condFrame->bindings = cons(trueVal, condFrame->bindings);
  condFrame->bindings = cons(elseVal, condFrame->bindings);

  return condFrame;
}

//Expects a list of the form
//(cond ((<boolean expression>) (<expression>))...)
//If a boolean expression evalutates to true, it short
// circuits and return the appropriate expression
Value *evalCond(Value *args, Frame *frame){
  Frame *condFrame = createCondFrame(frame);
  Value *temp;
  while(args->type != NULL_TYPE){
    if(length(car(args)) != 2){
      printf("Syntax Error: Cond needs 2 args per conditional\n");
      texit(1);
    }
    temp = eval(car(car(args)), condFrame);
    if(temp->type == BOOL_TYPE){
      if(temp->i){
        return eval(car(cdr(car(args))), frame);
      }
    } else {
      printf("Syntax Error: Cond needs a boolean as the first argument in each body\n");
      texit(1);
    }
    args = cdr(args);
  }
  temp = makeNull();
  temp->type = VOID_TYPE;
  return temp;
}
