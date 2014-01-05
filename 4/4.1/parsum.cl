__kernel void parsum(
    __global const unsigned long * starts,
    __global const unsigned long * ends,
    __global unsigned long * results)
{
    // Get the index of the current element to be processed
    int i = get_global_id(0);
    const unsigned long start = starts[i];
    const unsigned long end = ends[i];

    // Do the operation
    unsigned long accum = 0;
    for (unsigned long j = 0; start + j <= end; ++j)
    {
        accum += start + j;
    }
    results[i] = accum;
}




