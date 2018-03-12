#ifndef __H_C_ENCODING__
#define __H_C_ENCODING__
#include <windows.h> // from BOOL type

typedef enum
{
    EMIME_7bit,
    EMIME_quoted_printable,
    EMIME_base64,
    EMIME_8bit,
    EMIME_binary,
    EMIME_Unknown,
}e_MIME_Encoding;

class c_encoding
{
private:
	// quoted-printable
	static __int32 c_encoding::hex2int( BCHAR x );
	static BCHAR * c_encoding::qp_decode(const BCHAR *qp, __int32 len=0);
	static BCHAR *c_encoding::qp_encode(const BCHAR *qp, __int32 len=0);
	static bool qp_isNeed(const BCHAR *in);
public :    
	c_encoding(){};
	~c_encoding(){};	

    static e_MIME_Encoding GetEncoding(char * EncodingName, __int32 NameLen=0);

    // you must delete return result with delete
    static char * Encode(e_MIME_Encoding MIME_Encoding,  const char * DataIn, __int32 DataInLen=0);
    static char * Decode(e_MIME_Encoding MIME_Encoding,  const char * DataIn, __int32 DataInLen=0);
};
	

#endif //__H_C_ENCODING__