#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "helper.h"

typedef struct arg_t
{
  int philosophers;
  int run_time;
} arg_t;

typedef struct philosopher_arg_t
{
  int run_time;
  int left_fork;
  int right_fork;
  pthread_mutex_t * forks;
  timeval start;
} philosopher_arg_t;

void read_args(
  char** argv, // in
  arg_t* args) // out
{
  sscanf(argv[1], "%d", &args->philosophers);
  sscanf(argv[2], "%d", &args->run_time);
}

void * philosopher(void * args)
{
  philosopher_arg_t * philosopher_args = (philosopher_arg_t *) args;

  int64_t counter = 0;
  timeval current;
  gettimeofday(&current, NULL);

  printf("philosopher started eating left_fork:%i right_fork:%i\n",
      philosopher_args->left_fork, philosopher_args->right_fork);

  while (timediff(philosopher_args->start, current) <= philosopher_args->run_time)
  {
    pthread_mutex_t * first_fork = &philosopher_args->forks[philosopher_args->left_fork];
    pthread_mutex_t * second_fork = &philosopher_args->forks[philosopher_args->right_fork];
    // randomly exchange first & second fork assignment
    long rand = random();
    if (rand % 2)
    {
      pthread_mutex_t * memory = first_fork;
      first_fork = second_fork;
      second_fork = memory;
    }

    // pick the forks up
    while( pthread_mutex_lock(first_fork) );
    if (0 == pthread_mutex_trylock(second_fork))
    { // we did pick up the second fork
      ++counter;
      pthread_mutex_unlock(second_fork);
    }
    pthread_mutex_unlock(first_fork);

    // are we done yet?
    gettimeofday(&current, NULL);
  }

  printf("philosopher %i stopped eating\n", philosopher_args->left_fork);

  return (void *) counter;
}

int main(int argc, char** argv)
{
  arg_t args;

  if (argc != 3)
  {
    puts("wrong arguments. usage: dinner <threads> <run_time>");
    return -1;
  }

  read_args(argv, &args);
  printf("running with %i philosopher for %i seconds\n",
         args.philosophers, args.run_time);

  int64_t * counters = calloc(args.philosophers, sizeof(int64_t));
  pthread_mutex_t * forks = calloc(args.philosophers, sizeof(pthread_mutex_t));
  pthread_t * threads = calloc(args.philosophers, sizeof(pthread_t));

  for (int i = 0; i < args.philosophers; ++i)
  {
    if (pthread_mutex_init(&forks[i], NULL))
    {
      puts("could not initialize mutex");
      return -1;
    }
  }

  timeval start;
  gettimeofday(&start, NULL);

  // start philosophers
  for (int i = 0; i < args.philosophers; ++i)
  {
    philosopher_arg_t * philosopher_args = (philosopher_arg_t *) malloc(sizeof(philosopher_arg_t));
    philosopher_args->run_time = args.run_time;
    philosopher_args->left_fork = i;
    philosopher_args->right_fork = (i + 1) % args.philosophers; // last philosopher has fork 0 at his right
    philosopher_args->forks = forks;
    philosopher_args->start = start;
    pthread_t * thread = &threads[i];
    pthread_create(thread, NULL, philosopher, philosopher_args);
  }

  // stop philosophers
  for (int i = 0; i < args.philosophers; ++i)
  {
    int64_t * result = &counters[i];
    pthread_join(threads[i], (void * *) result);
  }

  FILE * output = fopen("output.txt", "w");
  for (int i = 0; i < args.philosophers; ++i)
  {
    if (i > 0)
      fprintf(output, ";");
    fprintf(output, "%lli", counters[i]);
  }

  return 0;
}
