#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <errno.h>

#define MIN(a, b) ((a < b) ? a : b)

typedef struct arg_t
{
  int width_field;
  int height_field;
  int n_rounds;
  int act_round;
  char* hotspot_filename;
  char* selection_filename;
} arg_t;

typedef struct thread_arg_t {
  int id;
  void * field;
} thread_arg_t;

typedef struct hotspot
{
  int x;
  int y;
  int start;
  int end;
} hotspot_t;

typedef struct hotspot_vector
{
  hotspot_t * elems;
  size_t      count;
} hotspot_vector_t;

typedef double field_value_t;

typedef struct field
{
  volatile field_value_t* old_values;
  volatile field_value_t* new_values;
  int height;
  int width;
  int n_threads;
  int rows_per_thread;
  pthread_mutex_t* start_conditions;
  pthread_mutex_t* stop_conditions;
  pthread_t* threads;
  thread_arg_t* thread_args;
} field_t;

int to_linear_field_index(
  const field_t* field,
  const int x,
  const int y);

void read_str_arg(
  const char*  arg, // in
  char**       out) // out
{
  *out = (char*) calloc(strlen(arg) + 1, sizeof(char));
  sscanf(arg,"%s", *out);
}

/* Funktionen zum einlesen von Werten*/
void read_args(
  const int    argc, // in
  const char** argv, // in
  arg_t*       args) // out
{
  sscanf(argv[1],"%d", &args->width_field);
  sscanf(argv[2],"%d", &args->height_field);
  sscanf(argv[3],"%d", &args->n_rounds);
  args->act_round = 0;
  read_str_arg(argv[4], &args->hotspot_filename);
  if (argc >= 6)
    read_str_arg(argv[5], &args->selection_filename);
}

#define MAX_HOTSPOTS 1024*1024

int read_hotspots(
  const char* filename, // in
  hotspot_vector_t* hotspots) // out
{
  FILE* append_new_line = fopen(filename, "a");
  if (NULL == append_new_line)
    return -1;
  fprintf(append_new_line, "\n");
  fclose(append_new_line);

  FILE* file = fopen(filename, "r");
  if (NULL == file)
    return -2;

  char first_line[256];
  if (NULL == fgets(first_line, sizeof(first_line), file))
    return -3;

  if (0 != strcmp(first_line, "x,y,startround,endround\n"))
    return -4;

  hotspots->count = 0;
  hotspots->elems = calloc(MAX_HOTSPOTS, sizeof(hotspot_t));

  // lesen von File ab zweiter Zeile und für jede Zeile ein neues Struct hotspot
  while (1) {
    hotspot_t* current_hotspot = &hotspots->elems[hotspots->count];
    int numbers_read = fscanf(file, "%d,%d,%d,%d\n",
          &current_hotspot->x,
          &current_hotspot->y,
          &current_hotspot->start,
          &current_hotspot->end);

//    printf("hotspot(%d):%d,%d,%d,%d\n",
//        numbers_read,
//        current_hotspot->x,
//        current_hotspot->y,
//        current_hotspot->start,
//        current_hotspot->end);

    if (numbers_read == -1) // end of file
      return 0;
    if (numbers_read != 4) // wrong format
      return -1;
    if (hotspots->count >= MAX_HOTSPOTS) // file too long
      return -1;

    ++hotspots->count;
  }
  return 0;
}


int read_coordinates(
  const char* filename, // in
  hotspot_vector_t* hotspots) // out
{
  FILE* append_new_line = fopen(filename, "a");
  if (NULL == append_new_line)
    return -1;
  fprintf(append_new_line, "\n");
  fclose(append_new_line);

  FILE* file = fopen(filename, "r");
  if (NULL == file)
    return -2;

  char first_line[256];
  if (NULL == fgets(first_line, sizeof(first_line), file))
    return -3;

  if (0 != strcmp(first_line, "x,y\n"))
    return -4;

  hotspots->count = 0;
  hotspots->elems = calloc(MAX_HOTSPOTS, sizeof(hotspot_t));

  // lesen von File ab zweiter Zeile und für jede Zeile ein neues Struct hotspot
  while (1) {
    hotspot_t* current_hotspot = &hotspots->elems[hotspots->count];
    int numbers_read = fscanf(file, "%d,%d\n",
          &current_hotspot->x,
          &current_hotspot->y);

//    printf("hotspot(%d):%d,%d,%d,%d\n",
//        numbers_read,
//        current_hotspot->x,
//        current_hotspot->y,
//        current_hotspot->start,
//        current_hotspot->end);

    if (numbers_read == -1) // end of file
      return 0;
    if (numbers_read != 2) // wrong format
      return -1;
    if (hotspots->count >= MAX_HOTSPOTS) // file too long
      return -1;

    ++hotspots->count;
  }
  return 0;
}




/*Warum modified als erstes dachte in/out*/
void set_hotspots(
  field_t * field, // modified
  const int current_round,
  const hotspot_vector_t * hotspots) // in
{
  for (int i = 0; i < hotspots->count; ++i)
  {
    const hotspot_t * hotspot = &hotspots->elems[i];
    if (current_round >= hotspot->start
        && current_round < hotspot->end)
    {
      field->old_values[to_linear_field_index(field, hotspot->x, hotspot->y)] = 1.0;
    }
  }
}





void * thread_main(void *);

void init_field(
  const arg_t args,     // in
  field_t* field) // out
{
  field->height = args.height_field;
  field->width = args.width_field;
  size_t field_size = (field->width + 2) * (field->height + 2);
  printf("allocating field %ix%i=%lu\n", field->width, field->height, field_size);
  field->old_values = calloc(field_size, sizeof(field_value_t));
  field->old_values = &field->old_values[3 + field->width];
  field->new_values = calloc(field_size, sizeof(field_value_t));
  field->new_values = &field->new_values[3 + field->width];
  field->rows_per_thread = 32 * 1024 / (field->width + 2);
  field->n_threads = (field->height)/field->rows_per_thread + (((field->height) % field->rows_per_thread) ? 1 : 0);
  printf("using %i threads\n", field->n_threads);
  field->start_conditions = calloc(field->n_threads, sizeof(pthread_mutex_t));
  field->stop_conditions = calloc(field->n_threads, sizeof(pthread_mutex_t));
  field->threads = calloc(field->n_threads, sizeof(pthread_t));
  field->thread_args = calloc(field->n_threads, sizeof(thread_arg_t));
  for (int i = 0; i < field->n_threads; ++i)
  {
    pthread_mutex_init(&field->start_conditions[i], NULL);
    pthread_mutex_lock(&field->start_conditions[i]);
    pthread_mutex_init(&field->stop_conditions[i], NULL);
    pthread_mutex_lock(&field->stop_conditions[i]);
    field->thread_args[i].id = i;
    field->thread_args[i].field = field;
    pthread_create(&field->threads[i], NULL, thread_main, &field->thread_args[i]);
  }
}

int to_linear_field_index(
  const field_t* field,
  const int x,
  const int y)
{
  return y * (field->width+2) + x;
}

double get_old_field_value(
  const field_t* field,
  const int x,
  const int y)
{
  return field->old_values[to_linear_field_index(field, x ,y)];
}

void print_field(
  const field_t* field) // in
{
  FILE * file = fopen("output.txt", "w+");
  for (int y = 0; y < field->height; ++y)
  {
    for (int x = 0; x < field->width; ++x)
    {
      const field_value_t value = get_old_field_value(field, x, y);
      if (value > 0.9)
        fprintf(file, "X");
      else
        fprintf(file, "%d", (int) (10.0 * (value + 0.09)));

    }
    fprintf(file, "\n");
  }
  fclose(file);
}

void print_coordinate_values(
  const field_t* field,
  const hotspot_vector_t* coordinates)
{
  FILE * file = fopen("output.txt", "w+");
  for (int i = 0; i < coordinates->count; ++i)
  {
    const hotspot_t * p = &coordinates->elems[i];
    const field_value_t value = get_old_field_value(field, p->x, p->y);
    fprintf(file, "%0.17lf\n", value);
  }
  fclose(file);
}

void sequential_simulate_round(
  field_t* field) // in and out
{
  for (int y = 0; y < field->height; ++y)
  {
    for (int x = 0; x < field->width; ++x)
    {
      const double new_value = get_old_field_value(field, x-1, y-1)
                             + get_old_field_value(field, x-1, y)
                             + get_old_field_value(field, x-1, y+1)
                             + get_old_field_value(field, x  , y-1)
                             + get_old_field_value(field, x  , y)
                             + get_old_field_value(field, x  , y+1)
                             + get_old_field_value(field, x+1, y-1)
                             + get_old_field_value(field, x+1, y)
                             + get_old_field_value(field, x+1, y+1);
      field->new_values[to_linear_field_index(field, x, y)] = new_value / 9.0;
    }
  }
}

void simulate_round(
  field_t* field) // in and out
{
  for (int i = 0; i < field->n_threads; ++i)
  {
    pthread_mutex_unlock(&field->start_conditions[i]);
  }
  for (int i = 0; i < field->n_threads; ++i)
  {
    pthread_mutex_lock(&field->stop_conditions[i]);
  }
}




void simulate_part_of_round(
  field_t* field,
  int id)
{

    int start_y = id * field->rows_per_thread;
    int end_y = MIN(field->height, (id+1) * field->rows_per_thread);

    for (int y = start_y; y < end_y; ++y)
    {
      for (int x = 0; x < field->width; ++x)
      {
        const double new_value = get_old_field_value(field, x-1, y-1)
                               + get_old_field_value(field, x-1, y)
                               + get_old_field_value(field, x-1, y+1)
                               + get_old_field_value(field, x  , y-1)
                               + get_old_field_value(field, x  , y)
                               + get_old_field_value(field, x  , y+1)
                               + get_old_field_value(field, x+1, y-1)
                               + get_old_field_value(field, x+1, y)
                               + get_old_field_value(field, x+1, y+1);
        field->new_values[to_linear_field_index(field, x, y)] = new_value / 9.0;
      }
    }
}

void * thread_main(void * _args)
{
  thread_arg_t * args = (thread_arg_t *) _args;
  field_t * field = (field_t *) args->field;
  int id = args->id;
  while (1)
  {
    pthread_mutex_lock(&field->start_conditions[id]);
    simulate_part_of_round(field, id);
    pthread_mutex_unlock(&field->stop_conditions[id]);
  }
  return NULL;

}

int main(int argc, const char** argv)
{
  arg_t args;
  read_args(argc, argv, &args);

  printf("args width:%i height:%i rounds:%i hotspots:%s selection:%s\n",
      args.width_field,
      args.height_field,
      args.n_rounds,
      args.hotspot_filename,
      argc >= 6 ? args.selection_filename : "empty");

  hotspot_vector_t hotspots;
  hotspot_vector_t coordinates;

  if (read_hotspots(args.hotspot_filename, &hotspots))
  {
    printf("could not read hotspots from: %s", args.hotspot_filename);
    return -1;
  }


  field_t field;
  init_field(args, &field);

  int current_round = 0;
  set_hotspots(&field, current_round, &hotspots);
  while (current_round++ < args.n_rounds)
  {
    /*print_field(&field);*/
    simulate_round(&field);

    // double buffering
    volatile field_value_t * temp = field.old_values;
    field.old_values = field.new_values;
    field.new_values = temp;

    set_hotspots(&field, current_round, &hotspots);
  }
  if (argc >= 6) {
    read_coordinates(args.selection_filename, &coordinates);
    print_coordinate_values(&field, &coordinates);}
  else
  {
    print_field(&field);
  }
   return 0;
}
