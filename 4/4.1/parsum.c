#define _ISOC99_SOURCE
#include "helper.h"
#include "math.h"

int main(int argc, char * * argv)
{
    if (argc != 3)
    {
        printf("usage: %s <start> <end>\n", argv[0]);
        return -1;
    }

    uint64_t start = atoll(argv[1]);
    uint64_t end = atoll(argv[2]);
    double total_work = end - start + 1;
    uint64_t num_kernels = 1024;
    double work_per_kernel = total_work / num_kernels + 1;

    printf("start:%llu end:%llu num_kernels:%llu total_work:%f work_per_kernel:%f\n", start, end, num_kernels, total_work, work_per_kernel);

    uint64_t * starts = (uint64_t *) calloc(sizeof(uint64_t), num_kernels);
    uint64_t * ends = (uint64_t *) calloc(sizeof(uint64_t), num_kernels);
    uint64_t * results = (uint64_t *) calloc(sizeof(uint64_t), num_kernels);

    for (uint64_t next_start = start, i = 0; i < num_kernels; ++i)
    {
        starts[i] = next_start;
        ends[i] = MIN(end, round(next_start + work_per_kernel));
        next_start = ends[i] + 1;
    }

    char * kernel_source;
    size_t kernel_source_size;

    if (read_file("parsum.cl", &kernel_source, &kernel_source_size))
        return -2;

    printf("Read kernel file (%i bytes).\n", (int)kernel_source_size);

    struct cl cl;
    int error;
    error = init_cl(&cl);
    printf("init_cl -> error:%i pid:%p nps:%i did:%p nds:%i\n", error, cl.platform_id, cl.num_platforms, cl.device_id, cl.num_devices);
    if (error)
        return -3;

    cl_int ret;
    cl_mem kernel_starts = clCreateBuffer(cl.context, CL_MEM_READ_ONLY,
              num_kernels * sizeof(uint64_t), NULL, &ret);
    cl_mem kernel_ends   = clCreateBuffer(cl.context, CL_MEM_READ_ONLY,
              num_kernels * sizeof(uint64_t), NULL, &ret);
    cl_mem kernel_results = clCreateBuffer(cl.context, CL_MEM_WRITE_ONLY,
              num_kernels * sizeof(uint64_t), NULL, &ret);

    ret = clEnqueueWriteBuffer(cl.command_queue, kernel_starts, CL_TRUE, 0,
            num_kernels * sizeof(uint64_t), starts, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(cl.command_queue, kernel_ends, CL_TRUE, 0,
            num_kernels * sizeof(uint64_t), ends, 0, NULL, NULL);

    error = init_kernel(&cl, kernel_source, kernel_source_size);
    printf("init_kernel -> error:%i\n", error);
    if (error)
        return -4;

    ret = clSetKernelArg(cl.kernel, 0, sizeof(cl_mem), (void *)&kernel_starts);
    ret = clSetKernelArg(cl.kernel, 1, sizeof(cl_mem), (void *)&kernel_ends);
    ret = clSetKernelArg(cl.kernel, 2, sizeof(cl_mem), (void *)&kernel_results);

    size_t global_item_size = num_kernels;
    size_t local_item_size = 1; // mac os x cpus are incapable of using more then 1 local item
    ret = clEnqueueNDRangeKernel(cl.command_queue, cl.kernel, 1, NULL,
        &global_item_size, &local_item_size, 0, NULL, NULL);
    ret = clEnqueueReadBuffer(cl.command_queue, kernel_results, CL_TRUE, 0,
        num_kernels * sizeof(uint64_t), results, 0, NULL, NULL);

    uint64_t result = 0;
    while (num_kernels--)
    {
        result += results[num_kernels];
    }
    printf("result:%llu\n", result);

    FILE * output = fopen("output.txt", "w");
    fprintf(output, "%llu", result);
    fclose(output);

    return 0;
}
