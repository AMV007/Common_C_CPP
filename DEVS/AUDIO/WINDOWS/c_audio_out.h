#ifndef __H_C_AUDIO_OUT__
#define __H_C_AUDIO_OUT__

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
#define ERR_C_AUDIO_OUT_NOT_SUPPORTED_FORMAT	-1
#define ERR_C_AUDIO_OUT_GET_DEV_CAPS			-2
#define ERR_C_AUDIO_OUT_MEM_ALLOC				-3
#define ERR_C_AUDIO_OUT_PREP_HEADER				-4
#define ERR_C_AUDIO_OUT_UNPREP_HEADER			-5
#define ERR_C_AUDIO_OUT_ADD_BUF					-6
#define ERR_C_AUDIO_OUT_STOP_PAUSE				-7
#define ERR_C_AUDIO_OUT_START_PAUSE				-8
#define ERR_C_AUDIO_OUT_CREATE_EVENT			-9
#define ERR_C_AUDIO_OUT_BUFF_OVERRUN			-10 // all buffers are done
#define ERR_C_AUDIO_OUT_READ_FIFO				-11
#define ERR_C_AUDIO_OUT_ADD_BUF_WRITE			-12
#define ERR_C_AUDIO_OUT_ADD_BUF_START			-13
#define ERR_C_AUDIO_OUT_RESET					-14


class c_audio_out	
{	
private:	

	HWAVEOUT hDevice;	 // audio device handle
	BOOL DeviceStopping; // indicate for callback function, that it must not fill headers to device

	DWORD numAudioBuffers;
	DWORD audioBufferSizeBytes;

	c_wave_param WaveParam;		

	BYTE * AudioArray;				// data buffer for read from device
	WAVEHDR * AudioHDR;				// array of headers to queue input data

	int LastError;
	MMRESULT LastMMError;	

	HANDLE AudioDataBufferDone; // space in FIFO exist for new audio data, auto reset		

	DWORD CurrAudioBufferWriteIndex; 	

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
	// need this function for flushing audio device output, because after reset, there are data in audio device
	BOOL FlushAudioDevice();

public:	
	
	c_audio_out(	const DWORD NumAudioBuffers=20, // 5 secs
					const DWORD AudioBufferSizeBytes=1024*100 // 4 ~ times per second
				);	
	~c_audio_out(void);	

	static	DWORD	GetDevsCount();	
	static	BOOL	GetDevNameByDevID(const UINT_PTR uDeviceID, TCHAR * Name,const DWORD NameMaxLength);
	static	DWORD	GetDevIDByName(const TCHAR * Name);
	static	BOOL	GetDevCaps(const UINT_PTR uDeviceID, LPWAVEOUTCAPS pwic,const UINT cbwic);
	
	BOOL	DeviceSupportingFormat(LPWAVEFORMATEX CurrFormat){return DeviceSupportingFormat(WaveParam.GetDeviceID(), CurrFormat);};
	BOOL	DeviceSupportingFormat(const UINT_PTR uDeviceID, LPWAVEFORMATEX CurrFormat);
	BOOL	DeviceSupportingFormat(const UINT_PTR uDeviceID,const int Freq,const WORD NumBits,const WORD NumChannels);

	BOOL SetParam(	const UINT_PTR uDeviceID=WAVE_MAPPER,
					const int Freq=44100,
					const WORD NumBits=16,
					const WORD NumChannels=1					
					);
	BOOL SetParam(const UINT_PTR uDeviceID,const LPWAVEFORMATEX pwfx);	
	BOOL SetParam(const LPWAVEFORMATEX pwfx){return SetParam(WaveParam.GetDeviceID(),pwfx);};	
	LPWAVEFORMATEX GetParam(){return WaveParam.GetWAVEFORMATEX();};
	DWORD GetDevID(){return WaveParam.GetDeviceID();};

	// start device in paused mode, to start playing, fill FIFO, or buffers and call Resume
	BOOL Start(BOOL PauseOnStart=TRUE);
	BOOL Pause();
	BOOL Paused; // read only
	BOOL Resume();
	BOOL Reset();
	BOOL Stop(BOOL NotWaitBuffers=FALSE);	

	int GetLastError(){return LastError;}; // return last error
	MMRESULT GetLastMMError(){return LastMMError;}; // return last multimedia error	

	// for internal use only !
	void __fastcall ProcessHeader(WAVEHDR * CurrHdr);

	// writing data in audioBuffers, 
	// AutoResumeDevice - if audiobuffers all full, start playback
	BOOL  __fastcall WriteData(BYTE * Data, DWORD DataLen, BOOL AutoResumeDevice=TRUE);	

	BOOL PlayWav(TCHAR * Filename);	
};
#endif //WIN32
#endif //__H_C_AUDIO_OUT__
