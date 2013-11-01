#include <sys/time.h>

typedef struct timeval timeval;

double timediff(timeval a, timeval b)
{
  return 1.0 * (b.tv_sec - a.tv_sec)
       + 0.000001 * (b.tv_usec - a.tv_usec);
}
