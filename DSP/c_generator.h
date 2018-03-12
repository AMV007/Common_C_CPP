#ifndef __C_GENERATOR_H__
#define __C_GENERATOR_H__

#define _USE_MATH_DEFINES
#include <math.h>

template <typename DATA_TYPE=double>
class c_generator
{
private:
	
	double * angle;
	double * angleAdd;
	double * ampl;
	DATA_TYPE offset;	

	int numFreq; // frequency number - how much freq generate, this is also array size angle=angleAdd=ampl	

	void DeleteArrays()
	{
		if(angle!=NULL) delete angle;
		if(angleAdd!=NULL) delete angleAdd;
		if(ampl!=NULL) delete ampl;		
	}

	__inline BOOL PrepareArrays(const DWORD NumFreq)
	{
		if(NumFreq<1) return FALSE;
		numFreq=NumFreq;

		DeleteArrays(); // delete arrays if them where allocated		

		angle=new double[NumFreq];
		angleAdd=new double[NumFreq];
		ampl = new double[NumFreq];

		if(angle==NULL||angleAdd==NULL||ampl==NULL) return FALSE; // error memory allocation

		return TRUE;
	}	
	
public:
	c_generator(double Freq=0,  // frequency to generate
				double FreqDevice=0, // device frequency				
				DATA_TYPE Offset=0,				
				double Ampl=0,
				double initPhazeRad=0
				)
	{
		numFreq=0;
		offset=0;
		angle=angleAdd=ampl=NULL;		

		SetParamAmpl(Freq, FreqDevice,Offset,Ampl,initPhazeRad);
	};

	// for single freq call
	BOOL SetParamRange(	const double Freq,		// frequency to generate
					const double FreqDevice=44100,// device frequency
					const DATA_TYPE MaxLimit=32767,const DATA_TYPE MinLimit=-32768, // sygnal param	
					const double initPhazeRad=0)
	{	
		
		return SetParam(&Freq, FreqDevice,1, &MaxLimit, &MinLimit, &initPhazeRad);
	};

	// for multiple freq call, ampl through maxlimit - minLimit
	BOOL SetParamRange(	const double * Freq,		// frequency array to generate										
						const double FreqDevice,	// device frequency
						const DWORD NumFreq=1,		// frequency param array size
						const DATA_TYPE *MaxLimit=NULL,const DATA_TYPE *MinLimit=NULL, // sygnal param from -32768 to 32767 for 16 bit
						const double *initPhazeRad=NULL					
						)
	{
		if(FreqDevice==0||Freq==NULL
			||!PrepareArrays(NumFreq) // also in function numFreq=NumFreq
			) return FALSE;	

		offset=0;
		for(int i=0;i<numFreq;i++)
		{
			if((FreqDevice/Freq[i])<2.0) return FALSE;
			angleAdd[i]=(2*M_PI*Freq[i])/FreqDevice;			
			
			if(MaxLimit==NULL||MinLimit==NULL) ampl[i]=32767;
			else ampl[i] = (MaxLimit[i]-MinLimit[i])/2.0;

			if(MaxLimit!=NULL) offset+=(DATA_TYPE)(MaxLimit[i]-ampl[i]);

			if(initPhazeRad==NULL) angle[i]=0;
			else angle[i]=initPhazeRad[i];	
		}

		return TRUE;
	};	

	// for multiple freq call, ampl through ampl and offset
	BOOL SetParamAmpl(	const double Freq,		// frequency array to generate										
					const double FreqDevice,	// device frequency
					const DATA_TYPE Offset=0, // offset for 16 bit from -32768 to 32767
					const double Ampl=NULL,	// ampl, for 16 bit from 0 to 32767					
					const double initPhazeRad=NULL // initial phase					
				  )
	{
		return SetParamAmpl(&Freq, FreqDevice,1, Offset, &Ampl, &initPhazeRad);
	}

	// for multiple freq call, ampl through ampl and offset
	BOOL SetParamAmpl(	const double * Freq,		// frequency array to generate										
						const double FreqDevice,	// device frequency
						const DWORD NumFreq=1,		// frequency param array size
						const DATA_TYPE Offset=0, // offset for 16 bit from -32768 to 32767
						const double * Ampl=NULL,	// ampl, for 16 bit from 0 to 32767						
						const double *initPhazeRad=NULL // initial phase					
					)
	{
		if(FreqDevice==0||Freq==NULL
			||!PrepareArrays(NumFreq) // also in function numFreq=NumFreq
			) return FALSE;	

		if(NumFreq<1) return FALSE;
		numFreq=NumFreq;

		offset=Offset;
		for(int i=0;i<numFreq;i++)
		{
			if((FreqDevice/Freq[i])<2.0) return FALSE;
			angleAdd[i]=(2*M_PI*Freq[i])/FreqDevice;			
			
			if(Ampl==NULL) ampl[i]=32767;
			else ampl[i] = Ampl[i];			

			if(initPhazeRad==NULL) angle[i]=0;
			else angle[i]=initPhazeRad[i];	
		}

		return TRUE;
	};

	// get next value
	DATA_TYPE __fastcall GetNextValue(void)
	{
		DATA_TYPE res=0;
		for(int i=0;i<numFreq;i++)
		{
			res+=(DATA_TYPE)(sin(angle[i])*ampl[i]);		
			angle[i]+=angleAdd[i];	
		}				
		return res+offset;
	};

	// get array of values
	void GetVals(DATA_TYPE * Data, DWORD DataLen)
	{
		for(DWORD i=0;i<DataLen;i++) Data[i] = GetNextValue();		
	};

	// resetting angle for all frequencies
	void ResetAngle(double InitPhaze=0)
	{
		for(int i=0;i<numFreq;i++)
		{			
			angle[i]=InitPhaze;	
		}		
	}
	
	~c_generator()
	{
		DeleteArrays();
	};
};

#endif //__C_GENERATOR_H__