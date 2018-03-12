#ifndef __C_FFT1D2R_TEMPL_H__
#define __C_FFT1D2R_TEMPL_H__

/*
this template contains class for
unrolling fft with nesting classes in template,
also optimizing calculating sin and cos,
and removing roots from calculations

using DanielsonLanczos algorithm

template<unsigned N, typename T=double>
N - number of fourier data, power of 2
input data array size must be N*2
*/

#define M_PI_MY 3.14159265358979323846264338327950288419716939937510


////// template class DanielsonLanczos
// Danielson-Lanczos section of the FFT

////// template class SinCosSeries
// common series to compile-time calculation
// of sine and cosine functions

template<unsigned M, unsigned N, unsigned B, unsigned A>
struct SinCosSeries {
	static double value() {
		return 1-(A*M_PI_MY/B)*(A*M_PI_MY/B)/M/(M+1)
			*SinCosSeries<M+2,N,B,A>::value();
	}
};

template<unsigned N, unsigned B, unsigned A>
struct SinCosSeries<N,N,B,A> {
	static double value() { return 1.; }
};

////// template class Sin
// compile-time calculation of sin(A*M_PI/B) function

template<unsigned B, unsigned A, typename T=double>
struct Sin;

template<unsigned B, unsigned A>
struct Sin<B,A,float> {
	static float value() {
		return (A*M_PI_MY/B)*SinCosSeries<2,24,B,A>::value();
	}
};
template<unsigned B, unsigned A>
struct Sin<B,A,double> {
	static double value() {
		return (A*M_PI_MY/B)*SinCosSeries<2,34,B,A>::value();
	}
};

////// template class Cos
// compile-time calculation of cos(A*M_PI/B) function

template<unsigned B, unsigned A, typename T=double>
struct Cos;

template<unsigned B, unsigned A>
struct Cos<B,A,float> {
	static float value() {
		return SinCosSeries<1,23,B,A>::value();
	}
};
template<unsigned B, unsigned A>
struct Cos<B,A,double> {
	static double value() {
		return SinCosSeries<1,33,B,A>::value();
	}
};

template<unsigned N, typename T>
class c_fft1D_templ_base
{		
public:
	// public only due to nesting classes
	static void scramble(T* data) {
		unsigned int i,m,j=1;
		for (i=1; i<2*N; i+=2) {
			if (j>i) {
				//swap(data[j-1], data[i-1]);
				{
					register T TempData = data[j-1];
					data[j-1]=data[i-1];
					data[i-1]=TempData;
				}
				
				//swap(data[j], data[i]);
				{
					register T TempData = data[j];
					data[j]=data[i];
					data[i]=TempData;
				}
			}
			m = N;
			while (m>=2 && j>m) {
				j -= m;
				m >>= 1;
			}
			j += m;
		}
	} 
};

template<unsigned N, typename T=double>
class c_fft1D_templ{

#ifdef _MSC_VER
	friend class c_fft1D_templ;
	private:
#else
	public:
#endif
	c_fft1D_templ<N/2,T> next;	

	void apply(T* data) {
		next.apply(data);
		next.apply(data+N);

		T wtemp,tempr,tempi,wr,wi,wpr,wpi;
		//    Change dynamic calculation to the static one
		//      wtemp = sin(M_PI/N);
		wtemp = -Sin<N,1,T>::value();
		wpr = -2.0*wtemp*wtemp;
		//    Change dynamic calculation to the static one
		//      wpi = -sin(2*M_PI/N);
		wpi = -Sin<N,2,T>::value();
		wr = 1.0;
		wi = 0.0;
		for (unsigned i=0; i<N; i+=2) {
			tempr = data[i+N]*wr - data[i+N+1]*wi;
			tempi = data[i+N]*wi + data[i+N+1]*wr;
			data[i+N] = data[i]-tempr;
			data[i+N+1] = data[i+1]-tempi;
			data[i] += tempr;
			data[i+1] += tempi;

			wtemp = wr;
			wr += wr*wpr - wi*wpi;
			wi += wi*wpr + wtemp*wpi;
		}
	}
public:
	// output in mixed Re and Im results, as input
	bool DoFFT(T* data)
	{
		if(N&1) return false; // check N - even
		c_fft1D_templ_base<N,T>::scramble(data);
		apply(data);
		return true;
	}
};



template<typename T>
class c_fft1D_templ<4,T>{
#ifdef _MSC_VER
	friend class c_fft1D_templ;
	private:
#else
	public:
#endif
	void apply(T* data) {
		T tr = data[2];
		T ti = data[3];
		data[2] = data[0]-tr;
		data[3] = data[1]-ti;
		data[0] += tr;
		data[1] += ti;
		tr = data[6];
		ti = data[7];
		data[6] = data[5]-ti;
		data[7] = tr-data[4];
		data[4] += tr;
		data[5] += ti;

		tr = data[4];
		ti = data[5];
		data[4] = data[0]-tr;
		data[5] = data[1]-ti;
		data[0] += tr;
		data[1] += ti;
		tr = data[6];
		ti = data[7];
		data[6] = data[2]-tr;
		data[7] = data[3]-ti;
		data[2] += tr;
		data[3] += ti;
	}
};

template<typename T>
class c_fft1D_templ<2,T> {
#ifdef _MSC_VER
	friend class c_fft1D_templ;
#endif
private:
	void apply(T* data) {
		T tr = data[2];
		T ti = data[3];
		data[2] = data[0]-tr;
		data[3] = data[1]-ti;
		data[0] += tr;
		data[1] += ti;
	}
};

#endif //__C_FFT1D2R_TEMPL_H__
