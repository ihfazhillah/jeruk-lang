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


jval* jval_num(long x){
    jval* v = malloc(sizeof(jval));
    v->type = JVAL_NUM;
    v->num = x;
    return v;
}

jval* jval_err(char* m){
    jval* v = malloc(sizeof(jval));
    v->type = JVAL_ERR;
    // alokasi memory untuk error string
    v->err = malloc(strlen(m) + 1);
    // copy string m to v->err
    strcpy(v->err, m);
    return v;
}

jval* jval_sym(char* s){
    jval* v = malloc(sizeof(jval));
    v->type = JVAL_SYM;
    // alokasi memory untuk symbol string
    v->sym = malloc(strlen(s) + 1);
    // copy
    strcpy(v->sym, s);
    return v;
}

/* init sexpr with zero count and NULL pointer */
jval* jval_sexpr(void){
    jval* v = malloc(sizeof(jval));
    v->type = JVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

/* jval deconstructor */
void jval_del(jval* v){
    switch(v->type){
        case JVAL_NUM: break;

        /* For sym and err, just free it */
        case JVAL_ERR: free(v->err); break;
        case JVAL_SYM: free(v->sym); break;

        /* For sexpr, then delete all element inside first, then free the container pointer */
        case JVAL_SEXPR:
            for(int i=0; i < v->count; i++){
                jval_del(v->cell[i]);
            }
            free(v->cell);
            break;
    }

    /* Dont forget to free the v */
    free(v);
}

jval* jval_read_num(mpc_ast_t* t){
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ?
        jval_num(x): jval_err("invalid number");
}

jval* jval_add(jval* x, jval* y);
jval* jval_eval(jval* v);
jval* jval_take(jval* v, int i);
jval* jval_pop(jval* v, int i);
jval* builtin_op(jval* a, char* op);

jval* jval_read(mpc_ast_t* t){
    /*  If Symbol or Number return conversion to that type */
    if (strstr(t->tag, "number")) return jval_read_num(t);
    if (strstr(t->tag, "symbol")) return jval_sym(t->contents);

    /*  if root (>) or sexpr then create empty list */
    jval* x = NULL;
    if (strcmp(t->tag, ">") == 0) x = jval_sexpr();
    if (strstr(t->tag, "sexpr")) x = jval_sexpr();

    /*  Fill this list with any valid expression contained within */
    for (int i = 0; i < t->children_num; i++){
        if (strcmp(t->children[i]->contents, "(") == 0) continue;
        if (strcmp(t->children[i]->contents, ")") == 0) continue;
        if (strcmp(t->children[i]->tag, "regex") == 0) continue;
        x = jval_add(x, jval_read(t->children[i]));
    }

    return x;
}

jval* jval_add(jval* v, jval* x){
    v->count++;
    v->cell = realloc(v->cell, sizeof(jval*) * v->count);
    v->cell[v->count - 1] = x;
    return v;
}

void jval_expr_print(jval* v, char open, char close){
    putchar(open);
    for (int i = 0; i < v->count; i++){
        /*  Print value contained within */
        jval_print(v->cell[i]);

        /*  DOn't print trailing space if last element */
        if (i != (v->count-1)){
            putchar(' ');
        }
    }
    putchar(close);
}


void jval_print(jval* v){
    switch(v->type){
        case JVAL_NUM: printf("%li", v->num); break;
        case JVAL_ERR: printf("Error: %s", v->err); break;
        case JVAL_SYM: printf("%s", v->sym); break;
        case JVAL_SEXPR: jval_expr_print(v, '(', ')'); break;
    }
}

void jval_println(jval* v){ jval_print(v); putchar('\n'); }


jval* jval_eval_sexpr(jval* v){
    /*  evaluate children */
    for (int i = 0; i < v->count; i++){
        v->cell[i] = jval_eval(v->cell[i]);
    }

    /*  Error checking */
    for (int i = 0; i < v->count; i++){
        if (v->cell[i]->type == JVAL_ERR) return jval_take(v, i);
    }

    /* Empty expresion */
    if (v->count == 0) return v;

    /* Single expression */
    if (v->count == 1) return jval_take(v, 0);

    /* Ensure first element is symbol */
    jval* f = jval_pop(v, 0);
    if (f->type != JVAL_SYM){
        jval_del(f); jval_del(v);
        return jval_err("S-expression does not start with symbol!");
    }

    /* Call builtin with operator */
    jval* result = builtin_op(v, f->sym);
    jval_del(f);
    return result;
}

jval* jval_eval(jval* v){
    /* Evaluate sexpressions */
    if (v->type == JVAL_SEXPR) return jval_eval_sexpr(v);
    return v;
}

jval* jval_pop(jval* v, int i){
    jval* x = v->cell[i];

    /* shift memory after the item at "i over the top" */
    memmove(&v->cell[i], &v->cell[i+1], sizeof(jval*) * (v->count-i-1));
    v->count--;
    v->cell = realloc(v->cell, sizeof(jval*) * v->count);
    return x;
}

jval* jval_take(jval* v, int i){
    jval* x = jval_pop(v, i);
    jval_del(v);
    return x;
}

jval* builtin_op(jval* a, char* op){
    /*  Ensure all arguments are numbers */
    for (int i = 0; i < a->count; i++){
        if(a->cell[i]->type != JVAL_NUM){
            jval_del(a);
            return jval_err("Cannot operate on non-number!");
        }
    }

    jval* x = jval_pop(a, 0);

    /* If no arguments and sub then perform unary negation */
    if((strcmp(op, "-") == 0) && a->count == 0){
        x->num = -x->num;
    }

    /* While thre are still elements remaining */
    while (a->count > 0){
        jval* y = jval_pop(a, 0);

        if (strcmp(op, "+") == 0) {x->num += y->num;}
        if (strcmp(op, "-") == 0) {x->num -= y->num;}
        if (strcmp(op, "*") == 0) {x->num *= y->num;}
        if (strcmp(op, "/") == 0) {
            if (y->num == 0){
                jval_del(x); jval_del(y);
                x = jval_err("Division By Zero!"); break;
            }
            x->num /= y->num;
        }
        if (strcmp(op, "%") == 0) {x->num %= y->num;}
        if (strstr(op, "min")) {
            x->num = x->num > y->num ? y->num: x->num;
        }
        if (strstr(op, "max")) {
            x->num = x->num > y->num ? x->num: y->num;
        }

        jval_del(y);

    }
    jval_del(a); return x;
}



int main(int argc, char ** argv){
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Expression = mpc_new("expression");
    mpc_parser_t* Sexpr = mpc_new("sexpr");
    mpc_parser_t* Qexpr = mpc_new("qexpr");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Jeruk = mpc_new("jeruk"); // :D jeruk language


    // TODO: how is decimal number?
    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                         \
        number : /-?[0-9]+/ ;                                     \
        symbol : '+' | '-' | '*' | '/' | '%' | '^'| \"min\" |     \
        \"max\";                                                  \
        expression: <number> | <symbol> | <sexpr> ;               \
        sexpr: '(' <expression>* ')';                             \
        qexpr: '{' <expression>* '}';                             \
        jeruk: /^/  <expression>* /$/ ;                           \
        ",
        Number, Symbol, Expression, Jeruk, Sexpr, Qexpr
    );


    puts("Lispy Version 0.0.0.0.0.0.1 :D");
    puts("Press CTRL+C to Exit.\n");

    while (1){

        char * input = readline("jeruk> ");
        add_history(input);

        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Jeruk, &r)){
            /* jval result = eval(r.output); */
            jval* x = jval_eval(jval_read(r.output));
            jval_println(x);
            jval_del(x);

            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    mpc_cleanup(6, Number, Symbol, Sexpr, Expression, Jeruk, Qexpr);

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
