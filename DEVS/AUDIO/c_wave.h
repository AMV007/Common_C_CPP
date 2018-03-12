#ifndef __H_C_WAVE__
#define __H_C_WAVE__

#include "../../COMMON/c_file.h"
#include "WINDOWS/c_wave_param.h"

#include <iostream>

// this class for load save raw data into wave and also play them

#pragma pack(1)

typedef struct
{
	BYTE riff[4];
	DWORD size;
	BYTE wave[4];
} s_WAVEDESCR, *ps_WAVEDESCR;

typedef struct
{
	BYTE id[4];
	DWORD size;
	// next part practically same as WAVEFORMATEX structure
	WORD wFormatTag;
    WORD nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD nBlockAlign;
    WORD wBitsPerSample;
} s_WAVEFORMAT, *ps_WAVEFORMAT;

typedef struct
{
	BYTE id[4];
	DWORD size;
}s_WAVEDATA,*ps_WAVEDATA;

#pragma pack()

class c_wave : public std::iostream
{
private:		
	
	BYTE * WaveData;
	DWORD WaveDataLen;
	WAVEFORMATEX waveFormat;
	c_file CurrFile;

	// for loading File counters
	DWORD TotalWaveFileSize; // total wave file size, it's decrementing during file read
	DWORD CurrChunkSize;

	// load parameters from .wav format file format type
	BOOL SetParamFromWaveFormat(s_WAVEFORMAT CurrFormat);	

	// generate .wav format from current parameters for .wav file format type
	s_WAVEFORMAT GetWaveFileFormat(void);
	s_WAVEDESCR GetWaveFileDescr(DWORD DataSize,DWORD FormatSize);
	s_WAVEDATA GetWaveDataHeader(DWORD DataSize=0);
	BOOL SetWaveDataSize(DWORD NewDataSize);

	// for file read witout offset provide
	DWORD LastOffset;

public:	

	c_wave(TCHAR * FileName=NULL):std::iostream(NULL)
	{
		WaveData=NULL;
		WaveDataLen=0;

		if(FileName!=NULL)
		{
			BeginLoad(FileName);
		}
	}

	~c_wave()
	{
		DeleteData();
		EndLoad();
	}

	void SetWaveFormat(WAVEFORMATEX * NewFormat)
	{
		memcpy(&waveFormat,NewFormat, sizeof(WAVEFORMATEX));
	};
	WAVEFORMATEX GetWaveFormat(){return waveFormat;};
	

	void DeleteData();
	BOOL SetData(BYTE * Data, DWORD DataLen);
	BOOL AddData(BYTE * Data, DWORD DataLen);
	BOOL ReadData(BYTE * Data, DWORD Offset, DWORD DataLen);
	std::streamsize readsome ( char* s, std::streamsize n );
	
	// loading file into internal buffer
	BOOL LoadFull(TCHAR * FileName);	
	
	// begining loading File, returning total Data in File
	BOOL BeginLoad(TCHAR * FileName);	
	// reading Wave raw data direct from file, BeginLoad must be called before
	BOOL LoadDataDirectFromFile(BYTE * Data, DWORD DataLen, DWORD * BytesReaded);	
	void EndLoad();

	BOOL SaveFull(TCHAR * FileName);
	
	// creating file and saving Wave Header
	BOOL BeginSave(TCHAR * FileName, DWORD TotalData);	
	// writing data direct into file, BeginSave must be called before
	BOOL SaveDataDirectIntoFile(BYTE * Data, DWORD DataLen);	
	// finishing saving, BeginSave must be called before
	void EndSave();
	
#ifdef _WIN32
	BOOL Play(TCHAR * FileName, int DeviceID=-1);
	BOOL Record(TCHAR * FileName, int RecordTime_ms, int DeviceID=-1);
#endif
};

#endif //__H_C_WAVE__
