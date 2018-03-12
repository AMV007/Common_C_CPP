#include "stdafx.h"
#include "..\\DEVS\\AUDIO\\WINDOWS\\c_audio_in.h"
#include "..\\DEVS\\AUDIO\\c_wave.h"

#include <iostream>
#include <ctype.h>

void c_audio_in_test()
{
	printf("Scanning input audio devices, please wait ...\n");
	int NumDevs = c_audio_in::GetDevsCount();
	printf("Found total - %d devices\n",NumDevs);
	for (int i=0;i<NumDevs;i++)
	{
		TCHAR DevName[256];
		memset(DevName,0,sizeof(DevName));
		if(c_audio_in::GetDevNameByDevID(i,DevName,256))
		{
			printf("Device %d - %s\n", i+1, DevName);
		}
		else
		{
			printf("Error get device Name %d\n", i+1);
		}
	}	

	c_audio_in CurrAudio;
	while(true)
	{
		int deviceNumber=0;
		while(true)
		{
			printf("Please enter device number !\n");
			if(scanf("%d",&deviceNumber)==1) break;
		}
	
		if(!CurrAudio.SetParam(deviceNumber-1))printf("Error Set param, LastError=%d, MMError=%d\n",CurrAudio.GetLastError(), CurrAudio.GetLastMMError());
		else break;
	}		
	
	printf("Begining recording and saving\n");
	if(!CurrAudio.RecordToWav(TEXT("e:\\test.wav"),10000)) printf("Error recording and saving, LastError=%d, MMError=%d\n",CurrAudio.GetLastError(), CurrAudio.GetLastMMError());	
	printf("Ending recording and saving, LastError=%d, MMError=%d\n",CurrAudio.GetLastError(), CurrAudio.GetLastMMError());		
}