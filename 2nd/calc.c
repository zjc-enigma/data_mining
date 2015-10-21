#include "calc.h"

float dist(mail x, mail y)
{
    float sigma;
    sigma = 0;
    for(int i = 0; i < 57; i++)
	sigma += pow(abs(x.data[i] - y.data[i]), 2);	    
    return sqrt(sigma);
}
