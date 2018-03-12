#include "StdAfx.h"
#include "c_audio_out.h"
#include "..\\c_wave.h"

c_audio_out::c_audio_out(const DWORD NumAudioBuffers,
						 const DWORD AudioBufferSizeBytes)
{	
	DeviceStopping=TRUE;
	hDevice=NULL;
	LastError=0;
	LastMMError=MMSYSERR_NOERROR;
	Paused = FALSE;
	CurrAudioBufferWriteIndex=0;

	numAudioBuffers=NumAudioBuffers;
	audioBufferSizeBytes=AudioBufferSizeBytes;

	AudioDataBufferDone = CreateEvent(NULL,FALSE,FALSE,NULL);
	if(AudioDataBufferDone==NULL) c_audio_out::SetLastError(ERR_C_AUDIO_OUT_CREATE_EVENT);	
	
	AudioHDR = new WAVEHDR[NumAudioBuffers];
	AudioArray = new BYTE[audioBufferSizeBytes*numAudioBuffers];
	if(AudioHDR==NULL||AudioArray==NULL)c_audio_out::SetLastError(ERR_C_AUDIO_OUT_MEM_ALLOC);	
}

c_audio_out::~c_audio_out(void)
{
	Stop();	

	if(AudioDataBufferDone!=NULL) CloseHandle(AudioDataBufferDone);	
	if(AudioArray!=NULL)	delete AudioArray;			// data buffer for read from device
	if(AudioHDR!=NULL)	delete AudioHDR;				// array of headers to queue input data
}

DWORD c_audio_out::GetDevsCount()
{	
	return waveOutGetNumDevs();
}

BOOL c_audio_out::GetDevCaps(const UINT_PTR uDeviceID, LPWAVEOUTCAPS pwic,const UINT cbwic)
{
	MMRESULT MMError=waveOutGetDevCaps(uDeviceID,pwic,cbwic);
	if(MMError==MMSYSERR_NOERROR) return TRUE;
	//c_audio_out::SetLastError(ERR_C_AUDIO_OUT_GET_DEV_CAPS,MMError);		
	return FALSE;
}

BOOL c_audio_out::GetDevNameByDevID(const UINT_PTR uDeviceID, TCHAR * Name,const DWORD NameMaxLength)
{
	WAVEOUTCAPS DevCaps;
	memset(&DevCaps,0,sizeof(WAVEOUTCAPS));
	if(!GetDevCaps(uDeviceID, &DevCaps, sizeof(DevCaps))) FALSE;
	my_wa_strcpy_s(Name,NameMaxLength, DevCaps.szPname);	
	return TRUE;
}

DWORD c_audio_out::GetDevIDByName(const TCHAR * Name)
{
	WAVEOUTCAPS DevCaps;
	DWORD DevNum=GetDevsCount();
	for(DWORD i=0;i<DevNum;i++)
	{
		memset(&DevCaps,0,sizeof(WAVEOUTCAPS));
		if(!GetDevCaps(i, &DevCaps, sizeof(DevCaps))) continue;
		if(!my_wa_strcmp(DevCaps.szPname,Name)) return i;
	}
	return (DWORD)-1;
}

BOOL c_audio_out::DeviceSupportingFormat(const UINT_PTR uDeviceID,const int Freq,const WORD NumBits,const WORD NumChannels)
{
	c_wave_param TempParam(uDeviceID, Freq, NumBits, NumChannels);		
	return DeviceSupportingFormat(uDeviceID, TempParam.GetWAVEFORMATEX());
}

BOOL c_audio_out::DeviceSupportingFormat(const UINT_PTR uDeviceID, LPWAVEFORMATEX CurrFormat)
{	
	MMRESULT MMError=waveOutOpen(NULL, 
						uDeviceID, CurrFormat, 0, 0,
						WAVE_FORMAT_QUERY); 
	return (MMError==MMSYSERR_NOERROR);
}

BOOL c_audio_out::SetParam(const UINT_PTR uDeviceID,const int Freq,const WORD NumBits,const WORD NumChannels)
{
	if(!DeviceSupportingFormat(uDeviceID,Freq, NumBits, NumChannels))
	{
		c_audio_out::SetLastError(ERR_C_AUDIO_OUT_NOT_SUPPORTED_FORMAT);
		return FALSE;
	}

	return WaveParam.SetParam(uDeviceID, Freq, NumBits,NumChannels);		
}

BOOL c_audio_out::SetParam(const UINT_PTR uDeviceID,const LPWAVEFORMATEX pwfx)
{
	if(!DeviceSupportingFormat(uDeviceID, pwfx))
	{
		c_audio_out::SetLastError(ERR_C_AUDIO_OUT_NOT_SUPPORTED_FORMAT);
		return FALSE;
	}
	return WaveParam.SetParam(uDeviceID, pwfx);		
}

BOOL c_audio_out::Pause()
{	
	MMRESULT MMError=waveOutPause(hDevice);
	if(MMError!=MMSYSERR_NOERROR)
	{
		c_audio_out::SetLastError(ERR_C_AUDIO_OUT_STOP_PAUSE, MMError);		
		return FALSE;
	}	
	Paused = TRUE;
	return TRUE;
}

BOOL c_audio_out::Resume()
{
	MMRESULT MMError=waveOutRestart(hDevice);
	if(MMError!=MMSYSERR_NOERROR)
	{
		c_audio_out::SetLastError(ERR_C_AUDIO_OUT_START_PAUSE, MMError);		
		return FALSE;
	}
	Paused = FALSE;
	return TRUE;
}

BOOL c_audio_out::Reset()
{	
	MMRESULT MMError=waveOutReset(hDevice);
	if(MMError!=MMSYSERR_NOERROR)
	{
		c_audio_out::SetLastError(ERR_C_AUDIO_OUT_RESET, MMError);		
		return FALSE;
	}	
	Paused = TRUE;
	return TRUE;
}

void __fastcall c_audio_out::ProcessHeader(WAVEHDR * CurrHdr)
{
	SetEvent(AudioDataBufferDone);
	if(DeviceStopping) return;
	
	DWORD NumDoneBuffers=0;
	for(DWORD i=0;i<numAudioBuffers;i++) if(AudioHDR[i].dwFlags&WHDR_DONE)NumDoneBuffers++;

	if(NumDoneBuffers>=numAudioBuffers)
	{ // all buffers are done and programm not fast enough to read !!!
		c_audio_out::SetLastError(ERR_C_AUDIO_OUT_BUFF_OVERRUN);		
	}	
}

void CALLBACK waveOutProc( HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	switch(uMsg)
	{
	case WOM_CLOSE:
		break;

	case WOM_DONE:		
		((c_audio_out * ) dwInstance)->ProcessHeader((WAVEHDR *)dwParam1);					
		break;

	case WOM_OPEN:
		break;

	default:
		break;
	}
}

BOOL c_audio_out::Start(BOOL PauseOnStart)
{
	Stop();
	Paused = FALSE;	
	CurrAudioBufferWriteIndex=0;

	BOOL res=TRUE;	
	MMRESULT MMError=waveOutOpen(&hDevice, 
		WaveParam.GetDeviceID(), WaveParam.GetWAVEFORMATEX(), 
		(DWORD_PTR)waveOutProc, (DWORD_PTR)this,
		CALLBACK_FUNCTION | WAVE_FORMAT_DIRECT);  

	if(MMError!=MMSYSERR_NOERROR)
	{
		c_audio_out::SetLastError(ERR_C_AUDIO_OUT_GET_DEV_CAPS, MMError);		
		return FALSE;
	}	
	else
	{
		Reset();
		if(PauseOnStart)Pause();
	}
	
	for(DWORD HeaderIndex=0;HeaderIndex<numAudioBuffers&&res;HeaderIndex++)
	{
		memset(&AudioHDR[HeaderIndex],0,sizeof(WAVEHDR)); // necessary !!!
		AudioHDR[HeaderIndex].lpData = (LPSTR)&AudioArray[HeaderIndex*audioBufferSizeBytes];		
		AudioHDR[HeaderIndex].dwBufferLength = audioBufferSizeBytes;	

		MMError=waveOutPrepareHeader(hDevice, &AudioHDR[HeaderIndex],sizeof(WAVEHDR));
		if(MMError!=MMSYSERR_NOERROR)
		{
			c_audio_out::SetLastError(ERR_C_AUDIO_OUT_PREP_HEADER, MMError);
			res=FALSE;	
			break;
		}		
		AudioHDR[HeaderIndex].dwFlags|=WHDR_DONE; // for write data recognize empty buffer !						
	}	

	if(res)
	{
		DeviceStopping=FALSE;				
	}
	else
	{
		for(DWORD HeaderIndex=0;HeaderIndex<numAudioBuffers;HeaderIndex++)
		{
			waveOutUnprepareHeader(hDevice, &AudioHDR[HeaderIndex],sizeof(WAVEHDR));					
		}
		waveOutClose(hDevice);
		hDevice=NULL;
	}

	return res;
}

BOOL c_audio_out::FlushAudioDevice()
{	// we need to flush audio buffers in device
	// need this function for flush audio aoutput after playing
	// because may be some audio scraps in sudio output in this moment 
	// if reset apper with buffers in stock

	BOOL res=TRUE;

	int BytesPerScrap = WaveParam.waveFormat.nChannels*(WaveParam.waveFormat.wBitsPerSample/8);
	BYTE *TempBuffer = new BYTE[BytesPerScrap];
	if(TempBuffer==NULL) return FALSE;

	memset(TempBuffer,0,BytesPerScrap);
	
	WAVEHDR TempBufferHDR;		
	memset(&TempBufferHDR,0,sizeof(WAVEHDR));
	TempBufferHDR.lpData = (LPSTR)TempBuffer;
	TempBufferHDR.dwBufferLength=BytesPerScrap;
	
	MMRESULT MMError=waveOutPrepareHeader(hDevice, &TempBufferHDR,sizeof(WAVEHDR));
	if(MMError!=MMSYSERR_NOERROR)
	{
		c_audio_out::SetLastError(ERR_C_AUDIO_OUT_PREP_HEADER, MMError);
		res=FALSE;	
	}	
	else
	{
		PutHeaderToAudioDeviceQueue(&TempBufferHDR);
		while(!(TempBufferHDR.dwFlags&WHDR_DONE))
		{			
			if(WaitForSingleObject(AudioDataBufferDone,500)==WAIT_ABANDONED) break;					
		}	

		MMError=waveOutUnprepareHeader(hDevice, &TempBufferHDR,sizeof(WAVEHDR));
		if(MMError!=MMSYSERR_NOERROR)
		{
			c_audio_out::SetLastError(ERR_C_AUDIO_OUT_UNPREP_HEADER, MMError);			
			res=FALSE;
		}	
	}

	delete TempBuffer;
	return res;
}

BOOL c_audio_out::Stop(BOOL NotWaitBuffers)
{
	if(hDevice==NULL||DeviceStopping) return TRUE;
	BOOL res=TRUE;

	DeviceStopping=TRUE; // indicate for callback function, that it must stop				
	if(NotWaitBuffers) Reset();

	// waiting all headers are done
	for(DWORD HeaderIndex=0;HeaderIndex<numAudioBuffers;)
	{
		if(!(AudioHDR[HeaderIndex].dwFlags&WHDR_DONE))
		{			
			if(WaitForSingleObject(AudioDataBufferDone,500)==WAIT_ABANDONED) break;			
			HeaderIndex=0; continue;
		}		
		HeaderIndex++;
	}

	if(!NotWaitBuffers)Reset();
	else FlushAudioDevice(); // after not waitted reset
	
	for(DWORD HeaderIndex=0;HeaderIndex<numAudioBuffers;HeaderIndex++)
	{
		MMRESULT MMError=waveOutUnprepareHeader(hDevice, &AudioHDR[HeaderIndex],sizeof(WAVEHDR));
		if(MMError!=MMSYSERR_NOERROR)
		{
			c_audio_out::SetLastError(ERR_C_AUDIO_OUT_UNPREP_HEADER, MMError);			
			res=FALSE;
		}		
	}

	waveOutClose(hDevice);
	hDevice=NULL;
	Paused = FALSE;

	ResetEvent(AudioDataBufferDone);	
	return res;
}

BOOL __fastcall c_audio_out::PutHeaderToAudioDeviceQueue(WAVEHDR * CurrHdr)
{	
	MMRESULT MMError=waveOutWrite(hDevice, CurrHdr,sizeof(WAVEHDR));	
	if(MMError!=MMSYSERR_NOERROR)
	{ // error add buffer into queue
		c_audio_out::SetLastError(ERR_C_AUDIO_OUT_ADD_BUF, MMError);
		DeviceStopping=TRUE; // indicate for callback function, that it must stop
		Reset();		
		return FALSE;
	}	
	return TRUE;
}


BOOL __fastcall c_audio_out::WriteData(BYTE * Data, DWORD DataLen, BOOL AutoResumeDevice)
{	
	while(DataLen&&!DeviceStopping)
	{
		while(!(AudioHDR[CurrAudioBufferWriteIndex].dwFlags&WHDR_DONE)&&!DeviceStopping)
		{
			if(AutoResumeDevice&&Paused&&hDevice!=NULL) Resume();	
			if(WaitForSingleObject(AudioDataBufferDone,500)==WAIT_ABANDONED)
				return FALSE;
		}
		DWORD NumDataToWrite = min(DataLen, audioBufferSizeBytes);
		AudioHDR[CurrAudioBufferWriteIndex].dwBufferLength = NumDataToWrite;
		memcpy((BYTE *)AudioHDR[CurrAudioBufferWriteIndex].lpData, Data, NumDataToWrite);
		if(!PutHeaderToAudioDeviceQueue(&AudioHDR[CurrAudioBufferWriteIndex])) return FALSE;
		Data+=NumDataToWrite;
		DataLen-=NumDataToWrite;
		CurrAudioBufferWriteIndex=(CurrAudioBufferWriteIndex+1)%numAudioBuffers;
	}	
	return (DataLen==0);
}


BOOL c_audio_out::PlayWav(TCHAR * Filename)
{		
	c_wave CurrWave;		
	return CurrWave.Play(Filename, WaveParam.GetDeviceID());	
}
