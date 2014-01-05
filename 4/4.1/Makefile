all: parsum

parsum: parsum.c
	gcc -O3 -fpeel-loops -funroll-loops --std=c99 parsum.c -pthread -lm -lOpenCL -o parsum

submission:
	tar -c parsum.c parsum.cl helper.h Makefile >submission.tar

