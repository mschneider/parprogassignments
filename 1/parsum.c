#include <stdio.h>
#include <stdint.h>

#include "helper.h"

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

uint64_t sum_naive(
  uint64_t start,
  uint64_t end)
{
  uint64_t result = 0;
  while (start <= end)
  {
    result += start;
    start += 1;
  }
  return result;
}

uint64_t sum_gauss(uint64_t limit)
{
  return limit * (limit+1) / 2;
}

int main(int argc, char** argv)
{
  arg_t args;
  timeval start, end;

  if (argc != 4)
  {
    puts("wrong arguments. usage: parsum <threads> <start> <end>");
    return -1;
  }

  read_args(argv, &args);
  printf("running with %u threads from %llu to %llu max\n",
         args.threads, args.start, args.end);

  gettimeofday(&start, NULL);
  uint64_t result = sum_naive(args.start, args.end);
  gettimeofday(&end, NULL);
  printf("calculated result:\t%llu in %fs\n",
         result, timediff(start, end));

  uint64_t check = sum_gauss(args.end) - sum_gauss(args.start - 1);
  printf("calculated check:\t%lld\n", check);

  return 0;
}
