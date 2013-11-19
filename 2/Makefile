all: heatmap

heatmap: heatmap.c
	gcc -O0 -g -fpeel-loops -funroll-loops --std=c99 heatmap.c -pthread -lm -lpthread -o heatmap

submission:
	tar -c heatmap.c Makefile >submission.tar

