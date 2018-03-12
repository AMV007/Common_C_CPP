#include "stdafx.h"
#include <malloc.h>
#include <algorithm>
#include <stdlib.h>
#include "c_wave.h"
#include "WINDOWS/c_audio_in.h"
#include "WINDOWS/c_audio_out.h"



// load parameters from .wav format file format type
BOOL c_wave::SetParamFromWaveFormat(s_WAVEFORMAT CurrFormat)
{
	if (strncmp((LPCSTR)CurrFormat.id, "fmt", 3) != 0 
		||CurrFormat.wFormatTag != WAVE_FORMAT_PCM		
		) 
		return FALSE;
	memcpy(&waveFormat, &CurrFormat.wFormatTag, 16);		
	waveFormat.cbSize = sizeof(WAVEFORMATEX);
	return TRUE;
}

// generate .wav format from current parameters for .wav file format type
s_WAVEFORMAT c_wave::GetWaveFileFormat(void)
{
	s_WAVEFORMAT CurrFormat;		
	WriteConstCharToArray(CurrFormat.id,"fmt ");		
	CurrFormat.size = 16; // for PCM
	memcpy(&CurrFormat.wFormatTag, &waveFormat, 16);
	return CurrFormat;
}	

// generate .wav file header
s_WAVEDESCR c_wave::GetWaveFileDescr(DWORD DataSize, DWORD FormatSize)
{
	if(!DataSize)DataSize=WaveDataLen;
	s_WAVEDESCR CurrHeader;	

	WriteConstCharToArray(CurrHeader.riff,"RIFF");
	WriteConstCharToArray(CurrHeader.wave,"WAVE");		

	int SubChunk1Size=FormatSize;
	int SubChunk2Size=DataSize;
	CurrHeader.size = 4 + (8 + SubChunk1Size) + (8 + SubChunk2Size);	

	return CurrHeader;
}

s_WAVEDATA c_wave::GetWaveDataHeader(DWORD DataSize)
{
	if(!DataSize)DataSize=WaveDataLen;
	s_WAVEDATA CurrDataHeader;
	memcpy(CurrDataHeader.id,"data",4);
	CurrDataHeader.size = DataSize;	
	return CurrDataHeader;
}

BOOL c_wave::SetWaveDataSize(DWORD NewDataSize)
{
	WaveData=(BYTE*)realloc(WaveData,NewDataSize);
	if(WaveData==NULL)
	{
		WaveDataLen=0;
		return FALSE;
	}
	WaveDataLen=NewDataSize;
	return TRUE;
};
//--end-private-----------------------------------------------------------

void c_wave::DeleteData()
{
	if(WaveData!=NULL) free(WaveData);
	WaveData=NULL;
	WaveDataLen=0;
}

BOOL c_wave::SetData(BYTE * Data, DWORD DataLen)
{
	DeleteData();
	if(!SetWaveDataSize(DataLen)) return FALSE;	
	memcpy(WaveData, Data,DataLen);	
	return TRUE;
}

BOOL c_wave::AddData(BYTE * Data, DWORD DataLen)
{
	if(!SetWaveDataSize(WaveDataLen+DataLen)) return FALSE;
	memcpy(WaveData+WaveDataLen,Data,DataLen);		
	WaveDataLen+=DataLen;
	return TRUE;
};

BOOL c_wave::ReadData(BYTE * Data, DWORD Offset, DWORD DataLen)
{
	if((Offset+DataLen)>=WaveDataLen) return FALSE;
	memcpy(Data, WaveData+Offset, DataLen);
	return TRUE;
}

std::streamsize c_wave::readsome( char* s, std::streamsize n )
{
	if((LastOffset+(int)n)>=WaveDataLen) return 0;
	memcpy(s, WaveData+LastOffset, (int)n);
	LastOffset+=(int)n;
	return n;
}

// loading file into internal buffer
BOOL c_wave::LoadFull(TCHAR * FileName)
{		
	BOOL res=TRUE;
	if(!BeginLoad(FileName)) return FALSE;

	DWORD BytesReaded=0;
	BYTE TempData[512];

	do
	{
		if(!LoadDataDirectFromFile(TempData,sizeof(TempData),&BytesReaded)) 
		{
			res=FALSE;
			break;
		}
		else
		{
			AddData(TempData,BytesReaded);
		}
	}while(BytesReaded);
	
	EndLoad();
	return res;
}

// begining loading File, returning total Data in File
BOOL c_wave::BeginLoad(TCHAR * FileName)
{	
	CurrFile.Open(FileName);
	if(!CurrFile.Opened()) return FALSE;

	TotalWaveFileSize=0;
	CurrChunkSize=0;

	LastOffset =0;

	memset(&waveFormat, 0, sizeof(waveFormat));		

	s_WAVEDESCR CurrHeader;
	if((CurrFile.Read((BYTE*)&CurrHeader,sizeof(CurrHeader))==sizeof(CurrHeader))
		&&!strncmp((LPCSTR)CurrHeader.wave, "WAVE", 4)
		&&!strncmp((LPCSTR)CurrHeader.riff, "RIFF", 4)
		)
	{
		s_WAVEFORMAT CurrFormat;
		if((CurrFile.Read((BYTE*)&CurrFormat,sizeof(CurrFormat))==sizeof(CurrFormat))
			&&!strncmp((LPCSTR)CurrFormat.id, "fmt ", 4)
			&&CurrFormat.wFormatTag == WAVE_FORMAT_PCM		
			)		
		{
			int PossibleExtraFormatData = CurrFormat.size-(sizeof(CurrFormat)-8); // 8 - header			
			CurrFile.SetPos(PossibleExtraFormatData,C_FILE_FILE_CURRENT);			

			SetParamFromWaveFormat(CurrFormat);
			
			int SubChunk1Size=CurrFormat.size;		
			TotalWaveFileSize=CurrHeader.size-(4 + (8 + SubChunk1Size));//SubChunk2Size			

			return TRUE;
		}
	}

	CurrFile.Close();
	return FALSE;
}

// reading Wave raw data direct from file, BeginLoad must be called before
// returning number of Bytes readed, because we not know total wavedata size in file,
// we must read and look at BytesReaded
BOOL c_wave::LoadDataDirectFromFile(BYTE * Data, DWORD DataLen, DWORD * BytesReaded)
{
	if(BytesReaded==NULL||!CurrFile.Opened()) return FALSE;	
	*BytesReaded=0;

	while(DataLen)
	{
		while(!CurrChunkSize)
		{ // reading next chunk size
			if(!TotalWaveFileSize) return TRUE; // finished file

			// or we have next wave chunk in file
			s_WAVEDATA NewChunk;			
			if(CurrFile.Read((BYTE *)&NewChunk, sizeof(NewChunk))!=sizeof(NewChunk))			
				return FALSE;				
			
			TotalWaveFileSize-=(NewChunk.size+8);

			if(strncmp((LPCSTR)NewChunk.id, "data", 4))
			{ // not data chunk, passing it
				CurrFile.SetPos(NewChunk.size, C_FILE_FILE_CURRENT);							
				continue;
			}			
			CurrChunkSize = NewChunk.size;
		}

		DWORD CurrRead = std::min<DWORD>(DataLen,CurrChunkSize);
		
		if(CurrFile.Read(Data, CurrRead)!=CurrRead)
		{
			return FALSE;				
		};

		CurrChunkSize-=CurrRead;
		DataLen-=CurrRead;		
		Data+=CurrRead;	
		*BytesReaded+=CurrRead;
	}
	return TRUE;
}

void c_wave::EndLoad()
{
	CurrFile.Close();
}

BOOL c_wave::SaveFull(TCHAR * FileName)
{
	if(!BeginSave(FileName, WaveDataLen)) return FALSE;
	if(!SaveDataDirectIntoFile(WaveData, WaveDataLen)) return FALSE;	
	EndSave();		
	return TRUE;
}

// creating file and saving Wave Header
BOOL c_wave::BeginSave(TCHAR * FileName, DWORD TotalData)
{
	CurrFile.Open(FileName,TRUE,TRUE,TRUE);
	if(!CurrFile.Opened()) return FALSE;

	s_WAVEFORMAT CurrFormat = GetWaveFileFormat();
	s_WAVEDESCR CurrHeader= GetWaveFileDescr(TotalData,CurrFormat.size);
	s_WAVEDATA CurrDataHeader = GetWaveDataHeader(TotalData);

	if(CurrFile.Write((BYTE*)&CurrHeader,sizeof(CurrHeader))!=sizeof(CurrHeader))
		return FALSE;

	if(CurrFile.Write((BYTE*)&CurrFormat,sizeof(CurrFormat))!=sizeof(CurrFormat))
		return FALSE;
	
	if(CurrFile.Write((BYTE*)&CurrDataHeader,sizeof(CurrDataHeader))!=sizeof(CurrDataHeader))
		return FALSE;
	return TRUE;	
}

// writing data direct into file, BeginSave must be called before
BOOL c_wave::SaveDataDirectIntoFile(BYTE * Data, DWORD DataLen)
{
	if(!CurrFile.Opened()) return FALSE;
	return (CurrFile.Write(Data,DataLen)==DataLen);
}

// finishing saving, BeginSave must be called before
void c_wave::EndSave()
{
	CurrFile.Close();
}

#ifdef _WIN32
BOOL c_wave::Play(TCHAR * FileName, int DeviceID)
{
	c_audio_out auido_out;
	if(DeviceID!=-1) auido_out.SetParam(DeviceID);
	
	if(!BeginLoad(FileName)) return FALSE;

	auido_out.SetParam(&GetWaveFormat());

	BOOL res=auido_out.Start();	
	if(res)
	{
		BYTE TempData[512];
		DWORD CurrBytesRead=0;
		do
		{			
			if(!LoadDataDirectFromFile(TempData, sizeof(TempData),&CurrBytesRead))
			{
				res=FALSE;
				break;
			}		
			res=auido_out.WriteData(TempData, CurrBytesRead);														
		}while(CurrBytesRead&&res);

		if(auido_out.Paused) auido_out.Resume();				
		if(res) res=auido_out.Stop();
		else auido_out.Stop();
	}
	
	EndLoad();
	return res;
}

BOOL c_wave::Record(TCHAR * FileName, int RecordTime_ms, int DeviceID)
{
	c_audio_in audio_in;

	if(DeviceID!=-1) audio_in.SetParam(DeviceID);
	
	int DataToRecord = audio_in.GetParam()->nAvgBytesPerSec*RecordTime_ms/1000;	
		
	SetWaveFormat(audio_in.GetParam());
	if(!BeginSave(FileName,DataToRecord)) return FALSE;

	BOOL res=audio_in.Start();
	if(res)
	{
		BYTE TempData[512];
		while(DataToRecord&&res)
		{
			int CurrWrite = std::min<DWORD>(DataToRecord, sizeof(TempData));
			if(res)res=audio_in.ReadData(TempData, CurrWrite);
			if(res)res=SaveDataDirectIntoFile(TempData, CurrWrite);
			DataToRecord-=CurrWrite;
		}		

		if(res)res=audio_in.Stop();
		else audio_in.Stop();
	}	

	EndSave();	
	return res;
}
#endif
