#ifndef __H_C_AUDIO_IN__
#define __H_C_AUDIO_IN__

#ifdef WIN32

#pragma comment( lib, "Winmm.lib" )

#include "c_wave_param.h"
#include "..\\..\\..\\COMMON\\c_FIFO.h"
#include "..\\..\\..\\COMMON\\Compatability.h"



/*
 how to work, at first setparam
 at second setdevice, if device support param, so it will be opened, otherwise error
*/


#define ERR_C_AUDIO_NO_ERROR					0
#define ERR_C_AUDIO_IN_NOT_SUPPORTED_FORMAT		-1
#define ERR_C_AUDIO_IN_GET_DEV_CAPS				-2
#define ERR_C_AUDIO_IN_MEM_ALLOC				-3
#define ERR_C_AUDIO_IN_PREP_HEADER				-4
#define ERR_C_AUDIO_IN_UNPREP_HEADER			-5
#define ERR_C_AUDIO_IN_ADD_BUF					-6
#define ERR_C_AUDIO_IN_STOP_PAUSE				-7
#define ERR_C_AUDIO_IN_START_PAUSE				-8
#define ERR_C_AUDIO_IN_CREATE_EVENT				-9
#define ERR_C_AUDIO_IN_BUFF_OVERRUN				-10
#define ERR_C_AUDIO_IN_RESET					-11

class c_audio_in	
{	
private:	

	HWAVEIN hDevice;	 // audio device handle
	BOOL DeviceStopping; // indicate for callback function, that it must not fill headers to device

	DWORD numAudioBuffers;
	DWORD audioBufferSizeBytes;

	c_wave_param WaveParam;		

	BYTE * AudioArray;				// data buffer for read from device
	WAVEHDR * AudioHDR;				// array of headers to queue input data

	int LastError;
	MMRESULT LastMMError;	

	HANDLE NewAudioDataReady; // auto reset event for data presence	
	DWORD CurrAudioBufferReadIndex;
	DWORD CurrAuioBufferRemainDataForRead; // how much data remaining in audiobuffer from prrevious read	

	// we must remember, that windows already have SetLastError in API !!
	void SetLastError(const int SetError, const int SetMMError=MMSYSERR_NOERROR)
    {	
        // if mistake occure, we must not rewrite it
        if((LastError!=ERR_C_AUDIO_NO_ERROR||LastMMError!=MMSYSERR_NOERROR)
			&&(SetError!=ERR_C_AUDIO_NO_ERROR))  // if this not clearing errors
			return;

        LastError=SetError;
		LastMMError=SetMMError;

		if(SetError==ERR_C_AUDIO_NO_ERROR) LastMMError=MMSYSERR_NOERROR;         
    }

	BOOL __fastcall PutHeaderToAudioDeviceQueue(WAVEHDR * CurrHdr);

public:	

	c_audio_in(	const DWORD NumAudioBuffers=20,  // 5 secs
				const DWORD AudioBufferSizeBytes=1024*10 // 4 ~ times per second
				);	
	~c_audio_in(void);	

	static	DWORD	GetDevsCount();	
	static	BOOL	GetDevNameByDevID(const UINT_PTR uDeviceID, TCHAR * Name,const DWORD NameMaxLength);
	static	DWORD	GetDevIDByName(const TCHAR * Name);
	static	BOOL	GetDevCaps(const UINT_PTR uDeviceID, LPWAVEINCAPS pwic,const UINT cbwic);	
	
	BOOL	DeviceSupportingFormat(LPWAVEFORMATEX CurrFormat){return DeviceSupportingFormat(WaveParam.GetDeviceID(), CurrFormat);};
	BOOL	DeviceSupportingFormat(const UINT_PTR uDeviceID, LPWAVEFORMATEX CurrFormat);
	BOOL	DeviceSupportingFormat(const UINT_PTR uDeviceID,const int Freq,const WORD NumBits,const WORD NumChannels);

	BOOL SetParam(	const UINT_PTR uDeviceID=WAVE_MAPPER,
					const int Freq=44100,
					const WORD NumBits=16,
					const WORD NumChannels=1					
					);
	BOOL SetParam(const UINT_PTR uDeviceID,const LPWAVEFORMATEX pwfx);	
	BOOL SetParam(const LPWAVEFORMATEX pwfx){SetParam(WaveParam.GetDeviceID(),pwfx);};	
	LPWAVEFORMATEX GetParam(){return WaveParam.GetWAVEFORMATEX();};
	DWORD GetDevID(){return WaveParam.GetDeviceID();};

	BOOL Start();
	BOOL Pause();
	BOOL Paused; // read only !
	BOOL Resume();
	BOOL Reset();
	BOOL Stop();	

	int GetLastError(){return LastError;}; // return last error
	MMRESULT GetLastMMError(){return LastMMError;}; // return last multimedia error	

	// for internal use only !
	void __fastcall ProcessHeader(WAVEHDR * CurrHdr);

	// for data read
	BOOL  __fastcall ReadData(BYTE * Data, DWORD DataLen);	

	BOOL RecordToWav(TCHAR * Filename,int RecordTime_ms);	
};

#endif //WIN32
#endif //__H_C_AUDIO_IN__
