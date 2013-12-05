#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <errno.h>
#include <omp.h>

//Datenstrukturen

struct user {
char* username;
char* crypt_salt;
char  salt[2];
} user_t

struct dictionary {
char *entry
} dictionary_t

int main (int argc, char** argv)
{
   char* userpassword = calloc(40,sizeof(char));
   char* passdictionary = calloc(40,sizeof(char));
   if(argc =! 3)
  {
    printf("Wrong number of arguments: <with> <height>");return -1;
  }
  else
  {
     sscanf(argv[1],"%d", &hgrid);
     sscanf(argv[2],"%d", &vgrid);
  }



}




