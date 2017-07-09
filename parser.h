#include "value.h"

#ifndef _PARSER
#define _PARSER

// Takes a list of tokens from a Racket program, and returns a pointer to a
// parse tree representing that program.
Value *parse(Value *tokens);

// Helper function for parse().
Value *addToParseTree(Value *tree, int *depth, Value *token);


// Prints the tree to the screen in a readable fashion. It should look just like
// Racket code; use parentheses to indicate subtrees.
void printTree(Value *tree);


// Adds a space after a list item unless it's the last item
void possiblyAddSpace(Value *list);


// Helper function for printTree(), based off of display() from linkelist.c
void displaySchemeStyle(Value *list);


// Gives an error message indicating an inbalance in left and right parens and calls
// texit().
void syntaxError(int i);


#endif
