#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct arg_t
{
  unsigned int threads;
  uint64_t start;
  uint64_t end;
} arg_t;


typedef struct thread_arg_t
{
  uint64_t start;
  uint64_t end;
} thread_arg_t;

inline void read_args(
  char** argv, // in
  arg_t* args) // out
{
  sscanf(argv[1], "%d", &args->threads);
  sscanf(argv[2], "%lld", &args->start);
  sscanf(argv[3], "%lld", &args->end);
}

inline uint64_t sum_naive(
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

inline uint64_t sum_unrolled_16(
  uint64_t start,
  uint64_t end)
{
  uint64_t result = 0;
  if (end >= 16)
  {
    uint64_t unrolled_end = end - 16;
    while (start <= unrolled_end)
    {
      result += start;
      result += start + 1;
      result += start + 2;
      result += start + 3;
      result += start + 4;
      result += start + 5;
      result += start + 6;
      result += start + 7;
      result += start + 8;
      result += start + 9;
      result += start + 10;
      result += start + 11;
      result += start + 12;
      result += start + 13;
      result += start + 14;
      result += start + 15;
      start += 16;
    }
  }
  return result + sum_naive(start, end);
}

void * single_thread(void * args)
{
  thread_arg_t * thread_args = (thread_arg_t *) args;
  uint64_t result = sum_unrolled_16(thread_args->start, thread_args->end);
  return (void *) result;
}

#define MIN(a, b) (a < b ? a : b)

inline uint64_t sum_threaded(const arg_t * args)
{
  // initialize threads
  pthread_t * threads = calloc(args->threads, sizeof(thread_arg_t));
  thread_arg_t * thread_args = calloc(args->threads, sizeof(thread_arg_t));
  double total_work = args->end - args->start;
  double work_per_thread = total_work / args->threads;
  uint64_t start = args->start;
  for (int i = 0; i < args->threads; ++i)
  {
    thread_args[i].start = start;
    thread_args[i].end = MIN(args->end, round(start + work_per_thread));
    start = thread_args[i].end + 1;
  }
  // start threads
  for (int i = 0; i < args->threads; ++i)
  {
    pthread_create(&threads[i], NULL, single_thread, &thread_args[i]);
  }

  // wait for results
  uint64_t sum = 0;
  uint64_t thread_result = 0;
  for (int i = 0; i < args->threads; ++i)
  {
    pthread_join(threads[i], (void * *) &thread_result);
    sum += thread_result;
  }
  return sum;
}

//uint64_t sum_gauss(uint64_t limit)
//{
//  return limit * (limit+1) / 2;
//}

int main(int argc, char** argv)
{
  arg_t args;
//  timeval start, end;

//  if (argc != 4)
//  {
//    puts("wrong arguments. usage: parsum <threads> <start> <end>");
//    return -1;
//  }

  read_args(argv, &args);
//  printf("running with %u threads from %llu to %llu max\n",
//         args.threads, args.start, args.end);

//  gettimeofday(&start, NULL);
//  uint64_t sequential_result = sum_unrolled_16(args.start, args.end);
//  gettimeofday(&end, NULL);
//  printf("sequential result:\t%llu in %fs\n",
//         sequential_result, timediff(start, end));

//  gettimeofday(&start, NULL);
  uint64_t parallel_result = sum_threaded(&args);
//  gettimeofday(&end, NULL);
//  printf("parallel result:\t%llu in %fs\n",
//         parallel_result, timediff(start, end));

//  uint64_t check = sum_gauss(args.end) - sum_gauss(args.start - 1);
//  printf("check sum:\t%lld\n", check);

  FILE * output = fopen("output.txt", "w");
  fprintf(output, "%llu", parallel_result);
  fclose(output);

  return 0;
}
