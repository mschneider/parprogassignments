#include <stdio.h>

typedef struct arg_t
{
  int threads;
  int start;
  int end;
} arg_t;

void read_args(
    char** argv, // in
    arg_t* args) // out
{
  args->threads = atoi(argv[1]);
  args->start = atoi(argv[2]);
  args->end = atoi(argv[3]);
}

int main(int argc, char** argv)
{
  arg_t args;

  if (argc != 4)
  {
    puts("wrong arguments. usage: parsum <threads> <start> <end>");
    return -1;
  }

  read_args(argv, &args);
  printf("running with %d threads from %d to %d max",
         args.threads, args.start, args.end);

  return 0;
}
