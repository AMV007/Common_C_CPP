#include "stdafx.h"
#include "..\\DSP\\FOURIER\\c_fft1D2R.h"
#include "..\\DSP\\FOURIER\\c_fft1D2R_templ.h"
//#include "..\\DSP\\FOURIER\\FFTW\\c_fftw.h"
#include "..\\DSP\\c_generator.h"

/*
#include "D:\\Program Files\\Intel\\Compiler\\11.1\\038\\mkl\\include\\mkl_dfti.h"
#include "D:\\Program Files\\Intel\\Compiler\\11.1\\038\\mkl\\examples\\dftc\\source\\mkl_dfti_examples.h"
#pragma comment( lib, "D:\\Program Files\\Intel\\Compiler\\11.1\\038\\mkl\\ia32\\lib\\mkl_core.lib")
*/
#include "..\\..\\Common_CVI\\DigitalControlsMathapi\\DigitalControlsMathapi.h"
#pragma comment( lib, "..\\..\\Common_CVI\\DigitalControlsMathapi\\msvc\\DigitalControlsMathapi.lib")

//#pragma comment( lib, "D:\\Program Files\\Intel\\Compiler\\11.1\\038\\mkl\\ia32\\lib\\mkl_cdft_core.lib")
//#pragma comment( lib, "D:\\Program Files\\Intel\\Compiler\\11.1\\038\\mkl\\ia32\\lib\\mkl_intel_c.lib")
//#pragma comment( lib, "D:\\Program Files\\Intel\\Compiler\\11.1\\038\\mkl\\ia32\\lib\\mkl_intel_s.lib")

#include <iostream>
#include <ctype.h>

void SaveData(TCHAR * FileName, double * Data, int DataLen)
{
	FILE * wfile=fopen(FileName, "w");
	if(wfile!=NULL)
	{
		for(int i=0;i<DataLen;i++)
		{
			CHAR TempDataValue[128];
			sprintf_s(TempDataValue, "%.3f\n",Data[i]);
			fwrite(TempDataValue, sizeof(CHAR),strlen(TempDataValue), wfile);
		}
		fclose(wfile);
	}
}

//#include "c_fft1D_templ.h"
void c_fft1D_templ_test()
{
	// testing different methods of FFT computation
	c_generator<double> generator;

#define FREQ_NUM	1
	double FreqArray[FREQ_NUM]={100};//, 1000, 3000};
	generator.SetParamAmpl(FreqArray,44100);

#define FFT_SIZE 32768
	double * GeneratorData = new double [FFT_SIZE];
	generator.GetVals(GeneratorData, FFT_SIZE);
	SaveData(TEXT("e:\\FFTinput.txt"),GeneratorData, FFT_SIZE);

	double * TestData = new double[FFT_SIZE*2];
	{ // standard FFT algorithm
		for(int i=0;i<FFT_SIZE;i++)
		{
			TestData[i*2] =GeneratorData[i];
			TestData[i*2+1]=0;
		}	


		c_fft1D_templ<FFT_SIZE> xx;
		xx.DoFFT(TestData);
		SaveData(TEXT("e:\\FFTOutClass.txt"),TestData, FFT_SIZE*2);	
	}

	{ // template FFT algorithm
		for(int i=0;i<(FFT_SIZE);i++)
		{
			TestData[i*2] =GeneratorData[i];
			TestData[i*2+1]=0;
		}	
		c_fft1D::DoFFT1P(TestData,FFT_SIZE);	
		SaveData(TEXT("e:\\FFTOutTemplate.txt"),TestData, FFT_SIZE*2);
	}

	{ // algorithm from CVI		
		double * Re = new double[FFT_SIZE];
		double * Im = new double[FFT_SIZE];
		_DoSimpleFFT(GeneratorData, FFT_SIZE, 0, Re, Im, -1);
		for(int i=0;i<(FFT_SIZE);i++)
		{
			TestData[i*2] =Re[i];
			TestData[i*2+1]=Im[i];
		}	
		delete Re;
		delete Im;
		SaveData(TEXT("e:\\FFTOutCVI.txt"),TestData, FFT_SIZE*2);
	}

	delete TestData;
	delete GeneratorData;
}
