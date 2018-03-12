#ifndef __C_BZIP2_H__
#define __C_BZIP2_H__

#include "bzlib.h"

#ifdef _MSC_VER	
	// because some functions overlapped from these library
	#pragma comment(linker, "/NODEFAULTLIB:libcmt.lib")	
#endif
#pragma comment( lib, "libbz2.lib")

#define BZIP2_INT_BUF_SIZE	(1024*16)

typedef unsigned __int32 (*c_bzip2_input)  (char* &pBuffer, PVOID Args);
typedef unsigned __int32 (*c_bzip2_output) (const char* pBuffer,unsigned __int32 nLength, PVOID Args);
typedef void (*c_bzip2_callback) (float ProgressPercent);

class c_bzip2
{
public:
	BOOL SetInternalBufSize(unsigned __int32 nWriteBufferSize=BZIP2_INT_BUF_SIZE)
	{
		if(m_pWriteBuffer==NULL||nWriteBufferSize!=m_nWriteBufferSize)
		{
			m_pWriteBuffer=new char[nWriteBufferSize];
			if(!m_pWriteBuffer)return false;
			m_nWriteBufferSize=nWriteBufferSize;
		}
		return true;
	}

	void BeginCompress(int nBlockSize=9,int nWorkFactor=30)
	{
		FinalizeCompressDecompress();
		ZeroMemory(&m_Stream_comp,sizeof(bz_stream));
		
					
		BZ2_bzCompressInit(&m_Stream_comp,nBlockSize,0,nWorkFactor);
		compressProcessing=true;		
	}

	void BeginDecompress(int nSmall = false)
	{
		FinalizeCompressDecompress();
		ZeroMemory(&m_Stream_decomp,sizeof(bz_stream));		
					
		BZ2_bzDecompressInit(&m_Stream_decomp,1,nSmall);
		decompressProcessing=true;	
	}	

	void FinalizeCompressDecompress()
	{
		if(compressProcessing)
		{
			BZ2_bzCompressEnd(&m_Stream_comp);
			compressProcessing=false;
		}

		if(decompressProcessing)
		{
			BZ2_bzDecompressEnd(&m_Stream_decomp);
			decompressProcessing=false;
		}
	}
public:	
	c_bzip2(int InternalBufSize=BZIP2_INT_BUF_SIZE)
	{
		compressProcessing = false;
		decompressProcessing = false;	
		m_pWriteBuffer=NULL;
		SetInternalBufSize(InternalBufSize); // default buffer size
	}
	~c_bzip2()
	{
		FinalizeCompressDecompress();
		if(m_pWriteBuffer!=NULL) delete m_pWriteBuffer;
	}

	int Compress(c_bzip2_input InputFunc, c_bzip2_output OutFunc, __int32 nBlockSize=9,int nWorkFactor=30, PVOID InFArgs=NULL, PVOID OutFArgs=NULL);
	int Decompress(c_bzip2_input InputFunc, c_bzip2_output OutFunc,int nSmall = false, PVOID InFArgs=NULL, PVOID OutFArgs=NULL);

	unsigned __int32 CompressFile(const TCHAR * PathIn, const TCHAR * PathOut, c_bzip2_callback ProgressCallBack=NULL, const __int32 ReadFileBufSize=BZIP2_INT_BUF_SIZE);
	unsigned __int32 DecompressFile(const TCHAR * PathIn,const  TCHAR * PathOut,c_bzip2_callback ProgressCallBack=NULL, const __int32 ReadFileBufSize=BZIP2_INT_BUF_SIZE);

protected:
	bz_stream m_Stream_comp;
	bz_stream m_Stream_decomp;
	char* m_pWriteBuffer;
	unsigned __int32 m_nWriteBufferSize;	

	bool compressProcessing;
	bool decompressProcessing;	
};


#endif //__C_BZIP2_H__