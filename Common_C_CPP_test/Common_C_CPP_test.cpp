// Common_C_CPP_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <time.h>
//#include "vld.h"
//#pragma(lib,"..\\\PRJ\LIB\Common_C_CPP\ARCH\bzip2")
#pragma comment( lib, "..\\debug\\Common_C_CPP.lib")
#include "..\\COMMON\\c_List.h"

#include <iostream>
#include <ctype.h>

#include "..\\TEST\\RunTests.h"
#include "..\IP\c_sock_base.h"

void RunBzipTest();
void RunBzipTestManual();

int rsatest();
void c_audio_in_test();
void c_audio_out_test(TCHAR * FileName=NULL);
void c_audio_out_test_gen(double freq);

void TestFilter();
void c_fft1D_templ_test();

int _tmain(int argc, _TCHAR* argv[])
{	
	c_sock_base *sb = new c_sock_base();
	sb->Create(0,C_SOCK_WSMAN_PORT,"10.0.59.45");
	if(!sb->Connect_s())
	{
		printf("Error connect socket %d", sb->GetWSAError());
	}
	delete sb;
	//c_fft1D_templ_test();
	//TestFilter();
	//c_audio_out_test_gen(1000);
	//c_audio_in_test();
	//c_audio_out_test(TEXT("e:\\SP0000.WAV"));	
	//rsatest();	

	//std::string xx = "W05vcnRvbiBBbnRpU3BhbV0gS3JlbWxpbi5ydTog0JXQttC10LTQvdC10LLQvdCw0Y8g0YDQsNGB0YHRi9C70LrQsA";
	//std::string xx1=base64_decode(xx);
	

	//TestSMTP();
	//TestPOP3();
	
	printf("\n");
	system("pause");
	return 0;
}

