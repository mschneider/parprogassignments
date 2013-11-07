all: parsum dinner

parsum: parsum.c
	gcc -fpeel-loops -funroll-loops --std=c99 -pthread -lpthread -O3 parsum.c -o parsum

submission:
	tar -c parsum.c parsumtry.c dinner.c helper.h Makefile >submission.tar

dinner: dinner.c
	gcc --std=c99 -pthread -lpthread -O3 dinner.c -o dinner
