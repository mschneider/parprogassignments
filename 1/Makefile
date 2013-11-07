all: parsum dinner parsumtry

parsum: parsum.c
	gcc -O3 parsum.c -o parsum

parsumtry: parsumtry.c
	gcc -O3 parsumtry.c -pthread -o parsumtry

asm: parsum.c
	gcc -O3 parsum.c -S -dp

dinner: dinner.c
	gcc --std=c99 -lpthread -O3 dinner.c -o dinner
