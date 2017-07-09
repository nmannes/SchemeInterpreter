#include <stdlib.h>
#include <stdio.h>
#include "talloc.h"

struct memHolder{
  void *pointer;
  struct memHolder *next;
};

typedef struct memHolder memHolder;

memHolder *tallocList = NULL;
int counter = 0;

// Replacement for malloc that stores the pointers allocated. It should store
// the pointers in some kind of list; a linked list would do fine, but insert
// here whatever code you'll need to do so; don't call functions in the
// pre-existing linkedlist.h. Otherwise you'll end up with circular
// dependencies, since you're going to modify the linked list to use talloc.
void *talloc(size_t size){
  memHolder *mem = malloc(sizeof(memHolder));
  mem->next = tallocList;
  mem->pointer = malloc(size);
  tallocList = mem;
  counter++;
  return mem->pointer;
}    

// Free all pointers allocated by talloc, as well as whatever memory you
// allocated in lists to hold those pointers.
void tfree(){
  for (int i=0; i<counter; i++){
    memHolder *mem;
    mem = tallocList;
    tallocList = tallocList->next;
    free(mem->pointer);
    free(mem);
  }
  counter = 0;
}

// Replacement for the C function "exit", that consists of two lines: it calls
// tfree before calling exit. It's useful to have later on; if an error happens,
// you can exit your program, and all memory is automatically cleaned up.
void texit(int status){
  tfree();
  exit(status);
}
