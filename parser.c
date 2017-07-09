#include <stdio.h>
#include <assert.h>
#include "value.h"
#include "talloc.h"
#include "parser.h"
#include "linkedlist.h"

// Takes a list of tokens from a Racket program, and returns a pointer to a
// parse tree representing that program.
Value *parse(Value *tokens) {
  Value *tree = makeNull();
  int depth = 0;

  Value *current = tokens;
  assert(current != NULL && "Error (parse): null pointer");
  while (current->type != NULL_TYPE) {
    Value *token = car(current);
    tree = addToParseTree(tree,&depth,token);
    current = cdr(current);
  }
  if(depth > 0){
    syntaxError(2);
  } else if(depth < 0){
    syntaxError(1);
  }
  return reverse(tree);
}

// Helper function for parse().
Value *addToParseTree(Value *tree, int *depth, Value *token){
  if(token->type == CLOSE_TYPE){
    *depth = *depth - 1;
    Value *newTree = makeNull();
    while(car(tree)->type != OPEN_TYPE){
      newTree = cons(car(tree), newTree);
      tree = cdr(tree);
      if(tree->type == NULL_TYPE){
	syntaxError(1);
      }
    }
    tree = cdr(tree);
    tree = cons(newTree, tree);
  } else if(token->type == OPEN_TYPE){
    *depth = *depth + 1;
    tree = cons(token, tree);
  } else {
    tree = cons(token, tree);
  }
  return tree;
}

// Helper function for printTree().
void printTreehelp(Value *tree){
  if(tree->type == CONS_TYPE){
  while(tree->type != NULL_TYPE){
    //assert(tree->type == CONS_TYPE);
    if(car(tree)->type == CONS_TYPE){
      printf("(");
      printTreehelp(car(tree));
      printf(")");
    } else {
      displaySchemeStyle(car(tree));
    }
    possiblyAddSpace(tree);
    tree = cdr(tree);
  }
} else {
	display(tree);
}

}

// Prints the tree to the screen in a readable fashion. It should look just like
// Racket code; use parentheses to indicate subtrees.
void printTree(Value *tree){
  if(tree->type != CONS_TYPE){
    displaySchemeStyle(tree);
  }else{
    printf("(");
    printTreehelp(tree);
    printf(")");
  }
}



// Adds a space after a list item unless it's the last item
void possiblyAddSpace(Value *list){
  if(cdr(list)->type != NULL_TYPE){
    printf(" ");
  }
}

// Adapted from display() in linkedlist.c
void displaySchemeStyle(Value *list){
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
  case CLOSURE_TYPE:
    printf("(closure)");
    break;

  case PRIMITIVE_TYPE:
    printf("(primitive function)\n");
    break;
  case NULL_TYPE:
    printf("()");
    break;
  case PTR_TYPE:
    printf("PTR_TYPE detected in displaySchemeStyle.\nThis shouldn't happen.\n");
    break;

  case OPEN_TYPE:
    printf("OPEN_TYPE detected in displaySchemeStyle.\nThis shouldn't happen.\n");
    break;

  case CLOSE_TYPE:
    printf("CLOSE_TYPE detected in displaySchemeStyle.\nThis shouldn't happen.\n");
    break;

  case BOOL_TYPE:
    if (list->i){
      printf("#t");
    } else {
      printf("#f");
    }
    break;

  case SYMBOL_TYPE:
    printf("%s", list->s);
    break;
  default:
    return;
  }
}

// Gives an error message indicating an inbalance in left and right parens and calls
// texit().
void syntaxError(int i){
  switch (i){
  case 1:
    printf("Syntax Error: More ')' than '('\n");
    break;

  case 2:
    printf("Syntax Error: More '(' than ')'\n");
    break;
  }
  texit(1);
}
