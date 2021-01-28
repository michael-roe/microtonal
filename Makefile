all: microtonal

microtonal: microtonal.c
	gcc -o microtonal microtonal.c -lm
