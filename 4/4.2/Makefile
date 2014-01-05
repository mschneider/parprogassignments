all: heatmap

heatmap: heatmap.c
	gcc -O3 -fpeel-loops -funroll-loops --std=c99 heatmap.c -pthread -lm -lOpenCL -o heatmap

submission:
	tar -c heatmap.c heatmap.cl helper.h Makefile >submission.tar

