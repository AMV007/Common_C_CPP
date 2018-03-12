#include "StdAfx.h"
#include "c_audio_in.h"
#include "..\\c_wave.h"

c_audio_in::c_audio_in( const DWORD NumAudioBuffers,
					   const DWORD AudioBufferSizeBytes)
{	
	DeviceStopping=TRUE;
	hDevice=NULL;
	LastError=0;
	LastMMError=MMSYSERR_NOERROR;
	Paused = FALSE;
	CurrAudioBufferReadIndex=0;
	CurrAuioBufferRemainDataForRead=0; 

	numAudioBuffers=NumAudioBuffers;
	audioBufferSizeBytes=AudioBufferSizeBytes;

	NewAudioDataReady = CreateEvent(NULL,FALSE,FALSE,NULL);
	if(NewAudioDataReady==NULL) c_audio_in::SetLastError(ERR_C_AUDIO_IN_CREATE_EVENT);	
	
	AudioHDR = new WAVEHDR[NumAudioBuffers];
	AudioArray = new BYTE[AudioBufferSizeBytes*NumAudioBuffers];
	if(AudioHDR==NULL||AudioArray==NULL)c_audio_in::SetLastError(ERR_C_AUDIO_IN_MEM_ALLOC);	
}

c_audio_in::~c_audio_in(void)
{
	Stop();	

	if(NewAudioDataReady!=NULL) CloseHandle(NewAudioDataReady);	
	if(AudioArray!=NULL)	delete AudioArray;			// data buffer for read from device
	if(AudioHDR!=NULL)	delete AudioHDR;				// array of headers to queue input data
}

DWORD c_audio_in::GetDevsCount()
{		
	return waveInGetNumDevs();
}

BOOL c_audio_in::GetDevCaps(const UINT_PTR uDeviceID, LPWAVEINCAPS pwic,const UINT cbwic)
{
	MMRESULT MMError=waveInGetDevCaps(uDeviceID,pwic,cbwic);
	if(MMError==MMSYSERR_NOERROR) return TRUE;

	//c_audio_in::SetLastError(ERR_C_AUDIO_IN_GET_DEV_CAPS,MMError);		
	return FALSE;
}

BOOL c_audio_in::GetDevNameByDevID(const UINT_PTR uDeviceID, TCHAR * Name,const DWORD NameMaxLength)
{
	WAVEINCAPS DevCaps;
	memset(&DevCaps,0,sizeof(WAVEINCAPS));
	if(!GetDevCaps(uDeviceID, &DevCaps, sizeof(DevCaps))) FALSE;
	my_wa_strcpy_s(Name,NameMaxLength, DevCaps.szPname);	
	return TRUE;
}

DWORD c_audio_in::GetDevIDByName(const TCHAR * Name)
{
	WAVEINCAPS DevCaps;
	DWORD DevNum=GetDevsCount();
	for(DWORD i=0;i<DevNum;i++)
	{
		memset(&DevCaps,0,sizeof(WAVEINCAPS));
		if(!GetDevCaps(i, &DevCaps, sizeof(DevCaps))) continue;
		if(!my_wa_strcmp(DevCaps.szPname,Name)) return i;
	}
	return (DWORD)-1;
}


BOOL c_audio_in::DeviceSupportingFormat(const UINT_PTR uDeviceID,const int Freq,const WORD NumBits,const WORD NumChannels)
{
	c_wave_param TempParam(uDeviceID, Freq, NumBits, NumChannels);		
	return DeviceSupportingFormat(uDeviceID, TempParam.GetWAVEFORMATEX());
}

BOOL c_audio_in::DeviceSupportingFormat(const UINT_PTR uDeviceID, LPWAVEFORMATEX CurrFormat)
{	
	MMRESULT MMError=waveInOpen(NULL, 
						uDeviceID, CurrFormat, 0, 0,
						WAVE_FORMAT_QUERY); 
	return (MMError==MMSYSERR_NOERROR);
}



BOOL c_audio_in::SetParam(const UINT_PTR uDeviceID,const int Freq,const WORD NumBits,const WORD NumChannels)
{
	if(!DeviceSupportingFormat(uDeviceID,Freq, NumBits, NumChannels))
	{
		c_audio_in::SetLastError(ERR_C_AUDIO_IN_NOT_SUPPORTED_FORMAT);	
		return FALSE;
	}

	return WaveParam.SetParam(uDeviceID, Freq, NumBits,NumChannels);
}

BOOL c_audio_in::SetParam(const UINT_PTR uDeviceID,const LPWAVEFORMATEX pwfx)
{
	if(!DeviceSupportingFormat(uDeviceID, pwfx))
	{
		c_audio_in::SetLastError(ERR_C_AUDIO_IN_NOT_SUPPORTED_FORMAT);	
		return FALSE;
	}
	return WaveParam.SetParam(uDeviceID, pwfx);
}

BOOL c_audio_in::Pause()
{
	MMRESULT MMError=waveInStop(hDevice);
	if(MMError!=MMSYSERR_NOERROR)
	{
		c_audio_in::SetLastError(ERR_C_AUDIO_IN_STOP_PAUSE, MMError);		
		return FALSE;
	}
	Paused = TRUE;
	return TRUE;
}

BOOL c_audio_in::Resume()
{
	MMRESULT MMError=waveInStart(hDevice);
	if(MMError!=MMSYSERR_NOERROR)
	{
		c_audio_in::SetLastError(ERR_C_AUDIO_IN_START_PAUSE, MMError);		
		return FALSE;
	}
	Paused = FALSE;
	return TRUE;
}

BOOL c_audio_in::Reset()
{
	MMRESULT MMError=waveInReset(hDevice);
	if(MMError!=MMSYSERR_NOERROR)
	{
		c_audio_in::SetLastError(ERR_C_AUDIO_IN_RESET, MMError);		
		return FALSE;
	}
	Paused = FALSE;
	return TRUE;
}

void __fastcall c_audio_in::ProcessHeader(WAVEHDR * CurrHdr)
{
	SetEvent(NewAudioDataReady);
	if(DeviceStopping) return;	

	DWORD NumFullBuffers=0;
	for(DWORD i=0;i<numAudioBuffers;i++) if(AudioHDR[i].dwFlags&WHDR_DONE)NumFullBuffers++;
	if(NumFullBuffers>=numAudioBuffers)
	{ // all buffers are done and programm not fast enough to read !!!
		c_audio_in::SetLastError(ERR_C_AUDIO_IN_BUFF_OVERRUN);		
	}	
}

void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	switch(uMsg)
	{
	case WIM_CLOSE:
		break;

	case WIM_DATA:		
			((c_audio_in * ) dwInstance)->ProcessHeader((WAVEHDR *)dwParam1);					
		break;

	case WIM_OPEN:
		break;

	default:
		break;
	}
}

BOOL c_audio_in::Start()
{
	Stop();
	CurrAudioBufferReadIndex=0;
	CurrAuioBufferRemainDataForRead=0; 	

	BOOL res=TRUE;	
	MMRESULT MMError=waveInOpen(&hDevice, 
				WaveParam.GetDeviceID(), WaveParam.GetWAVEFORMATEX(), 
				(DWORD_PTR)waveInProc, (DWORD_PTR)this,
                CALLBACK_FUNCTION | WAVE_FORMAT_DIRECT);  
	
	if(MMError!=MMSYSERR_NOERROR)
	{
		c_audio_in::SetLastError(ERR_C_AUDIO_IN_GET_DEV_CAPS, MMError);		
		return FALSE;
	}
	else 
	{
		Reset();
	}
	
	for(DWORD HeaderIndex=0;HeaderIndex<numAudioBuffers&&res;HeaderIndex++)
	{
		memset(&AudioHDR[HeaderIndex],0,sizeof(WAVEHDR)); // necessary !!!
		AudioHDR[HeaderIndex].lpData = (LPSTR)&AudioArray[HeaderIndex*audioBufferSizeBytes];
		AudioHDR[HeaderIndex].dwBufferLength = audioBufferSizeBytes;		
		
		MMError=waveInPrepareHeader(hDevice, &AudioHDR[HeaderIndex],sizeof(WAVEHDR));
		if(MMError!=MMSYSERR_NOERROR)
		{
			c_audio_in::SetLastError(ERR_C_AUDIO_IN_PREP_HEADER, MMError);
			res=FALSE;	
			break;
		}
		
		PutHeaderToAudioDeviceQueue(&AudioHDR[HeaderIndex]);		
	}	
	
	if(res)
	{
		DeviceStopping=FALSE;
		Paused = FALSE;
		waveInStart(hDevice);
	}
	else
	{
		for(DWORD HeaderIndex=0;HeaderIndex<numAudioBuffers;HeaderIndex++)
		{
			waveInUnprepareHeader(hDevice, &AudioHDR[HeaderIndex],sizeof(WAVEHDR));					
		}
		waveInClose(hDevice);
	}

	return res;
}

BOOL c_audio_in::Stop()
{
	if(hDevice==NULL||DeviceStopping) return TRUE;
	BOOL res=TRUE;

	DeviceStopping=TRUE; // indicate for callback function, that it must stop
	Reset();

	// waiting all headers are done
	for(DWORD HeaderIndex=0;HeaderIndex<numAudioBuffers;)
	{
		if(!(AudioHDR[HeaderIndex].dwFlags&WHDR_DONE))
		{			
			if(WaitForSingleObject(NewAudioDataReady,500)==WAIT_ABANDONED) break;			
			HeaderIndex=0; continue;
		}
		HeaderIndex++;
	}

	for(DWORD HeaderIndex=0;HeaderIndex<numAudioBuffers;HeaderIndex++)
	{
		MMRESULT MMError=waveInUnprepareHeader(hDevice, &AudioHDR[HeaderIndex],sizeof(WAVEHDR));
		if(MMError!=MMSYSERR_NOERROR)
		{
			c_audio_in::SetLastError(ERR_C_AUDIO_IN_UNPREP_HEADER, MMError);			
			res=FALSE;
		}		
	}

	waveInClose(hDevice);
	hDevice=NULL;
	Paused = FALSE;

	ResetEvent(NewAudioDataReady);
	return res;
}

BOOL __fastcall c_audio_in::PutHeaderToAudioDeviceQueue(WAVEHDR * CurrHdr)
{	
	MMRESULT MMError=waveInAddBuffer(hDevice, CurrHdr,sizeof(WAVEHDR));	
	if(MMError!=MMSYSERR_NOERROR)
	{ // error add buffer into queue
		c_audio_in::SetLastError(ERR_C_AUDIO_IN_ADD_BUF, MMError);
		DeviceStopping=TRUE; // indicate for callback function, that it must stop
		Reset();		
		return FALSE;
	}
	return TRUE;
}

BOOL __fastcall c_audio_in::ReadData(BYTE * Data, DWORD DataLen)
{
	while(DataLen&&!DeviceStopping)
	{
		if(!CurrAuioBufferRemainDataForRead)
		{// we finished current buffer or it first read with device
			// waiting while next buffer will be done
			while(!(AudioHDR[CurrAudioBufferReadIndex].dwFlags&WHDR_DONE)&&!DeviceStopping)				
				if(WaitForSingleObject(NewAudioDataReady,500)==WAIT_ABANDONED)
					return FALSE;

			CurrAuioBufferRemainDataForRead = AudioHDR[CurrAudioBufferReadIndex].dwBytesRecorded;
		}

		DWORD CurrDataAvailable=min(DataLen,CurrAuioBufferRemainDataForRead);
		BYTE *pData =(BYTE*)AudioHDR[CurrAudioBufferReadIndex].lpData; 
		pData+=(AudioHDR[CurrAudioBufferReadIndex].dwBytesRecorded-CurrAuioBufferRemainDataForRead);
		memcpy(Data,pData,CurrDataAvailable);

		Data+=CurrDataAvailable;
		CurrAuioBufferRemainDataForRead-=CurrDataAvailable;
		DataLen-=CurrDataAvailable;

		if(!CurrAuioBufferRemainDataForRead)
		{ // we finished curr buffer, and putting it in read queue
			if(!PutHeaderToAudioDeviceQueue(&AudioHDR[CurrAudioBufferReadIndex])) return FALSE;
			CurrAudioBufferReadIndex=(CurrAudioBufferReadIndex+1)%numAudioBuffers;			
		}						
	}
	return (DataLen==0);
}

BOOL c_audio_in::RecordToWav(TCHAR * Filename, int RecordTime_ms)
{
	c_wave CurrWave;	
	CurrWave.SetWaveFormat(WaveParam.GetWAVEFORMATEX());
	return CurrWave.Record(Filename, RecordTime_ms, WaveParam.GetDeviceID());	
}
