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

enum { JVAL_NUM, JVAL_ERR, JVAL_SYM, JVAL_SEXPR };

typedef struct jval{
    int type;
    long num;

    char* err;
    char* sym;

    /* Trick to do variable length struct */
    int count; // keep track count of length
    struct jval** cell;
} jval;

/* jval eval(mpc_ast_t* t); */
/* jval eval_op(jval x, char* op, jval y); */
/* jval min(jval x, jval y); */
/* jval max(jval x, jval y); */


jval jval_num(long x){
    jval v;
    v.type = JVAL_NUM;
    v.num = x;
    return v;
}

jval jval_err(int x){
    jval v;
    v.type = JVAL_ERR;
    v.err = x;
    return v;
}

void jval_print(jval v){
    switch(v.type){
        case JVAL_NUM: 
            printf("%li", v.num); 
            break;
        case JVAL_ERR:
            switch(v.err){
                case JERR_DIV_ZERO:
                    printf("Error: Division by zero!");
                    break;
                case JERR_BAD_OP:
                    printf("Error: Invalid operator!");
                    break;
                case JERR_BAD_NUM:
                    printf("Error: Invalid Number!");
                    break;
            }
            break;
    }
}

void jval_println(jval v){ jval_print(v); putchar('\n'); }


int main(int argc, char ** argv){
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Expression = mpc_new("expression");
    mpc_parser_t* Sexpr = mpc_new("sexpr");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Jeruk = mpc_new("jeruk"); // :D jeruk language


    // TODO: how is decimal number?
    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                         \
        number : /-?[0-9]+/ ;                                     \
        symbol : '+' | '-' | '*' | '/' | '%' | '^'| \"min\" |     \
        \"max\";                                                  \
        expression: <number> | <symbol> | <sexpr> ;               \
        sexpr: '(' <expression> ')';                              \ 
        jeruk: /^/ <operator> <expression>+ /$/ ;                 \
        ",
        Number, Symbol, Expression, Jeruk, Sexpr
    );


    puts("Lispy Version 0.0.0.0.0.0.1 :D");
    puts("Press CTRL+C to Exit.\n");

    while (1){

        char * input = readline("jeruk> ");
        add_history(input);

        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Jeruk, &r)){
            jval result = eval(r.output);
            jval_println(result);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    mpc_cleanup(5, Number, Symbol, Sexpr, Expression, Jeruk);

    return 0;
}


/* jval eval(mpc_ast_t* t){ */
/*     if (strstr(t->tag, "number")){ */
/*         errno = 0; */
/*         long x = strtol(t->contents, NULL, 10); */
/*         return errno != ERANGE ? jval_num(x) : jval_err(JERR_BAD_NUM); */
/*     } */

/*     /1* The operator is always second child. *1/ */
/*     char* op = t->children[1]->contents; */


/*     /1* handle when operator is negative, and expression is only one number *1/ */
/*     if (t->children_num == 4){ */
/*         if (strcmp(op, "-") == 0) { */
/*             errno = 0; */
/*             long y = strtol(t->children[2]->contents, NULL, 10); */
/*             return errno != ERANGE */
/*                 ? jval_num(-y) */ 
/*                 : jval_err(JERR_BAD_NUM); */
/*         } */
/*     } */

/*     /1* we store the third child in `x`  *1/ */ 
/*     jval x = eval(t->children[2]); */

/*     /1* Iterate the remaining children and combining. *1/ */
/*     int i = 3; */
/*     while (strstr(t->children[i]->tag, "expression")){ */
/*         x = eval_op(x, op, eval(t->children[i])); */
/*         i++; */
/*     } */

/*     return x; */
/* } */

/* jval eval_op(jval x, char* op, jval y){ */
/*     if (x.type == JVAL_ERR)  return x; */ 
/*     if (y.type == JVAL_ERR)  return y; */ 

/*     if (strcmp(op, "+") == 0)  return jval_num(x.num + y.num); */ 
/*     if (strcmp(op, "-") == 0)  return jval_num(x.num - y.num); */
/*     if (strcmp(op, "*") == 0)  return jval_num(x.num * y.num); */
/*     if (strcmp(op, "%") == 0)  return jval_num(x.num % y.num); */
/*     if (strcmp(op, "^") == 0)  return jval_num(pow(x.num,y.num)); */
/*     if (strstr(op, "min"))  return min(x, y); */
/*     if (strstr(op, "max"))  return max(x, y); */

/*     if (strcmp(op, "/") == 0)  { */
/*         return y.num == 0? */
/*             jval_err(JERR_DIV_ZERO) */
/*             : jval_num(x.num / y.num); */
/*     }; */

/*     return jval_err(JERR_BAD_OP); */
/* } */

/* jval min(jval x, jval y) {return x.num < y.num ? x : y;} */
/* jval max(jval x, jval y) {return x.num > y.num ? x : y;} */
