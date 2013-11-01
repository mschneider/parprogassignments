#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "helper.h"

typedef struct arg_t
{
  unsigned int philosophers;
  unsigned int run_time;
} arg_t;


void read_args(
  char** argv, // in
  arg_t* args) // out
{
  sscanf(argv[1], "%d", &args->philosophers);
  sscanf(argv[2], "%d", &args->run_time);
}

int main(int argc, char** argv)
{
  arg_t args;
  timeval start, current;

  if (argc != 3)
  {
    puts("wrong arguments. usage: dinner <threads> <run_time>");
    return -1;
  }

  read_args(argv, &args);
  printf("running with %u philosopher for %u seconds\n",
         args.philosophers, args.run_time);

  volatile unsigned int * counters
    = calloc(args.philosophers, sizeof(int));

  gettimeofday(&start, NULL);
  gettimeofday(&current, NULL);

  while (timediff(start, current) <= args.run_time)
  {
    // do something with counters;
    gettimeofday(&current, NULL);
  }

  FILE * output = fopen("output.txt", "w");
  for (int i = 0; i < args.philosophers; ++i)
  {
    if (i > 0)
      fprintf(output, ";");
    fprintf(output, "%u", counters[i]);
  }

  return 0;
}
