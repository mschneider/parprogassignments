#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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



void read_passwords (char userpassword,user_t users)
{
//Open File
FILE* file = fopen(filename, "r");
  if (NULL == file)
    return -2;
  int counter=0;
  user_t users=callloc(2000,sizeof(user_t));
   while(1)
    {
      users[counter]=calloc(1,sizeof(user_t));
      if (fscanf(file,"%s:%s",
      &login_hour,&login_min,&date_day,&date_mon,&date_year,
      &logout_hour,&logout_min,&date_dayx,&date_monx,
      &date_yearx,name)) != EOF )
    }



}

void read_dictionary(char passdictionary,dictionary_t dictionary);


int main (int argc, char** argv)
{
   user_t* users;
   char* userpassword = calloc(40,sizeof(char));
   char* passdictionary = calloc(40,sizeof(char));

   if(argc =! 3)
  {
    printf("Wrong number of arguments: <with> <height>");return -1;
  }
  else
  {
     sscanf(argv[1],"%d", &userpassword);
     sscanf(argv[2],"%d", &passdictionary);
  }
read_passwords(&userpassword,&users);
read_dictionary(&passdictionary,&dictionary);



}




