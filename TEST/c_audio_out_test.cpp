#include "stdafx.h"
#include "..\\DEVS\\AUDIO\\WINDOWS\\c_audio_out.h"
#include "..\\DEVS\\AUDIO\\c_wave.h"
#include "..\\DEVS\\AUDIO\\c_aud_generator.h"


#include <iostream>
#include <ctype.h>

void c_audio_out_test(TCHAR * FileName)
{
	TCHAR * fileName = TEXT("e:\\test.wav");
	if(FileName==NULL) FileName = fileName;

	printf("Scanning output audio devices, please wait ...\n");
	int NumDevs = c_audio_out::GetDevsCount();
	printf("Found total - %d devices\n",NumDevs);
	for (int i=0;i<NumDevs;i++)
	{
		TCHAR DevName[256];
		memset(DevName,0,sizeof(DevName));
		if(c_audio_out::GetDevNameByDevID(i,DevName,256))
		{
			printf("Device %d - %s\n", i+1, DevName);
		}
		else
		{
			printf("Error get device Name %d\n", i+1);
		}
	}
	
	c_audio_out CurrAudio;
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

	printf("Begining playing\n");
	if(!CurrAudio.PlayWav(FileName)) printf("Error play, LastError=%d, MMError=%d\n",CurrAudio.GetLastError(), CurrAudio.GetLastMMError());
	printf("Ending playing, LastError=%d, MMError=%d\n",CurrAudio.GetLastError(), CurrAudio.GetLastMMError());		
}

void c_audio_out_test_gen(double freq)
{
	printf("Begining generation %.2f Hz\n",freq);
	if(!c_aud_generator::Generate(freq,3000))printf("Error in Generation"); ;
	printf("Ending generation\n");
}