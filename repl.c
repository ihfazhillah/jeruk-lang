/*
 * =====================================================================================
 *
 *       Filename:  repl.c
 *
 *    Description:  Repl for lisp
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

#include <editline/readline.h>
#include <editline/history.h>



int main(int argc, char ** argv){
    puts("Lispy Version 0.0.0.0.0.0.1 :D");
    puts("Press CTRL+C to Exit.\n");

    while (1){

        char * input = readline("lispy> ");
        add_history(input);

        printf("No you're a %s\n", input);

        free(input);
    }

    return 0;
}
