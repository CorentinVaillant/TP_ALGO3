// Need this to use the getline C function on Linux. Works without this on MacOs. Not tested on Windows.
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "token.h"
#include "queue.h"
#include "stack.h"


void * unwrapMalloc(size_t size){
	void * result = malloc(size);
	if (!result){
		fprintf(stderr,"could not initialize %ld bytes into the heap\n",size);
		perror("stopping program ");
		exit(1);
	}
	return result;
}

void* unwrapPtr(void * ptr){
	if(!ptr){
		fprintf(stderr,"pointer is set to NULL");
		perror("stopping program ");
		exit(1);
	}
	return ptr;
}

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

//this is not doing a full copy, this is actually a copy of the pointers to v_token
void copyTokenMap(void * v_token, void * dest /* *queue*/){
	Queue * dest_queue = (Queue*) dest;
	Token * token = (Token*) v_token;

	queue_push(dest_queue,token);
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

	Queue * output = create_queue();
	Stack * operators = create_stack(queue_size(infix));

	Queue * copy_infix = create_queue();
	queue_map(infix,(QueueMapOperator) copyTokenMap,copy_infix);
	

	while (!queue_empty(copy_infix)){
		Token *token = (Token *)queue_top(copy_infix);
		printf("[Debug] dealing with token :");
		print_token(token,stdout);
		printf("\n");


		unwrapPtr(token);

		if(token_is_number(token)){
			queue_push(output,token);
		}

		else if(token_is_operator(token)){

			if (!stack_empty(operators)){

				Token *top_operator = (Token *)stack_top(operators);
				printf("wsh ---------------------------- --> ");
				print_token(top_operator,stdout);
				printf("\n");
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
					top_operator = (Token *)stack_top(operators);
					printf("wsh ---------------------------- --> ");
					print_token(top_operator,stdout); //error here, top_operator is invalid
					printf("\n");
				}
				
			}
			stack_push(operators,token);
		}

		else if(token_is_parenthesis(token)){

			if(token_parenthesis(token) == '(')
				stack_push(operators,token);
			
			else if(token_is_parenthesis(token) && token_parenthesis(token) == ')'){
				for( Token *top_operator = (Token *)stack_top(operators) ;
				(
					!stack_empty(operators) &&
					token_is_parenthesis(token) && token_parenthesis(token) != '('
				);
					top_operator = (Token *)stack_top(operators)

				){
					queue_push(output,top_operator);
					stack_pop(operators);
				}
				stack_pop(operators);

			}
			
		}
		queue_pop(copy_infix);

		
	}

	for(Token *top_operator = (Token *)stack_top(operators) ; !stack_empty(operators) ; top_operator = (Token *)stack_top(operators)){
		queue_push(output,top_operator);
		stack_pop(operators);
	}
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