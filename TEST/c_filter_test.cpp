#include "stdafx.h"
#include "..\\DSP\\FILTER\\c_filter.h"

void TestFilter()
{
	//c_iir<float> * ttx = new c_iir<float>(NULL,NULL,NULL,NULL);
	//delete ttx;

	double coeff[12];
	c_fir<float, double> TestFlt;	

	coeff[0]=0.8;
	TestFlt.InitFilter(coeff,12);
	//float tt1=TestFlt.Write(1);
	//tt1=tt1;
}