all: parsum dinner parsumtry

parsum: parsum.c
	gcc -O3 parsum.c -o parsum

parsumtry: parsumtry.c
	gcc -O3 parsumtry.c -o -pthread parsumtry

asm: parsum.c
	gcc -O3 parsum.c -S -dp

dinner: dinner.c
	gcc --std=c99 -O3 dinner.c -o dinner
