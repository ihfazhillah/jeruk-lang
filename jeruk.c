/*
 * =====================================================================================
 *
 *       Filename:  jeruk.c
 *
 *    Description:  Repl for jeruk lang
 *
 *        Version:  1.0
 *        Created:  15/09/19 07:32:05
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Muhammad Ihfazhillah (), mihfazhillah@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include "mpc.h"

#ifdef _WIN32
#include <string.h>


static char buffer[2048];

/*  fake readline function */
char * readline(char * prompt){
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char * cpy = malloc(strlen(buffer)+1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy)-1] = "\0";
    return cpy;
}

void add_history (char * unused) {};

#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

long eval(mpc_ast_t* t);
long eval_op(long x, char* op, long y);
int min(int x, int y);
int max(int x, int y);

int main(int argc, char ** argv){
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Expression = mpc_new("expression");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Jeruk = mpc_new("jeruk"); // :D jeruk language


    // TODO: how is decimal number?
    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                         \
        number : /-?[0-9]+/ ;                                     \
        operator : '+' | '-' | '*' | '/' | '%' | '^'| \"min\" |   \
        \"max\";                                                  \
        expression: <number> | '(' <operator> <expression>+ ')' ; \
        jeruk: /^/ <operator> <expression>+ /$/ ;                 \
        ",
        Number, Operator, Expression, Jeruk
    );


    puts("Lispy Version 0.0.0.0.0.0.1 :D");
    puts("Press CTRL+C to Exit.\n");

    while (1){

        char * input = readline("jeruk> ");
        add_history(input);

        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Jeruk, &r)){
            mpc_ast_print(r.output);
            long result = eval(r.output);
            printf("%li\n", result);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    mpc_cleanup(4, Number, Operator, Expression, Jeruk);

    return 0;
}


long eval(mpc_ast_t* t){
    if (strstr(t->tag, "number")){
        return atoi(t->contents);
    }

    /* The operator is always second child. */
    char* op = t->children[1]->contents;


    /* handle when operator is negative, and expression is only one number */
    if (t->children_num == 4){
        if (strcmp(op, "-") == 0) return -(atoi(t->children[2]->contents));
    }

    /* we store the third child in `x`  */ 
    long x = eval(t->children[2]);

    /* Iterate the remaining children and combining. */
    int i = 3;
    while (strstr(t->children[i]->tag, "expression")){
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }

    return x;
}

long eval_op(long x, char* op, long y){
    if (strcmp(op, "+") == 0)  return x + y; 
    if (strcmp(op, "-") == 0)  return x - y;
    if (strcmp(op, "*") == 0)  return x * y;
    if (strcmp(op, "/") == 0)  return x / y;
    if (strcmp(op, "%") == 0)  return x % y;
    if (strcmp(op, "^") == 0)  return pow(x,y);
    if (strstr(op, "min"))  return min(x, y);
    if (strstr(op, "max"))  return max(x,y);
    return 0;
}

int min(int x, int y) {return x < y ? x : y;}
int max(int x, int y) {return x > y ? x : y;}
