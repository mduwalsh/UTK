#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// struct for body
typedef struct body{
  double m;                // mass
  double x; double y;      // x and y position
  double vx; double vy;    // velocity x and y direction
  double ax; double ay;    // acceleration x and y direction
} Body;

//*** global variables ***//
Body B[3];
double DT;
int STEP_LENGTH;


double square (double x)
{
  return x*x;
}

// simulation for a step for n bodies
void simulation_nbody(Body *bd, int n, double dt)
{
  int i, j;
  double f;  // this could be made array and no. of calculation could be decreased but for now naive approach
  Body *b = malloc(sizeof(Body)*n);
  for(i = 0; i < n; i++){
    b[i].m = bd[i].m;
    b[i].x = bd[i].x;
    b[i].y = bd[i].y;
    b[i].vx = bd[i].vx;
    b[i].vy = bd[i].vy;   
  }
  
  for(i = 0; i < n; i++){
    // initialize acceleration to 0 ; calculated as sum of acceleration due to other bodies
    bd[i].ax = 0; bd[i].ay = 0;   	
    // calculate acceleration due to forces
    for(j = 0; j < n; j++){
      if(i == j) continue;    // no sense in calculating force from itself
      // calculate acceleration due to other body
      f = sqrt ( pow( square( b[i].x - b[j].x) + square(b[i].y - b[j].y), 3 ) );  // cube of distance between two bodies
      bd[i].ax += - b[j].m * ( b[i].x - b[j].x ) / f;
      bd[i].ay += - b[j].m * ( b[i].y - b[j].y ) / f;			 
    }	
    // update position of body      s = ut + 1/2*a*t^2
    bd[i].x += (bd[i].vx + 0.5*bd[i].ax*dt)*dt;
    bd[i].y += (bd[i].vy + 0.5*bd[i].ay*dt)*dt;
    // update velocity of a body    v = u + at;
    bd[i].vx += bd[i].ax * dt;           
    bd[i].vy += bd[i].ay * dt;	
  }
  // update other properties of body
  
  free(b);
}


#define EXPECT(a,b,c) if ((a) != fscanf(f,b "%*[ ^\n]\n",c)){ fclose(f); printf("Error: %s\n",b); return 1; }

int read_config(char *file_name)
{
  FILE *f;
  
  if (!(f = fopen(file_name,"r"))) return 1;
  EXPECT(1, "int    steps  = %d;", &STEP_LENGTH);
  EXPECT(1, "double period = %lf;", &DT);
  EXPECT(1, "double mass   = %lf;", &B[0].m);
  EXPECT(1, "double pos_x  = %lf;", &B[0].x);
  EXPECT(1, "double pos_y  = %lf;", &B[0].y);
  EXPECT(1, "double vel_x  = %lf;", &B[0].vx);
  EXPECT(1, "double vel_y  = %lf;", &B[0].vy);
  EXPECT(1, "double mass   = %lf;", &B[1].m);
  EXPECT(1, "double pos_x  = %lf;", &B[1].x);
  EXPECT(1, "double pos_y  = %lf;", &B[1].y);
  EXPECT(1, "double vel_x  = %lf;", &B[1].vx);
  EXPECT(1, "double vel_y  = %lf;", &B[1].vy);
  EXPECT(1, "double mass   = %lf;", &B[2].m);
  EXPECT(1, "double pos_x  = %lf;", &B[2].x);
  EXPECT(1, "double pos_y  = %lf;", &B[2].y);
  EXPECT(1, "double vel_x  = %lf;", &B[2].vx);
  EXPECT(1, "double vel_y  = %lf;", &B[2].vy);
  
  fclose(f); return 0;
}
#undef EXPECT


int main(int argc, char** argv)
{
  if (argc ^ 2){
    printf("Usage: ./threebody config_file\n");
    exit(1);
  }
  if (read_config(argv[1])){                    // read config file
    printf("READDATA: Can't process %s\n", argv[1]);
    return 1; 
  }
  
  int i,j;
  clock_t start, end;
  start = clock();                    // start of simulation for all steps
  for( j = 0; j < 10; j++){
    for( i = 0; i < STEP_LENGTH; i++){
      simulation_nbody(B, 3, DT);
    }
  }
  end = clock();
  // print stats for each bodies
  for( i = 0; i < 3; i++){
    printf("Body #%d:\n", i+1);
    printf("mass: %.2f\n", B[i].m);
    printf("position: %.2f, %.2f\n", B[i].x, B[i].y);
    printf("velocity: %.2f, %.2f\n", B[i].vx, B[i].vy);
    printf("acceleration: %.8f, %.8f\n", B[i].ax, B[i].ay);
    printf("\n");
  }
  printf("time of execution: %.8f\n", (double)(end-start)/(double)(CLOCKS_PER_SEC*10));
  
  return 0;
}


