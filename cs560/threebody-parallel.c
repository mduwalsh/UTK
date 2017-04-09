#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>

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
void simulation_nbody(Body *b, int n, int step_length, double dt)
{
  int i,j,k,l;
  Body *bd = malloc(n*sizeof(Body));
  double *f = malloc(n*sizeof(double));
  #pragma omp parallel private(k)
  {
    for( k = 0; k < step_length; k++){
      #pragma omp for
      for(l = 0; l < n; l++){
	bd[l].m = B[l].m;
	bd[l].x = B[l].x;
	bd[l].y = B[l].y;
	bd[l].vx = B[l].vx;
	bd[l].vy = B[l].vy;	
      }
      
#pragma omp for private(j)
      for(i = 0; i < n; i++){
	// initialize acceleration to 0 ; calculated as sum of acceleration due to other bodies
	b[i].ax = 0; b[i].ay = 0;   	
	// calculate acceleration due to forces
	for(j = 0; j < n; j++){
	  if(i == j) continue;    // no sense in calculating force from itself
	  // calculate acceleration due to other body
	  f[i] = sqrt ( pow( square( bd[i].x - bd[j].x) + square(bd[i].y - bd[j].y), 3 ) );  // cube of distance between two bodies
	  b[i].ax += - bd[j].m * ( bd[i].x - bd[j].x ) / f[i];
	  b[i].ay += - bd[j].m * ( bd[i].y - bd[j].y ) / f[i];			 
	}	
	// update position of body      s = ut + 1/2*a*t^2
	b[i].x += (b[i].vx + 0.5*B[i].ax*dt)*dt;
	b[i].y += (b[i].vy + 0.5*B[i].ay*dt)*dt;
	// update velocity of a body    v = u + at;
	b[i].vx += b[i].ax * dt;           
	b[i].vy += b[i].ay * dt;	
      }
    }
  }
  free(bd);
  free(f);
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
  
  int i;
  time_t start, end;
  time(&start);                    // start of simulation for all steps
  //Body bd[3]; // = malloc(sizeof(Body)*3);
  
  //double f[3];  
  omp_set_num_threads(3);
  for(i = 0; i < 10; i++){
    simulation_nbody(B, 3, STEP_LENGTH, DT);
  }
  time(&end);
  
  // print stats for each bodies
  for( i = 0; i < 3; i++){
    printf("Body #%d:\n", i+1);
    printf("mass: %.2f\n", B[i].m);
    printf("position: %.2f, %.2f\n", B[i].x, B[i].y);
    printf("velocity: %.2f, %.2f\n", B[i].vx, B[i].vy);
    printf("acceleration: %.8f, %.8f\n", B[i].ax, B[i].ay);
    printf("\n");
  }
  printf("time of execution: %.7f\n", (double)difftime(end,start)/(double)10);
  //free(b);

  
	return 0;
}


