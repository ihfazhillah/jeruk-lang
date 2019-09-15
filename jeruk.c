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



int main(int argc, char ** argv){
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Expression = mpc_new("expression");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Jeruk = mpc_new("jeruk"); // :D jeruk language


    // TODO: how is decimal number?
    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                         \
        number : /-?[0-9]+/ ;                                     \
        operator : '+' | '-' | '*' | '/';                         \
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