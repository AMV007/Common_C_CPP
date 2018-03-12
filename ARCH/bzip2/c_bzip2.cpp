#include "stdafx.h"
#include "c_bzip2.h"
#include "..\\..\\COMMON\\c_file.h"

typedef struct
{
	c_file *		File;
	char *			ReadBuf;
	unsigned int	ReadBufSize;
	c_bzip2_callback ProgressCallBack;
	__int64 ProgressSize;
	__int64 FileSize;
}s_InternalFileDataStruct;

c_file * InFComp=NULL,
		 OutFCom=NULL,
		 InFDecomp=NULL,
		 OutFDecomp=NULL;

unsigned __int32 InternalInputFunc (char* &pBuffer,PVOID Args=NULL)
{
	if(Args==NULL) return 0;
	s_InternalFileDataStruct * TempFInf = (s_InternalFileDataStruct*)Args;
	unsigned __int32 res=TempFInf->File->Read((PBYTE)(pBuffer=TempFInf->ReadBuf),TempFInf->ReadBufSize);	
	if(TempFInf->ProgressCallBack!=NULL)
	{
		TempFInf->ProgressSize+=res;
		TempFInf->ProgressCallBack((float)(((TempFInf->ProgressSize*100.0)/TempFInf->FileSize)));	
	}
	return res;
}
unsigned __int32 InternalOutputFunc (const char* pBuffer,unsigned __int32 nLength, PVOID Args=NULL)
{
	if(Args==NULL) return 0;
	c_file * TempF = (c_file*)Args;
	return TempF->Write((PBYTE)pBuffer,nLength);
}


unsigned __int32 c_bzip2::CompressFile(const TCHAR * PathIn, const TCHAR * PathOut, 
								   c_bzip2_callback ProgressCallBack, __int32 ReadFileBufSize)
{
	unsigned __int32 res=0;
	c_file * InF = new c_file(PathIn);
	c_file * OutF = new c_file(PathOut,true,true,true);

	s_InternalFileDataStruct InFStruct;
	InFStruct.ReadBufSize=ReadFileBufSize;
	InFStruct.ReadBuf = new char [ReadFileBufSize];
	InFStruct.File=InF;	
	InFStruct.ProgressCallBack=ProgressCallBack;
	InFStruct.ProgressSize=0;
	if(ProgressCallBack) InFStruct.FileSize = InF->GetFileSize64();	

	if(InF->Opened()&&OutF->Opened()&&InFStruct.ReadBuf!=NULL)
	{
		res=Compress(InternalInputFunc, InternalOutputFunc, 9,30,&InFStruct, OutF);
	}
	
	if(InF)delete InF;
	if(OutF)delete OutF;
	if(InFStruct.ReadBuf)delete InFStruct.ReadBuf;
	return res;
}

unsigned __int32 c_bzip2::DecompressFile(const TCHAR * PathIn, const TCHAR * PathOut, 
									 c_bzip2_callback ProgressCallBack, __int32 ReadFileBufSize)
{
	unsigned __int32 res=0;
	c_file * InF = new c_file(PathIn);
	c_file * OutF = new c_file(PathOut,true,true,true);

	s_InternalFileDataStruct InFStruct;
	InFStruct.ReadBufSize=ReadFileBufSize;
	InFStruct.ReadBuf = new char [ReadFileBufSize];
	InFStruct.File=InF;
	InFStruct.ProgressCallBack=ProgressCallBack;
	InFStruct.ProgressSize=0;
	if(ProgressCallBack) InFStruct.FileSize = InF->GetFileSize64();	

	if(InF->Opened()&&OutF->Opened()&&InFStruct.ReadBuf!=NULL)
	{
		res=Decompress(InternalInputFunc, InternalOutputFunc, false,&InFStruct, OutF);
	}
	
	if(InF)delete InF;
	if(OutF)delete OutF;
	if(InFStruct.ReadBuf)delete InFStruct.ReadBuf;
	return res;
}

int c_bzip2::Compress(c_bzip2_input InputFunc, c_bzip2_output OutFunc, __int32 nBlockSize,int nWorkFactor, PVOID InFArgs, PVOID OutFArgs)
{	
	if(InputFunc==NULL||OutFunc==NULL) return -1;

	BeginCompress(nBlockSize,nWorkFactor);

	m_Stream_comp.next_out=m_pWriteBuffer;
	m_Stream_comp.avail_out=m_nWriteBufferSize;
	while (true)
	{
		if (m_Stream_comp.avail_in==0)
		{
			if ((m_Stream_comp.avail_in=InputFunc(m_Stream_comp.next_in, InFArgs))==0)
			{	// No more data left to read
				int nError;
				while (true)
				{
					nError=BZ2_bzCompress(&m_Stream_comp,BZ_FINISH);
					switch (nError)
					{
					case BZ_FINISH_OK:
					case BZ_STREAM_END:						
						OutFunc(m_pWriteBuffer,m_nWriteBufferSize-m_Stream_comp.avail_out, OutFArgs);						
						break;
					default:
						FinalizeCompressDecompress();
						return 0;
					}
					if (nError==BZ_STREAM_END)
					{
						FinalizeCompressDecompress();
						return m_Stream_comp.total_out_lo32;
					}
					m_Stream_comp.next_out=m_pWriteBuffer;
					m_Stream_comp.avail_out=m_nWriteBufferSize;
				}
			}
		}
		if (BZ2_bzCompress(&m_Stream_comp,BZ_RUN)!=BZ_RUN_OK)
		{
			break;
		}

		if (m_Stream_comp.avail_out==0)
		{	// Flush data
			OutFunc(m_pWriteBuffer,m_nWriteBufferSize,OutFArgs);
			m_Stream_comp.next_out=m_pWriteBuffer;
			m_Stream_comp.avail_out=m_nWriteBufferSize;
		}
	}	

	FinalizeCompressDecompress();
	return 0;	// Won't be executed
}

int c_bzip2::Decompress(c_bzip2_input InputFunc, c_bzip2_output OutFunc,int nSmall, PVOID InFArgs, PVOID OutFArgs)
{
	if(InputFunc==NULL||OutFunc==NULL) return -1;

	BeginDecompress(nSmall);

	m_Stream_decomp.next_out=m_pWriteBuffer;
	m_Stream_decomp.avail_out=m_nWriteBufferSize;
	while (true)
	{
		if (m_Stream_decomp.avail_in==0)
		{
			if ((m_Stream_decomp.avail_in=InputFunc(m_Stream_decomp.next_in, InFArgs))==0)
			{	// No more data left to read
				int nError;
				while (true)
				{
					nError=BZ2_bzDecompress(&m_Stream_decomp);
					switch (nError)
					{
					case BZ_OK:
					case BZ_STREAM_END:
						OutFunc(m_pWriteBuffer,m_nWriteBufferSize-m_Stream_decomp.avail_out, OutFArgs);
						break;
					default:
						FinalizeCompressDecompress();
						return 0;
					}
					if (nError==BZ_STREAM_END)
					{
						FinalizeCompressDecompress();
						return m_Stream_decomp.total_out_lo32;
					}
					m_Stream_decomp.next_out=m_pWriteBuffer;
					m_Stream_decomp.avail_out=m_nWriteBufferSize;
				}
			}
		}
		switch (BZ2_bzDecompress(&m_Stream_decomp))
		{
		case BZ_OK:
			OutFunc(m_pWriteBuffer,m_nWriteBufferSize-m_Stream_decomp.avail_out,OutFArgs);
			m_Stream_decomp.next_out=m_pWriteBuffer;
			m_Stream_decomp.avail_out=m_nWriteBufferSize;
			break;
		case BZ_STREAM_END:
			OutFunc(m_pWriteBuffer,m_nWriteBufferSize-m_Stream_decomp.avail_out,OutFArgs);
			
			FinalizeCompressDecompress();
			return m_Stream_decomp.total_out_lo32;
			break;
		default:
			FinalizeCompressDecompress();
			return 0;
		}
	}	

	FinalizeCompressDecompress();
	return 0;
}