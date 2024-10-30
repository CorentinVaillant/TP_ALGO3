// Need this to use the getline C function on Linux. Works without this on MacOs. Not tested on Windows.
#define _GNU_SOURCE
#define DEBUG
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "token.h"
#include "queue.h"
#include "stack.h"

const void * stack_top_or_NULL(const ptrStack s){
	if(stack_empty(s))
		return NULL;
	else 
		return stack_top(s);
}

/// @brief make a malloc, and exit if the malloc failed, with an error message
/// @param size bytes to alloc
/// @return a successfull maloc
/// @details work kinda the same as .unwrap() in RUST where alloc memory
void * unwrapMalloc(size_t size){
	void * result = malloc(size);
	if (!result){
		fprintf(stderr,"could not initialize %ld bytes into the heap\n",size);
		perror("stopping program ");
		exit(1);
	}
	return result;
}

/// @brief exit the program with an error message if a ptr is set to NULL
/// @param ptr a pointer
/// @return the same pointer, if not set to NULL
void* unwrapPtr(void * ptr){
	if(!ptr){
		fprintf(stderr,"pointer is set to NULL");
		perror("stopping program ");
		exit(1);
	}
	return ptr;
}


/// @brief free tokens inside a queue, to use with map 
/// @param v_token the token that will be free
/// @param set_null_usr_param set fread token to NULL or not
void freeTokenMap(void * v_token, void * set_null_usr_param/* *bool */){
	ptrToken p_token = (ptrToken) v_token;

	bool* p_set_null = (bool *) set_null_usr_param; 
	if (*p_set_null){

	delete_token(&p_token);
	}
	else{
		free(p_token);
	}
	
}

/// @brief Making a copy of a queue with map
/// @param p_token the token ptr that is copy
/// @param dest the queue where the p_token is paste
/// @attention This is not doing a deep copy of a queue, so if p_token is free the source queue, p_token will be free in the dest_queue too !   
void copyTokenMap(void * p_token, void * dest){

	Queue * dest_queue = (Queue*) dest;
	queue_push(dest_queue,p_token);
}

bool isSymbole(char c);
bool isNum_f(char c);
Queue * stringToToken(const char* expression);
Queue *shuntingYard(Queue * infix);

/** 
 * Utilities function to print the token queues
 */
void print_token(const void* e, void* user_param);
void print_queue(FILE* f, Queue* q);

/** 
 * Function to be written by students
 */
void computeExpressions(FILE* input) {

	char *buffer =unwrapMalloc(sizeof(char) * 256);
	
	size_t buffer_size = 256;
	ssize_t readed;
	int i = 0;

	readed = getline(&buffer,&buffer_size,input);
	while(readed != -1){
		if(readed && *buffer != '\n'){
			i++;
			printf("Input : %s",buffer);


			printf("Infix :");
			Queue * infix = stringToToken(buffer);
			print_queue(stdout,infix);
			printf("\n");

			printf("Postfix :");
			Queue * postfix = shuntingYard(infix); //postfix is not a deep copy !
			print_queue(stdout,postfix);

			printf("\n\n");

			//free all the token
			bool usr_prm =true;
			bool * p_usr_prm = &usr_prm;

			//free all tokens from infix and postfix
			queue_map(infix,(QueueMapOperator)freeTokenMap,(void *) p_usr_prm);	

			delete_queue(&postfix);
			delete_queue(&infix);
		}

		readed = getline(&buffer,&buffer_size,input);
	}
	free(buffer);
}

/// @brief return if c is a symbole
bool isSymbole(char c){
	return( 
		(c == '+')
	||	(c == '-')
	||	(c == '*')
	||	(c == '/')
	||	(c == '^')
	||	(c == '(')
	||	(c == ')'));
}

/// @brief return if c is a char representing a number
bool isNum_f(char c){
	return (c >= '0' && c <= '9') || c == '.';
}

Queue * stringToToken(const char* expression){
	Queue * queue = create_queue();
	
	const char* curpos = expression;
	while (*curpos != '\0'){
		Token * p_token = NULL;
		if (*curpos != ' ' && *curpos != '\n'){
			if(isSymbole(*curpos)){
				p_token = create_token_from_string(curpos,1);
				queue_push(queue, (void *) p_token);

				curpos++;
			}else if (isNum_f(*curpos)){

				char buffer[256];
				int buffer_index = 0;
				while (isNum_f(*curpos)){
					buffer[buffer_index++] = *curpos;
					curpos++;
				}
				buffer[buffer_index] = '\0';
				float val = atof(buffer);
				p_token = create_token_from_value(val);
				queue_push(queue, (void *) p_token);

			}
		}
		else if (*curpos != '\0'){
			curpos++;
		}
	}


	return queue;
}

Queue * shuntingYard(Queue * infix){
	//TODO tout réecrire !!! :')

	Queue * output = create_queue();
	Stack * operators = create_stack(queue_size(infix));

	Queue * copy_infix = create_queue();
	queue_map(infix,(QueueMapOperator) copyTokenMap,copy_infix);
	

	while (!queue_empty(copy_infix)){
		Token *token = (Token *)queue_top(copy_infix);


		#ifdef DEBUG
		unwrapPtr(token);
		#endif

		if(token_is_number(token)){
			queue_push(output,token);
		}

		else if(token_is_operator(token)){

			if (!stack_empty(operators)){

				Token *top_operator = (Token *)stack_top(operators);
				while (
					!stack_empty(operators) &&
					(
						(
							//the operator at the top of the stack is not a left bracket 
							!(token_is_parenthesis(top_operator) && token_parenthesis(top_operator) ==  ')')
						)
						&& //and
						(
							
							(
								//there is an operator at the top of the operator stack with greater precedence
								token_operator_priority(top_operator) > token_operator_priority(token) 
							)
							||//or
							(
								(
									// the operator at the top of the operator stack has equal precedence 
									token_operator_priority(top_operator) == token_operator_priority(token)
								
								&&//and
								
									// the operator is left associative
									token_operator_leftAssociative(token)
								)
							) 
							
						) 
					))
				{
					queue_push(output,top_operator);
					stack_pop(operators);
					if (!stack_empty(operators))
						top_operator = (Token *)stack_top(operators); 
					else 
						top_operator = NULL;

				}
				
			}
			stack_push(operators,token);
		}

		else if(token_is_parenthesis(token)){

			if(token_parenthesis(token) == '(')
				stack_push(operators,token);
			
			else if(token_parenthesis(token) == ')'){
				Token *top_operator;
				if((top_operator = (Token *)stack_top_or_NULL(operators)))
					while (!stack_empty(operators) && !(token_is_parenthesis(token) && token_parenthesis(token) == '(')){
						
						queue_push(output,top_operator);
						if(token_is_parenthesis(top_operator)){
							printf("!!!!!!!!!!!!! : ");
							print_token(top_operator,stderr);
						}
						stack_pop(operators);

						if (!stack_empty(operators))
							top_operator = (Token *)stack_top(operators);
					}
				if (!stack_empty(operators))
					stack_pop(operators);
			}
			
		}
		queue_pop(copy_infix);

		
	}
	if (!stack_empty(operators))
		for(Token *top_operator = (Token *)stack_top(operators) ; !stack_empty(stack_pop(operators)) ; top_operator = stack_empty(operators)? NULL:  (Token *)stack_top(operators)){
			queue_push(output,top_operator);
		};

		for(int i = 0; i<10; i++);

	free(operators);
	delete_queue(&copy_infix);

	return output;
	
}

/** Main function for testing.
 * The main function expects one parameter that is the file where expressions to translate are
 * to be read.
 *
 * This file must contain a valid expression on each line
 *
 */
int main(int argc, char** argv){

	if (argc<2) {
		fprintf(stderr,"usage : %s filename\n", argv[0]);
		return 1;
	}
	
	FILE* input = fopen(argv[1], "r");

	if ( !input ) {
		perror(argv[1]);
		return 1;
	}

	computeExpressions(input);

	fclose(input);
	return 0;
}
 
void print_token(const void* e, void* user_param) {
	FILE* f = (FILE*)user_param;
	Token* t = (Token*)e;
	token_dump(f, t);
}

void print_queue(FILE* f, Queue* q) {
	fprintf(f, "(%d) --  ", queue_size(q));
	queue_map(q, print_token, f);
}