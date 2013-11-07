all: parsum dinner

parsum: parsum.c
	gcc -O3 -fpeel-loops -funroll-loops --std=c99 parsum.c -pthread -lm -lpthread -o parsum

submission:
	tar -c parsum.c dinner.c helper.h Makefile >submission.tar

dinner: dinner.c
	gcc --std=c99 -pthread -lpthread -O3 dinner.c -o dinner
