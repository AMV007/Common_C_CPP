#include "stdafx.h"
//---------------------------------------------------------------------------
#include "DownloadPage.h"
#include "base64.h"
//#include "wrapper.h"
// just partial class, not fully implemented support, only what I need for moment
//---------------------------------------------------------------------------

#define BLOCK_SIZE  512

AnsiString __fastcall TDownloadPage::RestartDownloadLastPage()
{
	return DownloadPage(LastPage, LastMaxSize);
}

AnsiString __fastcall TDownloadPage::ConvertToASCII(PCHAR Data, __int32 DataLength)
{
	if(DataLength==0) return "";
	AnsiString res=Data;
	res.SetLength(DataLength);
	memcpy((PVOID)res.data(), Data, DataLength);
	return res;
}

bool __fastcall IsHex(char TempChar)
{
	if(isdigit(TempChar)) return true;
	if(   TempChar=='A'||TempChar=='a'
		||TempChar=='B'||TempChar=='b'
		||TempChar=='C'||TempChar=='c'
		||TempChar=='D'||TempChar=='d'
		||TempChar=='E'||TempChar=='e'
		||TempChar=='F'||TempChar=='f'
		)
	{
		return true;
	}
	return false;
}

AnsiString __fastcall TDownloadPage::DownloadPage(AnsiString WebAddress, __int32 MaxSize, BOOL UseGZip, WORD Port, AnsiString PostParam)
{
/*
    WebAddress="http://win.mail.ru/cgi-bin/passremind";
    PostParam="action=login&Username=testwatch2&Domain=mail.ru";
*/
	LastPage=WebAddress;
	LastMaxSize=MaxSize;
	LastError=0;
	AnsiString RelativePath="/", HostName="";
	HostName=Channel.GetHostName(WebAddress,&RelativePath);

	if(HostName=="") return "";

	if(!Channel.Connect(HostName, Port))
	{
		LastError=ERROR_CONNECT_HOST;
		return "";
	}

	if(Channel.ProxySettings.IP!=0) RelativePath=WebAddress;

	AnsiString Request;
    if(PostParam=="")
    {
        Request = "GET " + RelativePath + " HTTP/1.1\r\n";
    }
    else
    {
        Request = "POST " + RelativePath + " HTTP/1.1\r\n";
    }

    if(UseGZip)
    {
        Request+="Accept-Encoding: gzip,deflate\r\n";
    }
    else
    {
        Request+="Accept-Encoding: deflate\r\n";
    }

    Request+="User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1)\r\n";
    Request+="Host: " + HostName + "\r\n";
    Request+="Referer: " + WebAddress + "\r\n";

        //"Content-Length: " + MaxSize + "\r\n" +

    if(Channel.ProxySettings.Login!=""&&Channel.ProxySettings.ProxyType==PTYPE_NO_PROXY)
    {
        Request+="Proxy-Authorization: Basic "+HTTPProxyAuth+"\r\n";
    }


    if(PostParam=="")
    {
        Request+="Connection: Close\r\n\r\n";
    }
    else
    {
        Request+="Accept-Charset:windows-1251\r\n";
        Request+="Content-Length: "+(AnsiString)PostParam.Length()+"\r\n\r\n";
        Request+=PostParam;
    }

	AnsiString result="";
	if(Channel.Send((char *)Request.data(), Request.Length(), 5000))
	{


		int NumBytes;
		int NumBytesRemain=MaxSize;
		do
		{

			int NumDataRecieve =BLOCK_SIZE;
			if(MaxSize!=-1)
			{
				NumDataRecieve=std::min<int>(NumBytesRemain,BLOCK_SIZE);
			}

			NumBytes=Channel.RecvAny(TempData, NumDataRecieve);
			if(NumBytes>0)
			{
				result+=ConvertToASCII(TempData, NumBytes);
			}
			else
			{
				break;
			}

			if(MaxSize!=-1)
			{
				NumBytesRemain-=NumBytes;
				if(NumBytesRemain==0) break;
			}
			if(!Channel.Connected())
			{
				result="";
				break;
			}
		}while(NumBytes>0);
	}

	Channel.Disconnect();

    if(result!="")
    {
        if(result.Length()>60)
        {
            if(result.SubString(0,60).Pos("407 Proxy Authentication Required")!=0)
            {
                LastError=ERROR_GATEWAY_TIMEOUT;
                return "";
            }

            if(result.SubString(0,60).Pos("504 Gateway Timeout")!=0)
            {
                LastError=ERROR_CONNECT_PROXY_AUTHENTICATION;
                return "";
            }
        }

        __int32 GZipHeaderPos=result.Pos("Content-Encoding: gzip");
        if(GZipHeaderPos!=0)
        {
            __int32 InfoEndPos=result.Pos("\r\n\r\n");
            if(InfoEndPos!=0)
            {
                InfoEndPos+=4;
                AnsiString TempString="";
                AnsiString DecodeRes="";
                __int32 StringCount=0;
                for (int i = InfoEndPos; i <result.Length(); i++)
                {
                    if((i<(result.Length()-1)&&result.SubString(i+1,2)=="\r\n"))
                    {
                        AnsiString ter=result.SubString(i,50);

                        if((StringCount==0&&IsHex(TempString[1]))||TempString==0)
                        {

                        }
                        else
                        {
    #ifdef ZUTIL_H
                            AnsiString NewStr;
                            uLong Length=80000;
                            NewStr.SetLength(Length);
                            uLong LastLen=TempString.Length();

                            b_uncompress((PCHAR)NewStr.data(), &Length, (PCHAR)TempString.data(), LastLen);
                            AnsiString tt = base64_decode(TempString.c_str()).c_str();
                            DecodeRes+=tt;
    #endif
                        }

                        StringCount++;
                        i+=2;
                        TempString="";
                    }
                    TempString+=result[i+1];
                }
                result = result.SubString(0,GZipHeaderPos-1)+DecodeRes;
            }

        }else LastError=ERROR_GZIPHEADER;
    }

	return result;
}

void __fastcall TDownloadPage::SetProxy(s_ProxySettings * NewProxySettings)
{
	Channel.SetProxy(NewProxySettings);
	if(Channel.ProxySettings.Login!=""&&Channel.ProxySettings.ProxyType==PTYPE_NO_PROXY)
	{
		AnsiString TempStr=Channel.ProxySettings.Login+":"+Channel.ProxySettings.Pass;
		std::string encoded=base64_encode(TempStr.c_str(), TempStr.Length());
		HTTPProxyAuth=(AnsiString)encoded.c_str();
	}
}

void __fastcall TDownloadPage::HangUp()
{
    Channel.Close();
}

TDownloadPage::TDownloadPage(void)
{
	TempData = new BYTE[BLOCK_SIZE];
}

TDownloadPage::~TDownloadPage(void)
{
	if(TempData!=NULL) delete [] TempData;
}
