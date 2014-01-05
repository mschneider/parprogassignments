typedef struct hotspot
{
  int x;
  int y;
  int offset;
  int start;
  int end;
} hotspot_t;

typedef float field_value_t;

__kernel void set_hotspots(
    __global int current_round,
    __global const hotspot_t * hotspots,
    __global field_value_t * buffer)
{
    const int i = get_global_id(0);

    if ( current_round >= hotspots[i].start &&
         current_round <  hotspots[i].end )
    {
        buffer[hotspots[i].offset] = 1.0;
    }
}

int offset(const int width, const int x, const int y)
{
    const size_t line_size = width + 2;
    const size_t offset_0_0 = line_size + 1;
    return offset_0_0 + y * line_size + x;
}

__kernel void simulation(
    __global const field_value_t * source_buffer,
    __global field_value_t * target_buffer)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const size_t width = get_global_size(0);
    field_value_t new_value = 0.0;
    new_value += source_buffer[offset(width, x-1, y-1)];
    new_value += source_buffer[offset(width, x-1, y)];
    new_value += source_buffer[offset(width, x-1, y+1)];
    new_value += source_buffer[offset(width, x, y-1)];
    new_value += source_buffer[offset(width, x, y)];
    new_value += source_buffer[offset(width, x, y+1)];
    new_value += source_buffer[offset(width, x+1, y-1)];
    new_value += source_buffer[offset(width, x+1, y)];
    new_value += source_buffer[offset(width, x+1, y+1)];
    new_value /= 9.0;
    target_buffer[offset(width, x, y)] = new_value;
}


