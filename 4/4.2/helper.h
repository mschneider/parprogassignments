#include <stdio.h>
#include <stdlib.h>

#define MIN(a, b) (a < b ? a : b)

#define MAX_FILE_SIZE 1024 * 1024
int read_file(
    char * filename, // in
    char * * target, // out
    size_t * size)   // out
{
    FILE * file = fopen(filename, "r");
    if (! file)
    {
        fprintf(stderr, "Failed to open file '%s'.\n", filename);
        return 1;
    }

    *target = (char *) malloc(MAX_FILE_SIZE);
    *size = fread(*target, 1, MAX_FILE_SIZE, file);

    fclose(file);

    if (*size == MAX_FILE_SIZE)
    {
        fprintf(stderr, "File was longer then MAX_FILE_SIZE(%i).\n", MAX_FILE_SIZE);
        return 2;
    }

    return 0;
}

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

typedef struct cl
{
    cl_platform_id platform_id;
    cl_platform_id platform_ids[2];
    cl_uint num_platforms;

    cl_device_id device_id;
    cl_device_id device_ids[2];
    cl_uint num_devices;

    cl_context context;
    cl_command_queue command_queue;

    cl_program program;
    cl_kernel hotspot_kernel;
    cl_kernel simulation_kernel;
} cl_t;

int init_cl(cl_t * cl)
{
    cl->platform_id = NULL;
    cl->device_id = NULL;

    cl_int ret;
    ret = clGetPlatformIDs(2, cl->platform_ids, &cl->num_platforms);
    if (ret != CL_SUCCESS)
        return -1;
    cl->platform_id = cl->platform_ids[cl->num_platforms - 1];

    ret = clGetDeviceIDs( cl->platform_id, CL_DEVICE_TYPE_DEFAULT, 2, cl->device_ids, &cl->num_devices);
    if (ret != CL_SUCCESS)
        return -2;
    cl->device_id = cl->device_ids[cl->num_devices - 1];

    cl->context = clCreateContext( NULL, 1, &cl->device_id, NULL, NULL, &ret);
    if (ret != CL_SUCCESS)
        return -3;

    cl->command_queue =  clCreateCommandQueue(cl->context, cl->device_id, 0, &ret);
    if (ret != CL_SUCCESS)
        return -4;

    return 0;
}

int init_kernels(
    cl_t * cl,
    char * source,
    size_t source_size)
{
    cl_int ret;
    cl->program = clCreateProgramWithSource(cl->context, 1, (const char * *) &source, &source_size, &ret);
    if (ret != CL_SUCCESS)
        return -1;

    ret = clBuildProgram(cl->program, 1, &cl->device_id, NULL, NULL, NULL);
    if (ret == CL_BUILD_PROGRAM_FAILURE) {
        // Determine the size of the log
        size_t log_size;
        clGetProgramBuildInfo(cl->program, cl->device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

        // Allocate memory for the log
        char *log = (char *) malloc(log_size);

        // Get the log
        clGetProgramBuildInfo(cl->program, cl->device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

        // Print the log
        printf("build log:\n%s\n", log);
    }
    if (ret != CL_SUCCESS)
        return ret;

    cl->hotspot_kernel = clCreateKernel(cl->program, "set_hotspots", &ret);
    if (ret != CL_SUCCESS)
        return -3;

    cl->simulation_kernel = clCreateKernel(cl->program, "simulation", &ret);
    if (ret != CL_SUCCESS)
        return -4;

    return 0;
}



#include <sys/time.h>

typedef struct timeval timeval;

double timediff(timeval a, timeval b)
{
  return 1.0 * (b.tv_sec - a.tv_sec)
       + 0.000001 * (b.tv_usec - a.tv_usec);
}

