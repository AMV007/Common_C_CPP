#ifndef __H_C_WAVE_PARAM__
#define __H_C_WAVE_PARAM__

#include "../../../COMMON/Compatability.h"

#ifdef WIN32
#include <windows.h>
#include <Mmsystem.h>
#else
	#include "string.h"
	#include "../../../COMMON/std_types.h"

	typedef struct {
	  WORD  wFormatTag;
	  WORD  nChannels;
	  DWORD nSamplesPerSec;
	  DWORD nAvgBytesPerSec;
	  WORD  nBlockAlign;
	  WORD  wBitsPerSample;
	  WORD  cbSize;
	} WAVEFORMATEX, *LPWAVEFORMATEX;

	#define WAVE_FORMAT_1M08            1
	#define WAVE_FORMAT_1S08            2
	#define WAVE_FORMAT_1M16            4
	#define WAVE_FORMAT_1S16            8
	#define WAVE_FORMAT_2M08            16
	#define WAVE_FORMAT_2S08            32
	#define WAVE_FORMAT_2M16            64
	#define WAVE_FORMAT_2S16            128
	#define WAVE_FORMAT_4M08            256
	#define WAVE_FORMAT_4S08            512
	#define WAVE_FORMAT_4M16            1024
	#define WAVE_FORMAT_4S16            2048
	#define WAVE_FORMAT_48M08           4096
	#define WAVE_FORMAT_48S08           8192
	#define WAVE_FORMAT_48M16           16384
	#define WAVE_FORMAT_48S16           32768
	#define WAVE_FORMAT_96M08           65536
	#define WAVE_FORMAT_96S08           131072
	#define WAVE_FORMAT_96M16           262144
	#define WAVE_FORMAT_96S16           524288

	#ifndef WAVE_FORMAT_PCM
	#define WAVE_FORMAT_PCM             1
	#endif


	#define WAVE_MAPPER ((UINT)-1)

#endif







class c_wave_param
{	
private:
	
	UINT_PTR devID;				// audio device Number

public:	

	WAVEFORMATEX waveFormat;	// device WAV format, will be good to not access it from outside

	c_wave_param(void)
	{
		SetParam(); // default device i think		
	};
	c_wave_param(UINT_PTR uDeviceID, int Freq, WORD NumBits, WORD NumChannels)
	{		
		SetParam(uDeviceID, Freq, NumBits, NumChannels);
	};
	virtual ~c_wave_param(void){};
	
	LPWAVEFORMATEX GetWAVEFORMATEX() {return &waveFormat;};
	UINT_PTR GetDeviceID(){return devID;};

	BOOL SetParam(	UINT_PTR uDeviceID=WAVE_MAPPER, 
					int Freq=44100, 
					WORD NumBits=16, 
					WORD NumChannels=1)
	{
		if(NumChannels<1||Freq<1||NumBits<8) return FALSE;

		devID=uDeviceID;

		waveFormat.wFormatTag = WAVE_FORMAT_PCM;

		waveFormat.nChannels=NumChannels;
		waveFormat.nSamplesPerSec=Freq;
		waveFormat.wBitsPerSample=NumBits;

		waveFormat.nBlockAlign = (short)(NumChannels * NumBits / 8);
		waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;

		waveFormat.cbSize = 0; //!!!!!

		return TRUE;
	};

	BOOL SetParam(UINT_PTR uDeviceID, LPWAVEFORMATEX pwfx)
	{
		if(pwfx==NULL) return FALSE;
		devID=uDeviceID;
		memcpy(&waveFormat,pwfx, sizeof(WAVEFORMATEX));		
		return TRUE;
	};	

	static DWORD GetWaveFormatFlags(int Freq, int NumBits, int NumChannels)
	{
		switch (Freq)
		{
		case 11025:
			if(NumChannels==1)
			{
				if (NumBits==8)			return WAVE_FORMAT_1M08;
				else if(NumBits==16)	return WAVE_FORMAT_1M16;
			}
			else if(NumChannels==2)
			{
				if (NumBits==8)			return WAVE_FORMAT_1S08;
				else if(NumBits==16)	return WAVE_FORMAT_1S16;
			}
			break;
		case 22050:
			if(NumChannels==1)
			{
				if (NumBits==8)			return WAVE_FORMAT_2M08;
				else if(NumBits==16)	return WAVE_FORMAT_2M16;
			}
			else if(NumChannels==2)
			{
				if (NumBits==8)			return WAVE_FORMAT_2S08;
				else if(NumBits==16)	return WAVE_FORMAT_2S16;
			}
			break;
		case 44100:
			if(NumChannels==1)
			{
				if (NumBits==8)			return WAVE_FORMAT_4M08;
				else if(NumBits==16)	return WAVE_FORMAT_4M16;
			}
			else if(NumChannels==2)
			{
				if (NumBits==8)			return WAVE_FORMAT_4S08;
				else if(NumBits==16)	return WAVE_FORMAT_4S16;
			}
			break;
		case 96000:
			if(NumChannels==1)
			{
				if (NumBits==8)			return WAVE_FORMAT_96M08;
				else if(NumBits==16)	return WAVE_FORMAT_96M16;
			}
			else if(NumChannels==2)
			{
				if (NumBits==8)			return WAVE_FORMAT_96S08;
				else if(NumBits==16)	return WAVE_FORMAT_96S16;
			}
			break;
		}
		return 0;
	}
};

#endif //__H_C_WAVE_PARAM__
