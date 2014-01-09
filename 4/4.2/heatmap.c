#include "helper.h"
#include <string.h>


typedef struct arg_t
{
  int width_field;
  int height_field;
  int n_rounds;
  int act_round;
  char* hotspot_filename;
  char* selection_filename;
} arg_t;

typedef struct hotspot
{
  int x;
  int y;
  int offset;
  int start;
  int end;
} hotspot_t;

typedef struct hotspot_vector
{
  hotspot_t * elems;
  size_t      count;
  cl_mem      buffer;
} hotspot_vector_t;

void read_str_arg(
  const char*  arg, // in
  char**       out) // out
{
  *out = (char*) calloc(strlen(arg) + 1, sizeof(char));
  sscanf(arg,"%s", *out);
}

typedef float field_value_t;

typedef struct field
{
  int height;
  int width;
  size_t size;
  volatile field_value_t* values;
  cl_mem buffers[2];
} field_t;

int to_linear_field_index(
  const field_t* field,
  const int x,
  const int y)
{
  const int line_size = field->width + 2;
  const int offset_0_0 = line_size + 1;
  return offset_0_0 + y * line_size + x;
}

field_value_t get_field_value(
  const field_t* field,
  const int x,
  const int y)
{
  return field->values[to_linear_field_index(field, x ,y)];
}

void print_field(
  const field_t* field) // in
{
  FILE * file = fopen("output.txt", "w+");
  for (int y = 0; y < field->height; ++y)
  {
    for (int x = 0; x < field->width; ++x)
    {
      const field_value_t value = get_field_value(field, x, y);
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
    const field_value_t value = get_field_value(field, p->x, p->y);
    fprintf(file, "%0.17lf\n", value);
  }
  fclose(file);
}



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
  const field_t* field, // in
  cl_t* cl,
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

  if (0 != strcmp(first_line, "x,y,startround,endround\r\n"))
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

    if (numbers_read == -1) // end of file
      break;
    if (numbers_read != 4) // wrong format
      return -1;

    current_hotspot->offset = to_linear_field_index(field, current_hotspot->x, current_hotspot->y);
    printf("hotspot(%d):%d,%d,%d,%d,%d\n",
        numbers_read,
        current_hotspot->x,
        current_hotspot->y,
        current_hotspot->start,
        current_hotspot->end,
        current_hotspot->offset);

    if (hotspots->count >= MAX_HOTSPOTS) // file too long
      return -2;

    ++hotspots->count;
  }

  cl_int ret;
  const size_t buffer_size = hotspots->count * sizeof(hotspot_t);
  hotspots->buffer = clCreateBuffer(cl->context, CL_MEM_READ_ONLY, buffer_size, NULL, &ret);
  if (ret != CL_SUCCESS)
    return -3;
  ret = clEnqueueWriteBuffer(cl->command_queue, hotspots->buffer, CL_TRUE, 0,
        buffer_size, (void *) hotspots->elems, 0, NULL, NULL);
  if (ret != CL_SUCCESS)
    return -4;

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

  if (0 != strcmp(first_line, "x,y\r\n"))
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
    {
       puts("EOF");
      return 0;
    }
    if (numbers_read != 2) // wrong format
    {
      printf("wrong: %i\n", numbers_read);
      return -1;
    }
    if (hotspots->count >= MAX_HOTSPOTS) // file too long
    {
      return -1;
    }

    ++hotspots->count;
  }
  return 0;
}

int init_field(
  const arg_t args,
  cl_t * cl,
  field_t* field)
{
  field->height = args.height_field;
  field->width = args.width_field;
  field->size = (field->width + 2) * (field->height + 2) * sizeof(field_value_t);
  printf("allocating field %ix%ix%lu=%lu\n", field->width, field->height, sizeof(field_value_t), field->size);
  field->values = calloc(field->size, 1);

  cl_int ret;
  int num_buffers = 2;
  while (num_buffers--)
  {
      cl_mem * buffer = &field->buffers[num_buffers];
      *buffer = clCreateBuffer(cl->context, CL_MEM_READ_WRITE, field->size, NULL, &ret);
      if (ret != CL_SUCCESS)
          return -1;
      ret = clEnqueueWriteBuffer(cl->command_queue, *buffer, CL_TRUE, 0,
          field->size, (void *) field->values, 0, NULL, NULL);
      if (ret != CL_SUCCESS)
          return -2;
  }

  return 0;
}

void set_hotspots(
    cl_t * cl,
    hotspot_vector_t * hotspots,
    cl_mem * buffer,
    cl_int current_round)
{
  clSetKernelArg(cl->hotspot_kernel, 0, sizeof(cl_int), &current_round);
  clSetKernelArg(cl->hotspot_kernel, 1, sizeof(cl_mem), &hotspots->buffer);
  clSetKernelArg(cl->hotspot_kernel, 2, sizeof(cl_mem), buffer);
  size_t global_item_size = hotspots->count;
  size_t local_item_size = 1; // mac os x cpus are incapable of using more then 1 local item

  printf("set_hotspots:%p\n", buffer);
  clEnqueueNDRangeKernel(cl->command_queue, cl->hotspot_kernel, 1, NULL,
        &global_item_size, &local_item_size, 0, NULL, NULL);
  clFlush(cl->command_queue);
}

void simulate_round(
    cl_t * cl,
    field_t * field,
    cl_mem * source_buffer,
    cl_mem * target_buffer)
{
  clSetKernelArg(cl->simulation_kernel, 0, sizeof(cl_mem), source_buffer);
  clSetKernelArg(cl->simulation_kernel, 1, sizeof(cl_mem), target_buffer);
  size_t global_item_sizes[2];
  global_item_sizes[0] = field->width;
  global_item_sizes[1] = field->height;
  size_t global_item_size = field->size / sizeof(field_value_t);
  size_t local_item_sizes[2];
  local_item_sizes[0] = 1;
  local_item_sizes[1] = 1; // mac os x cpus are incapable of using more then 1 local item
  printf("simulate: %lux%lu %p->%p\n", global_item_sizes[0], global_item_sizes[1], source_buffer, target_buffer);
  clEnqueueNDRangeKernel(cl->command_queue, cl->simulation_kernel, 2, NULL,
        global_item_sizes, local_item_sizes, 0, NULL, NULL);

}


int main(int argc, const char** argv)
{
  int ret;
  arg_t args;
  read_args(argc, argv, &args);

  printf("args width:%i height:%i rounds:%i hotspots:%s selection:%s\n",
      args.width_field,
      args.height_field,
      args.n_rounds,
      args.hotspot_filename,
      argc >= 6 ? args.selection_filename : "empty");

  char * kernel_source;
  size_t kernel_source_size;

  if (ret = read_file("heatmap.cl", &kernel_source, &kernel_source_size))
  {
      printf("could not read kernel from: %s (%d)\n", "heatmap.cl", ret);
      return -2;
  }

  printf("Read kernel file (%i bytes).\n", (int)kernel_source_size);

  cl_t cl;
  if (ret = init_cl(&cl))
  {
      printf("could not initialize opencl: %d\n", ret);
      return -3;
  }
  printf("init_cl -> %i pid:%p nps:%i did:%p nds:%i\n", ret, cl.platform_id, cl.num_platforms, cl.device_id, cl.num_devices);

  field_t field;
  if (ret = init_field(args, &cl, &field))
  {
      printf("could not allocate field: %d\n", ret);
      return -4;
  }

  hotspot_vector_t hotspots;
  if (ret = read_hotspots(args.hotspot_filename, &field, &cl, &hotspots))
  {
    printf("could not read hotspots from: %s (%d)", args.hotspot_filename, ret);
    return -1;
  }

  if (ret = init_kernels(&cl, kernel_source, kernel_source_size))
  {
      printf("could not compile kernels: %d\n", ret);
      return -5;
  }

  int current_round = 0;
  set_hotspots(&cl, &hotspots, &field.buffers[0], current_round);

  while (current_round++ < args.n_rounds)
  {
      cl_mem * const target_buffer = &field.buffers[current_round % 2];
      cl_mem * const source_buffer = &field.buffers[(current_round - 1) % 2];

      /*printf("simulating round:%i src:%p\n", current_round, source_buffer);*/
      /*ret = clEnqueueReadBuffer(cl.command_queue, *source_buffer,*/
      /*CL_TRUE, 0, field.size, (void *) field.values, 0, NULL, NULL);*/
      /*print_field(&field);*/


      simulate_round(&cl, &field, source_buffer, target_buffer);
      set_hotspots(&cl, &hotspots, target_buffer, current_round);

      /*printf("simulating round:%i tgt:%p\n", current_round, target_buffer);*/
      /*ret = clEnqueueReadBuffer(cl.command_queue, *target_buffer,*/
      /*CL_TRUE, 0, field.size, (void *) field.values, 0, NULL, NULL);*/
      /*print_field(&field);*/

  }

  printf("printing round %i\n", current_round);

  ret = clEnqueueReadBuffer(cl.command_queue, field.buffers[(current_round - 1) % 2],
      CL_TRUE, 0, field.size, (void *) field.values, 0, NULL, NULL);

  //Output Coordinates or complete heatmap
  if (argc >= 6) {
    hotspot_vector_t coordinates;
    if (ret = read_coordinates(args.selection_filename, &coordinates))
    {
      printf("could not read coordinates from: %s (%d)", args.selection_filename, ret);
      return -1;
    }
    print_coordinate_values(&field, &coordinates);}
  else
  {
    print_field(&field);
  }
   return 0;
}
