#include <stdio.h>
#include "math.h"

double pow(double x, int y)
{
    int i;
    double res = 1;
    if (y == 0)
        return res;
    else if (y > 0)
    {
        for (i = 0; i < y; i++)
            res = res * x;
        return res;
    }
    else
    {
        for (i = 0; i > y; i--)
            res = res / x;
        return res;
    }
}

double log(double x)
{
    int n = 20;
    int i;
    double result = x - 1;

    for (i = 2; i < n; i++)
    {
        result += pow(-1, i - 1) * pow(x - 1, i) / i;
    }
    return result;
}

double expdev(double lambda)
{
    double dummy;
    do
        dummy = (double)rand() / RAND_MAX;
    while (dummy == 0.0);
    return -log(dummy) / lambda;
}

