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

/// @brief free tokens inside a queue, to use with map 
/// @param v_token the token that will be free
/// @param set_null_usr_param set fread token to NULL or not
void freeTokenQueueMap(void * token_ptr, void * set_null_usr_param/* *bool */){
	ptrToken p_token = (ptrToken) token_ptr;

	bool* p_set_null = (bool *) set_null_usr_param; 
	if (*p_set_null){

	delete_token(&p_token);
	}
	else{
		free(p_token);
	}
}


bool isSymbol(char c);
bool isFloatingPointNumber(char c);
Queue* stringToToken(const char* expression);
Queue* shuntingYard(Queue * infix);
float evaluateExpression(Queue* postfix);
const Token* evaluateOperator(const Token* arg1, const Token* op, const Token* arg2);

/** 
 * Utilities function to print the token queues
 */
void print_token(const void* e, void* user_param);
void print_queue(FILE* f, Queue* q);

/** 
 * Function to be written by students
 */
#define BUFFER_SIZE 256
void computeExpressions(FILE* input) {

	char *buffer =unwrapMalloc(sizeof(char) * BUFFER_SIZE);
	
	size_t buffer_size = BUFFER_SIZE;
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
			printf("\n");

			printf("Evaluate : %f\n",evaluateExpression(postfix));

			printf("\n");

			//free all tokens from infix and postfix
			bool usr_prm =true;
			bool * p_usr_prm = &usr_prm;
			
			queue_map(infix,(QueueMapOperator)freeTokenQueueMap,(void *) p_usr_prm);

			delete_queue(&postfix);
			delete_queue(&infix);
		}

		readed = getline(&buffer,&buffer_size,input);
	}
	free(buffer);
}
#undef BUFFER_SIZE

/// @brief return if c is a symbole
bool isSymbol(char c){
	switch (c){
	case '+':
	case '-':
	case '*':
	case '/':
	case '^':
	case '(':
	case ')':
		return true;
	
	default:
		return false;
	}

}

/// @brief return if c is a char representing a number
bool isFloatingPointNumber(char c){
	return (c >= '0' && c <= '9') || c == '.';
}

#define BUFFER_SIZE 256
Queue * stringToToken(const char* expression){
	Queue * queue = create_queue();
	
	const char* curpos = expression;
	while (*curpos != '\0'){
		Token * p_token = NULL;
		if (*curpos != ' ' && *curpos != '\n'){
			if(isSymbol(*curpos)){
				p_token = create_token_from_string(curpos,1);
				queue_push(queue, (void *) p_token);

				curpos++;
			}else if (isFloatingPointNumber(*curpos)){

				char buffer[BUFFER_SIZE];
				int buffer_index = 0;
				while (isFloatingPointNumber(*curpos)){
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
#undef BUFFER_SIZE

struct shuntingYardParams{
	Queue * output;
	Stack * operators;

};

void shuntingYardMap (const void *v_token, void *params){
	struct shuntingYardParams * p_params = params;
	Queue * output  = p_params->output;
	Stack * operator= p_params->operators;
	Token * token = (Token *) v_token;

	if(token_is_number(token)){
		queue_push(output,v_token);
	}
	else if(token_is_operator(token)){
		while (
				!stack_empty(operator)
			&&
				(!token_is_parenthesis(stack_top(operator)))
			&&
				(
					( token_operator_priority(stack_top(operator))>token_operator_priority(token))
				||
					(token_operator_priority(stack_top(operator))==token_operator_priority(token) && token_operator_leftAssociative(token))
				)
		){
			queue_push(output,stack_top(operator));
			stack_pop(operator);
		}
		stack_push(operator,token);
		
	}
	else if(token_is_parenthesis(token)){
		if(token_parenthesis(token) == '('){
			stack_push(operator,token);
		}
		else /*if equal to ')'*/{
			const Token * top_operator =  stack_top(operator);
			while (!token_is_parenthesis(top_operator)){
				queue_push(output,stack_top(operator));
				stack_pop(operator);

				top_operator =  stack_top(operator);
			}
			stack_pop(operator);	
		}
	}
}

Queue * shuntingYard(Queue * infix){

	struct shuntingYardParams params;
	params.output = create_queue();
	params.operators = create_stack(queue_size(infix));
	
	queue_map(infix,shuntingYardMap,&params);

	while (!stack_empty(params.operators)){
		queue_push(params.output,stack_top(params.operators));
		stack_pop(params.operators);
	}
	

	free(params.operators);
	return params.output;
}


const Token* evaluateOperator(const Token* arg1, const Token* op, const Token* arg2){
	assert(token_is_number(arg1) && token_is_number(arg2) && token_is_operator(op));

	switch (token_operator(op)){
	case ('+') : return create_token_from_value(token_value(arg1) + token_value(arg2));
	case ('-') : return create_token_from_value(token_value(arg1) - token_value(arg2));
	case ('*') : return create_token_from_value(token_value(arg1) * token_value(arg2));
	case ('/') : return create_token_from_value(token_value(arg1) / token_value(arg2)); //considering no ZERO division (if so, return inf, read the IEEE standard documentation for more informations)
	case ('^') : return create_token_from_value( powf(token_value(arg1) , token_value(arg2)));
	default:
		fprintf(stderr,"evaluateOperator failed, something was set as an operator, and was not an operator");
		return NULL;
	};
} 

float evaluateExpression(Queue* postfix){
	const Token * token = queue_empty(postfix) ? NULL : queue_top(postfix);
	Stack *stack = create_stack(queue_size(postfix));
	Queue *to_free_queue = create_queue();
	while (!queue_empty(postfix)){
		
		
		if(token_is_operator(token)){
			const Token * operand_2 = stack_top(stack);
			const Token * operand_1 = stack_top(stack_pop(stack));
			const Token * result = evaluateOperator(operand_1,token,operand_2);

			stack_push(stack_pop(stack),result);

			queue_push(to_free_queue,result);

		}
		else if(token_is_number(token)){
			stack_push(stack,token);
		}

		token = queue_empty(queue_pop(postfix)) ? NULL : queue_top(postfix);
	}
	float result = token_value(stack_top(stack));

	free(stack);
	bool param = true;
	queue_map(to_free_queue,(QueueMapOperator)freeTokenQueueMap,&param);
	delete_queue(&to_free_queue);

	return result;
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