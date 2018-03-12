#include "stdafx.h"
#include "c_codepage.h"

typedef struct
{
	int CodePage;
    const char * Name;
    const char * DescRus;
}s_CP_Definitions;

const s_CP_Definitions CP_Definitions[]=
{
	37,  "", "37    (IBM EBCDIC - США и Канада)",
	437, "", "437   (OEM - США)",
	500, "", "500   (IBM EBCDIC - международная)",
	850, "", "850   (OEM - многоязычная латиница 1)",
	855, "", "855   (OEM - кириллица традиционная)",
	860, "", "860   (OEM - португальская)",
	861, "", "861   (OEM - исландская)",
	863, "", "863   (OEM - франко-канадская)",
	865, "", "865   (OEM - скандинавская)",
	866, "", "866   (OEM - русская)",
	874, "windows-874",		"874   (ANSI/OEM - тайская)",
	932, "", "932   (ANSI/OEM - японская Shift-JIS)",
	936, "", "936   (ANSI/OEM - китайская упрощенная GBK)",
	949, "", "949   (ANSI/OEM - корейская)",
	950, "", "950   (ANSI/OEM - китайская традиционная Big5)",
	1250, "", "1250  (ANSI - центрально-европейская)",
	1251, "windows-1251",	"1251  (ANSI - кириллица)",
	1252, "windows-1252",	"1252  (ANSI - латиница 1)",
	1253, "windows-1253",	"1251  (ANSI - greek)",
	1254, "windows-1254",	"1254  (ANSI - турецкая)",
	1255, "windows-1255",	"1255  (ANSI - иврит)",
	1256, "windows-1256",	"1256  (ANSI - арабская)",
	1257, "windows-1257",	"1257  (ANSI - балтийская)",
	1258, "windows-1258",	"1258  (ANSI/OEM - вьетнамская)",
	10000, "", "10000 (MAC - латиница)",
	10007, "maccyrillic",	"10007 (MAC - кириллица)",
	10017, "", "10017 (MAC - украинская)",
	10079, "", "10079 (MAC - исландская)",
	20127, "", "20127 (США-ASCII)",
	20261, "", "20261 (T.61)",
	20866, "koi8-r",		"20866 (русская - КОИ8)",
	21866, "koi8-u",		"21866 (украинская - КОИ8-U)",
	28591, "iso-8859-1",	"28591 (ISO 8859-1 латиница 1)",
	28592, "iso-8859-2",	"28592 (ISO 8859-2 центральноевропейская)",
	28595, "iso-8859-5",	"28595 (ISO 8859-5 кириллица)",
	28597, "iso-8859-7",	"28597 (ISO 8859-7 греческая)",
	28598, "iso-8859-8",	"(ISO 8859-8 Hebrew)",	
	28605, "iso-8859-15",	"28605 (ISO 8859-15 латиница 9)",
	65000, "utf-7",			"65000 (UTF-7)",
	65001, "utf-8",			"65001 (UTF-8)"
};



int c_codepage::GetCodePage(char * CodePageName, __int32 NameLen)
{
	if(NameLen==0)NameLen=strlen(CodePageName);
	if(NameLen==0) return 0;

	// to lower
	for(int i=0;i<NameLen;i++)
    {
        CodePageName[i] = (char)tolower(CodePageName[i]);
	}
    // delete spaces before
    for(int i=0;i<NameLen;i++)
    {
        if(CodePageName[0]!=' ') break;
        CodePageName++;
    }

	int NumSupportedCodepages=sizeof(CP_Definitions)/sizeof(s_CP_Definitions);
	for (int i=0;i<NumSupportedCodepages;i++)
	{
		if(strlen(CP_Definitions[i].Name)==0) continue; // not supported meanwhile
		if(!strcmp(CodePageName,CP_Definitions[i].Name))
		{
			return CP_Definitions[i].CodePage;
		}
	}
	return 0;
}



int c_codepage::GetRequiredBufferSizeToConvert(int CodePageNumber, char * Data, __int32 DataLen)
{
	// not realized
	return 0;
}

BOOL c_codepage::Convert(int CodePageNumber,  char * DataOut, __int32  DataOutLen, char * DataIn, __int32 DataInLen)
{
	//if(WideCharToMultiByte(CodePageNumber,0, DataIn, DataInLen, DataOut, DataOutLen)==0) return false;
	//return true;
	return false;
}
