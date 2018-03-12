#include "stdafx.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include "c_fft1D2R.h"
/*
main idea is in nested call instead loops,
because in template programming nested calls unrolling in
compilation time, thus we have time saving with more larger memory usage
*/

/* 
	common Cooley - Turkey Algorithm
	Output size must be InputCount/2
	InputCount must be power of 2
	2P - power of 2
*/
BOOL c_fft1D::DoFFT1P(double * data, 
					const DWORD nn					
					)
{
	/*if(	Input==NULL||Output==NULL||
		(InputCount&0x1)|| InputCount<2) return FALSE; // must be odd and power of 2
	
	{
		//we will not check right power, we will just find it
		//and take nearest power of 2
		int power=0;
		for(;power<32&&InputCount;power++)InputCount>>=1;
		InputCount=1<<(power-1);	
	}*/
	
    unsigned long n, mmax, m, j, istep, i;
    double wtemp, wr, wpr, wpi, wi, theta;
    double tempr, tempi;

    // reverse-binary reindexing
    n = nn<<1;
    j=1;
    for (i=1; i<n; i+=2) {
        if (j>i) {
			//swap(data[j-1], data[i-1]);
			{
				register double TempData=data[j-1];
				data[j-1]=data[i-1];
				data[i-1]=TempData;
			}

			{
				register double TempData=data[j];
				data[j]=data[i];
				data[i]=TempData;
			}
        }
        m = nn;
        while (m>=2 && j>m) {
            j -= m;
            m >>= 1;
        }
        j += m;
    };

    // here begins the Danielson-Lanczos section
    mmax=2;
    while (n>mmax) {
        istep = mmax<<1;
        theta = -(2*M_PI/mmax);
        wtemp = sin(0.5*theta);
        wpr = -2.0*wtemp*wtemp;
        wpi = sin(theta);
        wr = 1.0;
        wi = 0.0;
        for (m=1; m < mmax; m += 2) {
            for (i=m; i <= n; i += istep) {
                j=i+mmax;
                tempr = wr*data[j-1] - wi*data[j];
                tempi = wr * data[j] + wi*data[j-1];

                data[j-1] = data[i-1] - tempr;
                data[j] = data[i] - tempi;
                data[i-1] += tempr;
                data[i] += tempi;
            }
            wtemp=wr;
            wr += wr*wpr - wi*wpi;
            wi += wi*wpr + wtemp*wpi;
        }
        mmax=istep;
    }
    
	return TRUE;
}

