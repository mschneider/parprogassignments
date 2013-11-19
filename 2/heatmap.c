#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct arg_t
{
  int width_field;
  int height_field;
  int n_rounds;
  int act_round;
  char* hotspot_file;
  char* coord_file;
} arg_t;


typedef struct hotpoint
{
  int coord_x;
  int coord_y;
  int start_r;
  int end_r;
}hotpoint;

typedef struct field
{
  float* source_field;
  float* new_field;
}field;


/* Funktionen zum einlesen von Werten*/
void read_args(char** argv,arg_t* args,int* argc) //Warum Zeiger auf Zeiger ?!?!
{
  sscanf(argv[1],"%d", &args->width_field);
  sscanf(argv[2],"%d", &args->height_field);
  sscanf(argv[3],"%d", &args->n_rounds);
  &args->act_round = 0; //Warum=?!?!?!
  sscanf(argv[4],"%s", &args->hotspot_file);
  if(&argc == 5) //Test if 5th argument is given
  {
  sscanf(argv[5],"%s", &args->coord_file);
  }

}


int read_hotspot(char* const filename)
{
  FILE *filepointer;
  filepointer = fopen(filename,"r");
  /*lesen von File ab zweiter Zeile und für jede Zeile ein neues Struct hotspot*/


}


/*Funktionen die Structs initialisieren*/

void initialize_field(field* fields,arg_t* args)
{
 int counter_i;
 int counter_j;

 fields->source_field = calloc(args->width_field*args->height_field, sizeof(float));
 fields->new_field = calloc(args->width_field*args->height_field, sizeof(float));
 for (counter_i=0;counter_i<args->width_field;counter_i++) //Komplette auf 0 setzen
   {
     for(counter_j=0;counter_j<args->height_field;counter_j++)
     {
       fields->source_field[counter_i+counter_j*args->width_field]=0;
       fields->new_field[counter_i+counter_j*args->width_field]=0;
     }
   }

}




/*Funktionen zur Berechnung neuer Hotpoints*/

int set_hotpoints(hotpoint* hotpoints,field* fields)

  {
  
  //Funktion die Hotpoint für jede Runde setzt in new_array aus Struct Hotpoint
  
  }



void calcualte_new_heat_map()
  {
  }

int main(int argc, char** argv)
{
  arg_t args;
  field *fields = (field*) malloc (sizeof(field)); //Warum muss hier Malloc und bei arg_t nicht?!?!?;
  int counter = 0; //Zaehlvariable fuer Rounds

  if(argc !=4)
  {
    puts("Wrong Arguments. Usage: <Width> <Height> <Rounds> <Filename_hotspot.csv> <Filename_coordinates.csv>");
    return -1;
  }

  read_args(argv,&args,&argc);
  read_hotpots(&args.hotspot_file);
  initialize_field(&fields,&args);

  for(counter=0;args.n_rounds < counter;counter++)
    {

    //nach jedem Durchlauf wird new_field zu source_field indem Zeigeradressen gedreht werden
    
    }

}
