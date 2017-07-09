#include "value.h"
#include "linkedlist.h"
#include "talloc.h"
#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_SYMBOL_SIZE 1000

// Returns 1 if the char inputted is a digit
int isDigit(char input){
  char *checker = "0123456789";
  for(int i = 0; i < 10; i++){
    if(input == checker[i]){
      return 1;
    }
  }
  return 0;
}

// Returns 1 if the string consists only of digits(is it an integer)
int isDigitsOnly(char *input){
  return isDigit(*input) && ( !*(input + 1) || isDigitsOnly(input + 1) );
}

// Returns 1 if the string consists of all digits and only
// one decimal point
int isDecimal(char *input){
  if(*(input) == '.'){
    return isDigitsOnly(input + 1);
  }
  else{
    return isDigit(*input) && isDecimal(input + 1);
  }
}

// Returns 1 if the character array only consists of digits
int isInteger(char *input){
  return isDigitsOnly(input);
}

// Returns 1 if the input is an integer or a decimal
int isReal(char *input){
  return (isInteger(input) || isDecimal(input));
}

// Returns 1 if the input consists only of digits
int isDigitPlus(char *input){
  return (isDigit(*input) && isDigitPlus(input + 1))
    || (isDigit(*input) && !*(input + 1));
}

// Returns 1 if the input is a + or -
int isSign(char input){
  return input == '+' || input == '-';
}

//Returns 1 if the input is a valid number
int isNumber(char *input){
  return ( isSign(*(input)) && isReal(input + 1) ) || isReal(input);
}


//Checks to see the next char delineates a new lexeme
int terminate(char input){
  return input == ')' || input == '(' || input == ' ' || input == '\n' || input == '\t';
}


//Returns 2 if true, 1 if false, 0 if not a boolean type
int isBoolean(char *input){
  if(*(input) == '#' && *(input + 1) == 'f' && *(input + 2) == '\0'){
    return 0;
  }
  if(*(input) == '#' && *(input + 1) == 't' && *(input + 2) == '\0'){
    return 1;
  }
  return -1;
}

// Returns 1 if the input is a valid letter
int isLetter(char input){
  char *checker = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  for(int i = 0; i < 52; i++){
    if(input == checker[i]){
      return 1;
    }
  }
  return 0;
}

// Returns 1 if the input is a valid initial
int isInitialChar(char input){
  char *checker = "!$%&*/:<=>?~_^";
  for(int i = 0; i < 14; i++){
    if(input == checker[i]){
      return 1;
    }
  }
  return 0;
}

// Returns 1 if the char is a valid initial
int isInitial(char input){
  return isLetter(input) || isInitialChar(input);
}

int isSubsequent(char input){
  return isInitial(input) || isDigit(input)
    || isSign(input) || input == '.';

}


// Returns 1 if input consists of only valid subsequents
// as defined by the grammar on the assignment page
int isSubsequentContinuous(char *input){
  return isSubsequent(*input)
    && ( !*(input + 1) || isSubsequentContinuous(input + 1) );
}



// Returns 1 if the input is a valid identifier
int isIdentifier(char *input){
  return isSign(*input)||
    isInitial(*input) || isSubsequentContinuous(input + 1);
}

//Returns a value of type Open Parenthesis
Value *openParen(){
  Value *open = talloc(sizeof(Value));
  open->type = OPEN_TYPE;
  return open;
}

//Returns a value of type Close Parenthesis
Value *closeParen(){
  Value *close = talloc(sizeof(Value));
  close->type = CLOSE_TYPE;
  return close;
}

//Sets all of the chars in the string to null
void clear(char *input){
  for(int i = 0; i < strlen(input); i++){
    input[i] = '\0';
  }
}

//Input a char string and it returns an appropriate type of value
Value *understand(char *input, int size){
  Value *value = talloc(sizeof(Value));
  input[size] = '\0';
  //Checks for if there is a sign
  if(isSign(*input) && !isNumber(input) ){
    value->s = talloc(sizeof(char) * 2);
    value->type = SYMBOL_TYPE;
    strcpy(value->s, input);
  }
  else if(isNumber(input)){
    if(isSign(*input)){
      if(isDecimal(input+1) ){
	value->type = DOUBLE_TYPE;
	value->d = atof(input);
      } else {
	value->type = INT_TYPE;
	value->i = atoi(input);
      }
    }
    else{
      if(isDecimal(input) ){

	value->type = DOUBLE_TYPE;
	value->d = atof(input);
      } else {
	value->type = INT_TYPE;
	value->i = atoi(input);
      }
    }
  }
  else if(isBoolean(input) > -1){
    value->type = BOOL_TYPE;
    value->i = isBoolean(input);
  }
  else if(isIdentifier(input)){
    value->type = SYMBOL_TYPE;
    value->s = talloc(sizeof(char) * (size+1));
    strcpy(value->s, input);
  }
  else{

    printf("I don't recognize this token, dude.\nExciting program.\n");
    texit(1);
  }
  return value;
}

// Read all of the input from stdin, and return a linked list consisting of the
// tokens.
Value *tokenize(){
  char charRead;
  Value *list = makeNull();
  char *Lexeme = talloc(sizeof(char) * MAX_SYMBOL_SIZE);
  int lexSize = 0;
  charRead = fgetc(stdin);
  while (charRead != EOF) {
    //printf("%c\n", charRead);
    // This block handles a comment
    if (charRead == ';') {
      charRead = fgetc(stdin);
      while (charRead != '\n') {
	charRead = fgetc(stdin);
      }
      if(lexSize > 0){
	list = cons(understand(Lexeme, lexSize), list);
	lexSize = 0;
      }
    }
    //This block handles strings

    if (charRead == '"') {
      charRead = fgetc(stdin);
      while (charRead != '"') {
	Lexeme[lexSize] = charRead;
	lexSize++;
	charRead = fgetc(stdin);
      }
      charRead = fgetc(stdin);
      if (lexSize == 0) {
	Value *empty = makeNull();
	empty->type = STR_TYPE;
	empty->s = talloc(sizeof(char) * MAX_SYMBOL_SIZE);
	empty->s[0] = '\0';
	list = cons(empty, list);
	lexSize = 0;
      } else {
	Value *str = makeNull();
	str->type = STR_TYPE;
	str->s = talloc(sizeof(char) * MAX_SYMBOL_SIZE);
	Lexeme[lexSize] = '\0';
	strcpy(str->s, Lexeme);
	lexSize = 0;
	list = cons(str, list);
      }
    }
    else if (terminate(charRead) && lexSize > 0) {
      list = cons(understand(Lexeme, lexSize), list);
      clear(Lexeme);
      lexSize = 0;
    }

    if(charRead == '('){
      list = cons(openParen(), list);
    }
    else if(charRead == ')'){
      list = cons(closeParen(), list);
    }
    else {
      if(charRead != ' ' && charRead != '\n'){
	Lexeme[lexSize] = charRead;
	lexSize++;
      }
    }
    charRead = fgetc(stdin);
  }

  if(lexSize > 0){
    list = cons(understand(Lexeme, lexSize), list);
  }

  return reverse(list);
}

// Displays the contents of the linked list as tokens, with type information
void displayTokens(Value *list){
  switch (list->type){
  case INT_TYPE:
    printf("%i: Int Type\n", list->i);
    break;

  case DOUBLE_TYPE:
    printf("%f: Double Type\n", list->d);
    break;

  case STR_TYPE:
    printf("\"%s\": String Type\n", list->s);
    break;

  case OPEN_TYPE:
    printf("(: Open Type\n");
    break;

  case CLOSE_TYPE:
    printf("): Close Type\n");
    break;

  case BOOL_TYPE:
    if (list->i){
      printf("#t: Bool Type\n");
    } else {
      printf("#f: Bool Type\n");
    }
    break;

  case SYMBOL_TYPE:
    printf("%s:Symbol Type\n", list->s);
    break;


  case NULL_TYPE:
    //printf("(): Null Type\n");
    return;

  case CONS_TYPE:
    displayTokens( car(list) );
    displayTokens( cdr(list) );
    break;

  case PTR_TYPE:
    displayTokens(list->p);
    break;

  case CLOSURE_TYPE:
    break;

  case VOID_TYPE:
    break;

  default:
    printf("What type could this possibly be?\n");
    texit(1);
  }
}
