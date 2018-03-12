#pragma once
#ifndef __H_C_AUD_GENERATOR__
#define __H_C_AUD_GENERATOR__

#include "WINDOWS\\c_audio_out.h"
#include "..\\..\\COMMON\\c_thread.h"
#include "..\\..\\DSP\\c_generator.h"

// audio generator

class c_aud_generator:public c_thread
{
private:	
	double * freq;				// saving value for thread call
	double * freqAmpl;
	double	 freqOffset;
	int		 freqNum; // freq param array size
	DWORD timeToGenerate_ms;	// saving value for thread call

	static void ChangingFreqToMax(c_audio_out * audio_out)
	{		
		WAVEFORMATEX CurrParam = *audio_out->GetParam();		
		int CurrFreq=CurrParam.nSamplesPerSec;
		CurrParam.nSamplesPerSec = 96000;
		if(audio_out->SetParam(&CurrParam)) return;
		CurrParam.nSamplesPerSec = 48000;
		if(audio_out->SetParam(&CurrParam)) return;
		CurrParam.nSamplesPerSec = 44100;
		if(audio_out->SetParam(&CurrParam)) return;
		CurrParam.nSamplesPerSec = 22050;
		if(audio_out->SetParam(&CurrParam)) return;
		CurrParam.nSamplesPerSec = 11025;
		if(audio_out->SetParam(&CurrParam)) return;
		CurrParam.nSamplesPerSec = CurrFreq;
		audio_out->SetParam(&CurrParam);
	}	


	static BOOL Generate_internal(	double * Freq, 
									double * FreqAmpl,
									double FreqOffset,
									int NumFreq, // frequency param array size
									DWORD TimeToGenerate_ms,
									c_aud_generator * generatorClass=NULL // for thread parameter, not in use for common
						)
	{
		BOOL res=TRUE;

		c_audio_out audio_out;		
		ChangingFreqToMax(&audio_out);				

		c_generator<short> generator;
		generator.SetParamAmpl(Freq, audio_out.GetParam()->nSamplesPerSec);		
		
		DWORD buffLen = (DWORD)(audio_out.GetParam()->nSamplesPerSec/4); // update buffer 4 times per second		

		short * Buffer = new short[buffLen];
		if(Buffer==NULL) return FALSE;

		audio_out.Start();
		DWORD BeginTime=GetTickCount();		
		while(res)		
		{
			if(TimeToGenerate_ms!=INFINITE)
				if((GetTickCount()-BeginTime)>=TimeToGenerate_ms) break;

			if(generatorClass!=NULL)
			{
				if(generatorClass->Stopped) break;
				while(generatorClass->Paused)Sleep(10);
			}

			generator.GetVals(Buffer,buffLen);	
			
			if(!audio_out.WriteData((BYTE *)Buffer, buffLen*sizeof(short)))
			{
				res=FALSE;
				break;
			}
		}
 		audio_out.Stop(TRUE);
		delete Buffer;
		return res;
	}
public:
	volatile BOOL Paused;

	c_aud_generator()
	{
		freq=0;
		timeToGenerate_ms=0;
	};	
	~c_aud_generator(){};


	void Execute(void)
	{
		Generate_internal(freq, freqAmpl, freqOffset, freqNum, timeToGenerate_ms, this);		
	};

	BOOL Start(double Freq,DWORD TimeToGenerate_ms=INFINITE)
	{
		return Start(&Freq,NULL,0,1,TimeToGenerate_ms);
	}

	BOOL Start(double * Freq, double * FreqAmpl, double FreqOffset, int FreqNum, DWORD TimeToGenerate_ms=INFINITE)
	{		
		freq=Freq;
		freqAmpl = FreqAmpl;
		freqOffset = FreqOffset;
		freqNum = FreqNum;
		timeToGenerate_ms=TimeToGenerate_ms;

		Paused=FALSE;
		c_thread::Start();
		return TRUE;
	};

	static BOOL Generate(double Freq, DWORD TimeToGenerate_ms)
	{
		return Generate(&Freq, NULL,0,1,TimeToGenerate_ms);
	}	

	static BOOL Generate(double * Freq, double * FreqAmpl, double FreqOffset, int FreqNum, DWORD TimeToGenerate_ms)
	{		
		return Generate_internal(Freq, FreqAmpl, FreqOffset, FreqNum, TimeToGenerate_ms);
	}	
};

#endif //__H_C_AUD_GENERATOR__