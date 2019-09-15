all:
	gcc -std=c99 -Wall jeruk.c mpc.c -ledit -lm -o jeruk

