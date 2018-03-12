#include "stdafx.h"

#include "..\\ARCH\\bzip2\\c_bzip2.h"
#include "..\\COMMON\\c_file.h"


void TestCallback(float Value)
{
	static int LastTimeValue=0;
	if((GetTickCount()-LastTimeValue)>1000)
	{
		printf("%f\n",Value);
		LastTimeValue=GetTickCount();
	}
}

void RunBzipTest()
{
	c_bzip2 * bzip2 = new c_bzip2();
	printf("begin decompression \n");
	int res=bzip2->DecompressFile(TEXT("G:\\Tmp\\dict\\XML\\ruwiki-20100331-pages-articles.xml.bz2"),
		TEXT("h:\\temp\\dict\\test.dd"),&TestCallback);
	printf("end decompression %d\n",res);
	delete bzip2;	
}

void RunBzipTestManual()
{
	c_file xx(TEXT("h:\\temp\\dict\\test2.dd"),true,true,true);
	BZFILE *BZ2fp_r = BZ2_bzopen("G:\\Tmp\\dict\\XML\\ruwiki-20100331-pages-articles.xml.bz2","rb");

	char buff[0x1000];
	int len=0;
	while(BZ2fp_r!=NULL&&xx.Opened()&&
		(len=BZ2_bzread(BZ2fp_r,buff,0x1000))>0)
	{
		xx.Write((BYTE*)buff,len);
	}
	BZ2_bzclose(BZ2fp_r);
	xx.Close();
}