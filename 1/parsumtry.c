#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include "helper.h"

uint64_t *values; //Array mit allen Werten von Start bis Ende
uint64_t *globalSum; //Array mit Zwischenergebnissen


typedef struct arg_t
{
  unsigned int threads;
  uint64_t start;
  uint64_t end;
} arg_t;

void read_args(
  char** argv, // in
  arg_t* args) // out
{
  sscanf(argv[1], "%d", &args->threads);
  sscanf(argv[2], "%lld", &args->start);
  sscanf(argv[3], "%lld", &args->end);
}



void InitGlobals(int start,int end,int threads)
{
  int i,j;
  i = end-start;
  values=malloc(i*sizeof(uint64_t));
  globalSum=malloc(threads*sizeof(uint64_t));

  for (j=0;j<i;j++)
  {

    values[j]=start+j;
  }



}

void *ParSum(void *t_id)
{
  int tid = *((int *) t_id);
  int sum;
  //wie teile ich Array gleichmäßig in X Teile auf?
  // unrolled_16 implementieren
  //sinnvoll struct zu nehmen um mehrere Parameter zu übergeben

}


int main(int argc, char** argv)
{
  arg_t args;
  int i,j;
  uint64_t sum;


  if (argc != 4)
  {
    puts("wrong arguments. usage: parsum <threads> <start> <end>");
    return -1;
  }

  read_args(argv, &args);
  printf("running with %u threads from %llu to %llu max\n",
         args.threads, args.start, args.end);

  pthread_t threads[args.threads];
  int thread_nr[args.threads];

  InitArray(args.start,args.end,args.threads);

  for (i=0; i<args.threads; ++i) {
      thread_nr[i] = i;
      pthread_create(&threads[i], NULL, ParSum, (void *) &thread_nr[i]);
   }
 
  for (i=0; i<args.threads; ++i) {
      pthread_join(threads[i], NULL);
      sum = sum + globalSum[i]
   }

}
