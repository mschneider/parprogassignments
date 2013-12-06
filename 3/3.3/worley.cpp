#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <ctime>
#include <cfloat>
#include <omp.h>

using namespace std;

/*****************************************FILENAME DEFINITION***/
const char oname[16]="output.bmp";

/*******************************************STRUCT DEFINITION***/
struct color{
  //v[0]=red, v[1]=green, v[2]=blue
  unsigned char v[3];

  color(unsigned char r, unsigned char g, unsigned char b){
    v[0]=r;
    v[1]=g;
    v[2]=b;
  }
};

/********************************************SIZE DEFINITIONS***/
unsigned hgrid,vgrid;

/*********************************************SEED DEFINITION***/
int permutations[256];

/*****************************************FUNCTION PROTOTYPES***/
int ranInt();
float random(int x, int y, int z);
color lerp(color c1, color c2, float value);//LERP = Linear intERPolation

void fillMap(float* map, float &min, float &max);// this is the algorithm part (the interesting part)
void printMap(float* map, float min, float max);//bitmap part
void printPage(time_t beginning, time_t end);//webpage part

float smoothedNoise(float x,float y);
float euclideanSquared(float x1, float y1, float x2, float y2);
float manhattan(float x1, float y1, float x2, float y2);
float chebychev(float x1, float y1, float x2, float y2);
float quadratic(float x1, float y1, float x2, float y2);

/********************************************************MAIN***/
int main(int argc, char** argv)
{

  if(argc =! 3)
  {
    printf("Wrong number of arguments: <with> <height>");return -1;
  }
  else
  {
     sscanf(argv[1],"%d", &hgrid);
     sscanf(argv[2],"%d", &vgrid);
  }

  time_t beginning = time(NULL),//these two are used to time our algorithm
       end;

  srand((unsigned)10);

  int i,j,k;

  for (i=0;i<256;++i)
  {
    permutations[i]=i;
  }

  for (i=0;i<256;++i)
  {
    j=permutations[i];
    k=ranInt();
    permutations[i]=permutations[k];
    permutations[k]=j;
  }


  float* map = (float*)malloc(sizeof(float)*hgrid*vgrid);//make the empty array

  float min,max;

  //now that the blank map is set up, unblank it
  fillMap(map, min, max);

  //now that we have an interesting map, create a .BMP file
  printMap(map, min, max);

  //finally, output a webpage
  end = time(NULL);//set it to now
  printPage(beginning, end);

  return (0);
}

/***************************************FUNCTION DECLARATIONS***/
color lerp(color c1, color c2, float value){
  color tcolor(0,0,0);

  for (int g=0;g<3;g++){
    if (c1.v[g]>c2.v[g])
      tcolor.v[g]=c2.v[g]+(unsigned char)(float(c1.v[g]-c2.v[g])*value);

    else
      tcolor.v[g]=c1.v[g]+(unsigned char)(float(c2.v[g]-c1.v[g])*value);
  }

  return (tcolor);
}

void fillMap(float* map, float &min, float &max)
{
  const int octaves = 8;

  const float gain = 0.65f; //modifier for the amplitude
  const float lacunarity = 2.0f; //modifier for the frequency

  min=10000.0f;
  max=0.0f;//for averaging

  float local_min = 10000.0f;
  float local_max = 0.0f;

  #pragma omp parallel for collapse(2) reduction(max:local_max) reduction(min:local_min)
  for (int x = 0; x < hgrid; ++x)
  {
    for (int y = 0; y < vgrid; ++y)
    {
      //for each pixel, get the value
      float total = 0.0f;
      float frequency = 1.0f/(float)hgrid;
      float amplitude = 1.0f;
      for (int i = 0; i < octaves; ++i)
      {

        const float offset = (float)i * 7.0f;//this is just to add variance between layers

        total += smoothedNoise((float)x * frequency + offset, (float)y * frequency + offset) * amplitude;

        frequency *= lacunarity;
        amplitude *= gain;
        //  printf("Thread Number  = %d, i = %d\n Offset=%f total=%f frequency=%f amplitude=%f\n",omp_get_thread_num(),i,offset,total,frequency,amplitude);
      }
      // printf("Total gesamt: %f Offset=%f total=%f frequency=%f amplitude=%f\n \n",total,offset,frequency,amplitude);
      //now that we have the value, put it in
      map[x+y * hgrid]  = total;

      //just do some minor calculations while we're here anyway
      if (total < local_min)
        local_min = total;
      if (total > local_max)
        local_max = total;
    }
  }

  min = local_min;
  max = local_max;
}

float random(int x, int y, int z){
  int r1 = permutations[(x + permutations[(y + permutations[z & 255]) & 255]) & 255];

  return((float)r1/256.0f);
}

int ranInt(){
  int r;


  r=rand();


    return (r&255);
}

float smoothedNoise(float x, float y)
{
  int int_x = (int)x,
    int_y = (int)y;

  float frac_x = x - (float)int_x,
      frac_y = y - (float)int_y,
      points[3][3][2],
      temp_dis,
      distance1,distance2;

  //get the point inside the current square, and the point inside the 8 surrounding squares

  for (int i = 0;i<3;++i)
      {
        for (int j = 0; j < 3; ++j)
        {
          int tempx = int_x + i - 1;
          int tempy = int_y + j - 1;
          points[i][j][0] = tempx + random(tempx, tempy, 1);
          points[i][j][1] = tempy + random(tempx, tempy, 2);
        }
      }

  //get the distance to the closest point from x, y
  distance1 = distance2 = FLT_MAX;
  for (int i = 0; i < 3; ++i)
  {
    for (int j = 0; j < 3; ++j)
    {

      temp_dis = euclideanSquared(x,y,points[i][j][0],points[i][j][1]);
      //temp_dis = manhattan(x,y,points[i][j][0],points[i][j][1]);
      //temp_dis = chebychev(x,y,points[i][j][0],points[i][j][1]);
      //temp_dis = quadratic(x,y,points[i][j][0],points[i][j][1]);
      if (temp_dis < distance1)
      {
        if (distance1 < distance2)
          distance2 = distance1;

        distance1 = temp_dis;
      }
      else if (temp_dis < distance2)
        distance2 = temp_dis;

   //  printf("Distance1 in Par Construct:%f Temp_Dis:%f\n",distance1,temp_dis);
    }
  }
//  printf("Distance1: %f\n",distance1);
  return (1.0f - distance1);//this just inverts it
  //return (distance2);
}

void printMap(float* map, float min, float max)
{
//set up some variables
  float diff = max-min,
      flood=0.5f,//flood level
      mount=0.85f;//mountain level

  flood*=diff;
  mount*=diff;

  int i,j,k;

  //these can be changed for interesting results
  color landlow(0,64,0),
      landhigh(116,182,133),
      waterlow(55,0,0),
      waterhigh(106,53,0),
      mountlow(147,157,167),
      mounthigh(226,223,216),
      black(0,0,0),white(255,255,255);

//3.0 output to file
  //3.1 Begin the file
  //3.1.1 open output file
  ofstream out;
  out.open(oname, ofstream::binary);
  if (!(out.is_open())){
    cout << "Target file opening error"<<endl;
    exit(0);
  }

  //3.1.2 copy the header
    //3.1.2.1 magic number
    out.put(char(66));
    out.put(char(77));

    //3.1.2.2 filsize/unused space
    for (i=0;i<8;i++)
      out.put(char(0));

    //3.1.2.3 data offset
    out.put(char(54));

    //3.1.2.4 unused space
    for (i=0;i<3;i++)
      out.put(char(0));

    //3.1.2.5 header size
    out.put(char(40));

    //3.1.2.6 unused space
    for (i=0;i<3;i++)
      out.put(char(0));

    //3.1.2.7 file width (trickier)
    out.put(char(hgrid%256));
    out.put(char((hgrid>>8)%256));
    out.put(char((hgrid>>16)%256));
    out.put(char((hgrid>>24)%256));

    //3.1.2.8 file height (trickier)
    out.put(char(vgrid%256));
    out.put(char((vgrid>>8)%256));
    out.put(char((vgrid>>16)%256));
    out.put(char((vgrid>>24)%256));

    //3.1.2.9 color planes
    out.put(char(1));
    out.put(char(0));

    //3.1.2.10 bit depth
    out.put(char(24));

    //3.1.2.11 the rest
    for (i=0;i<25;i++)
      out.put(char(0));

  //3.2 put in the elements of the array
  color newcolor(0,0,0);

  for (i=(vgrid-1);i>=0;i--){//bitmaps start with the bottom row, and work their way up...
    for (j=0;j<hgrid;j++){//...but still go left to right
      float & val = map[j+i*hgrid];
      val-=min;

      //if this point is below the floodline...
      if (val<flood)
        newcolor=lerp(waterlow,waterhigh,val/flood);

      //if this is above the mountain line...
      else if (val>mount)
        newcolor=lerp(mountlow,mounthigh,(val-mount)/(diff-mount));

      //if this is regular land
      else
        newcolor=lerp(landlow,landhigh,(val-flood)/(mount-flood));

      //uncomment the line below to make it black 'n' white
      //newcolor = lerp(black,white,map[j][i]/diff);

      out.put(char(newcolor.v[0]));//blue
      out.put(char(newcolor.v[1]));//green
      out.put(char(newcolor.v[2]));//red
    }
    //round off the row
    for (k=0;k<(hgrid%4);k++)
      out.put(char(0));
  }

  //3.3 end the file
  out.close();
}

void printPage(time_t beginning, time_t end)
{
  double timeTaken = double(end-beginning);
  cout<<"Content-Type: text/html\n\n"
    <<"<html><head><title>FTG Page</title></head>\n"
    <<"<body>\n"
    <<"<h2>Fractal Terrain Generator Page</h2>\n"
    <<"<img src=\"test.bmp\" /><br />\n"
    <<"This took " << timeTaken << " seconds to create.<br />\n"
    <<"</body>\n"
    <<"</html>\n";
}

float euclideanSquared(float x1, float y1, float x2, float y2)
{
  float dif_x = x1-x2,
      dif_y = y1-y2;

  return (dif_x * dif_x + dif_y * dif_y);
}

float manhattan(float x1, float y1, float x2, float y2)
{
  float dif_x = x1 - x2,
      dif_y = y1 - y2;

  return (fabs(dif_x) + fabs(dif_y));
}

float chebychev(float x1, float y1, float x2, float y2)
{
  float dif_x = fabs(x1 - x2),
      dif_y = fabs(y1 - y2);

  if (dif_x < dif_y)
    return (dif_x);
  else
    return (dif_y);

}

float quadratic(float x1, float y1, float x2, float y2)
{
  float dif_x = x1 - x2,
      dif_y = y1 - y2;

  return (dif_x * dif_x + dif_x * dif_y + dif_y * dif_y);
}
