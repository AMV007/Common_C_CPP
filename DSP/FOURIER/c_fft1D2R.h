#ifndef __C_FFT1D2R_H__
#define __C_FFT1D2R_H__

class c_fft1D
{
private:
public:
	// FFT power of 2
	// output in mixed Re and Im results, as input
	static BOOL DoFFT1P(double * data, const DWORD nn);
	//static BOOL DoFFT1P(double * Input, DWORD InputCount, double * OutputRe, double * OutputIm);
};

#endif //__C_FFT1D_H__