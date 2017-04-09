#include <cstdlib>
#include <cmath>
#include <iostream>

using namespace std;

#define Usage "Usage: ./gradientD stepSize accuracy initialPoint"

int main(int argc, char **argv)
{
	// check if arguments number is correct
	if(argc^4)
	{
		cout<<Usage<<endl;
		exit(1);
	}
	float c, epsilon, x0;
	c=atof(argv[1]);
	epsilon=atof(argv[2]);
	x0=atof(argv[3]);
	int N=20;
	float *x= new float [N];
	cout<<"Local minima \t\t x"<<endl;
	float xT=x0;
	float xT1=0;
	int finish=0;
	while(~finish)
	{
		xT1=xT-c*(50*cos(xT)+xT);
		if(fabs(xT1-xT)<epsilon)
		{
			finish=1;
		}
		else
		{
			xT=xT1;
		}
	}
	cout<<(50*sin(xT)+xT*xT)<<"\t \t \t\t "<<xT<<endl;
	return 0;
}
