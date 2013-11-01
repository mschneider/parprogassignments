all: parsum dinner

parsum: parsum.c
	gcc -O3 parsum.c -o parsum

asm: parsum.c
	gcc -O3 parsum.c -S -dp

dinner: dinner.c
	gcc --std=c99 -O3 dinner.c -o dinner
