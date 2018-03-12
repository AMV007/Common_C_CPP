#include "stdafx.h"
#include "..\\..\\COMMON\\Compatability.h"
#include "c_encoding.h"
#include "..\\base64.h"

const char * ENC_Names[]=
{
     "7bit",
     "quoted-printable",
     "base64",
     "8bit",
     "binary"
};

e_MIME_Encoding c_encoding::GetEncoding(char * EncodingName, __int32 NameLen)
{
	if(NameLen==0)NameLen=strlen(EncodingName);
	if(NameLen==0) return EMIME_Unknown;

	// to lower
	for(int i=0;i<NameLen;i++)
    {
		EncodingName[i] = (char)tolower(EncodingName[i]);        
	}

    // delete spaces before
    for(int i=0;i<NameLen;i++)
    {
        if(EncodingName[0]!=' ') break;
        EncodingName++;
    }

	int NumSupportedEncodings=sizeof(ENC_Names)/sizeof(ENC_Names[0]);
	for (int i=0;i<NumSupportedEncodings;i++)
	{
		if(!strcmp(EncodingName,ENC_Names[i]))
		{
			return (e_MIME_Encoding)i;
		}
	}
	return EMIME_Unknown;
}

char * c_encoding::Encode(e_MIME_Encoding MIME_Encoding,  const char * DataIn, __int32 DataInLen)
{
    if(DataInLen==0)DataInLen=strlen(DataIn);
    if(DataInLen==0) return NULL;
    switch(MIME_Encoding)
    {
        case EMIME_quoted_printable:
            return (char *)qp_encode((BCHAR *)DataIn,DataInLen);
		case EMIME_base64:
			return (char *)base64_encode((unsigned char *)DataIn).c_str();
        default:
            break;
    }
    return NULL;
}

char * c_encoding::Decode(e_MIME_Encoding MIME_Encoding,  const char * DataIn, __int32 DataInLen)
{
    if(DataInLen==0)DataInLen=strlen(DataIn);
    if(DataInLen==0) return NULL;
    switch(MIME_Encoding)
    {
        case EMIME_quoted_printable:
            return (char *)qp_decode((BCHAR *)DataIn,DataInLen);
		case EMIME_base64:
        {
            std::string res="";
            std::string source=DataIn;
            __int32 LastPos=0;
            bool EndofMessageDetected = false;
            for(int i=0;i<(DataInLen-1)&&!EndofMessageDetected;i++)
            {
                if(DataIn[i]=='='&&DataIn[i+1]=='=')
                {
                    EndofMessageDetected = true;
                }
                if((DataIn[i]=='\r'&&DataIn[i+1]=='\n')|| EndofMessageDetected)
                {
                    __int32 EncStrLen =i-LastPos;
                    if(EncStrLen>0)
                    {
                        std::string tempVal = source.substr(LastPos,EncStrLen);
                        res+=base64_decode(tempVal);
                    }
                    LastPos=i+2;
                }
            }
            if(res=="")
            { // this is not mail message and decode it fully
                res=base64_decode(DataIn);
            }
            char * DataOut = new char [res.length()+1];
            memcpy(DataOut,res.c_str(),res.length());
            DataOut[res.length()]=0;
			return DataOut;
        }
        default:
            break;
    }
    return NULL;
}


// quoted - printable convert-begin------------------------------------
int c_encoding::hex2int( BCHAR x )
{
    return (x >= '0' && x <= '9') ? x - '0' :
        (x >= 'A' && x <= 'F') ? x - 'A' + 10 :
        (x >= 'a' && x <= 'f') ? x - 'a' + 10 :
        0;
}

BCHAR * c_encoding::qp_decode(const BCHAR *qp, __int32 len)
{
    if(len==0)len=strlen((char *)qp)+1;
    const BCHAR *in;
    BCHAR *ret = new BCHAR[len];
    BCHAR *out = ret;

    for (in = qp; *in; in++ ) {
        // Handle encoded chars
        if ( *in == '=' ) {
            if (in[1] && in[2] ) {
                // The sequence is complete, check it
                in++;   // skip the '='
                if (in[0] == '\r' && in[1] == '\n') {
                    // soft line break, ignore it
                    in ++;
                    continue;
                }
                else if ( isxdigit(in[0]) && isxdigit(in[1]) ) {
                    // Ok, we have two hex digit: decode them
                    *out = (hex2int(in[0]) << 4) | hex2int(in[1]);
                    out++;
                    in ++;
                    continue;
                }
            }
            // In all wrong cases leave the original bytes
            // (see RFC 2045). They can be incomplete sequence,
            // or a '=' followed by non hex digit.
        }
        // TODO:
        // RFC 2045 says to exclude control characters mistakenly
        // present (unencoded) in the encoded stream.

        // Copy other characters
        *out = *in;
        out++;
    }
    *out = 0;

    return ret;
}

// A simple version of qp_encoding not used yet
BCHAR *c_encoding::qp_encode(const BCHAR *qp, __int32 len) 
{
        if(len==0)len=strlen((char *)qp)*3+1;
        BCHAR QP_DIGITS[] = TEXT("0123456789ABCDEF");
        BCHAR* ret = new BCHAR[len];
        __int32 i = 0;

        const BCHAR *in;
        for (in = qp; *in; in++ ) {
                if ( (0x21 <= in[0]) & (in[0] <= 0x7e) && in[0] != '=' ) {
            ret[i] = *in;
                        i++;
        }
        else {
            ret[i] = '=';
                        i++;
            ret[i] = QP_DIGITS[in[0] >> 4 & 0xf];
                        i++;
            ret[i] = QP_DIGITS[in[0] & 0xf];
                        i++;
        }
        }

        ret[i] = '\0';

        return ret;
}

bool c_encoding::qp_isNeed(const BCHAR *in) {
        for(int i = 0; i < int(strlen((char *)in)); i++)
                if ( (0x21 > in[i]) | (in[i] > 0x7e) || in[i] == '=' )
                        return true;

        return false;
}


// quoted - printable convert-end------------------------------------


