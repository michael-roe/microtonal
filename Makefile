all: microtonal microtonal2

microtonal: microtonal.c
	gcc -o microtonal microtonal.c -lm

microtonal2: microtonal2.c
	gcc -o microtonal2 microtonal2.c -lm
