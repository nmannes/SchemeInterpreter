#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "linkedlist.h"
#include "value.h"
#include "talloc.h"

// Create a new NULL_TYPE value node.
// Returns a pointer to a Value.
Value *makeNull(){
  Value *value = talloc(sizeof(Value));
  value->type = NULL_TYPE;
  value->i = 1;
  return value;
}

// Create a new CONS_TYPE value node.
// Returns a pointer to a Value.
Value *cons(Value *car, Value *cdr){
  Value *value = talloc(sizeof(Value));
  value->type = CONS_TYPE;
  value->c.car = car;
  value->c.cdr = cdr;
  return value;
}


// Display the contents of the linked list to the screen in some kind of
// readable format
void display(Value *list){ 
  switch (list->type){
  case INT_TYPE:
    printf("%i", list->i);
    break;
        
  case DOUBLE_TYPE:
    printf("%f", list->d);
    break;
        
  case STR_TYPE:
    printf("%s", list->s);
    break;
        
  case NULL_TYPE:
    printf("()\n");
    return;
        
  case CONS_TYPE: 
    display( car(list) );
    display( cdr(list) );
    break;

  case PTR_TYPE:
    display(list->p);
    break;

  case OPEN_TYPE:
    printf("(\n");
    break;
        
  case CLOSE_TYPE:
    printf(")\n");
    break;
	
  case BOOL_TYPE:
    if (list->i){
      printf("#t\n");
    } else {
      printf("#f\n");
    }
    break;
	  
  case SYMBOL_TYPE:
    printf("%s\n", list->s);
    break;
	  
  case CLOSURE_TYPE:
    printf("Here lies a closure\n");
    break;
  case VOID_TYPE:
    break;
  default: 
    break;
  }
}

// Return a new list that is the reverse of the one that is passed in. All
// content within the list should be duplicated; there should be no shared
// memory between the original list and the new one.
// Returns a pointer to the reversed list Value.
Value *reverseHelp(Value *list, Value *newList){
  if(isNull(list)){
    return newList;
  }
  else{
    return reverseHelp( cdr(list), cons(list->c.car, newList) ); 
  }
}

// Function that calls reverseHelp()
Value *reverse(Value *list){
  Value *end = makeNull();
  end->i = 0;
  return reverseHelp(list, end);
  //we make a new null type including all of the cons types
  //sad!
}

// Utility to make it less typing to get car value. Use assertions to make sure
// that this is a legitimate operation.
Value *car(Value *list){
  if(list->type == NULL_TYPE){
    printf("You can't take the car of an empty list\n");
    texit(1);
  }
  assert(list->type == CONS_TYPE);
  return list->c.car;
}

// Utility to make it less typing to get cdr value. Use assertions to make sure
// that this is a legitimate operation.
Value *cdr(Value *list){
  if(list->type == NULL_TYPE){
    printf("You can't take the cdr of an empty list\n");
    texit(1);
  }
  assert(list->type == CONS_TYPE);
  return list->c.cdr;
}

// Utility to check if pointing to a NULL_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isNull(Value *value){
  assert(value != NULL);
  return value->type == NULL_TYPE;
}

// Measure length of list. Use assertions to make sure that this is a legitimate
// operation.
int lengthHelp(Value *value, int counter){
  if(isNull(value)){
    return counter;
  } else {
    return lengthHelp(cdr(value), counter + 1);
  }
}

// Checks if input is NULL_TYPE or CONS_TYPE. If the former, returns length as 0.
// If the latter, calls lengthHelp.
int length(Value *value){
  if(value->type == NULL_TYPE){
    return 0;
  }
  assert(value->type == CONS_TYPE);
  return lengthHelp(value, 0);
}
