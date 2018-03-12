#ifndef __C_FILTER_H__
#define __C_FILTER_H__
/*
	Coefficient type must be same or rather precision than data type
*/

template <typename DATA_TYPE, typename COEFF_TYPE=double>
class c_fir
{
private:
	DATA_TYPE * data;
	COEFF_TYPE * coeff;

	int filterLen;
	int currDataPtr; // pionter to the current data in array data	

	template <typename DATA_TYPE, typename COEFF_TYPE>	friend class c_iir;	

public:
	c_fir(COEFF_TYPE * Coeff=NULL,int FilterLen=0, DATA_TYPE InitDataVal=0)
	{
		// we must initialize these fields to NULL, for correct class deletion
		data=NULL;
		coeff=NULL;
		InitFilter(Coeff,FilterLen,InitDataVal);		
	}	
	~c_fir()
	{
		if(data!=NULL) delete data;
		if(coeff!=NULL) delete coeff;
	}

	virtual BOOL InitFilter(COEFF_TYPE * Coeff,int FilterLen, DATA_TYPE InitDataVal=0)
	{		
		if(Coeff!=NULL&&FilterLen!=0)		
		{			
			currDataPtr=0;
			filterLen=FilterLen;

			data = new DATA_TYPE[filterLen];
			if(data==NULL) return FALSE;

			coeff = new COEFF_TYPE[filterLen];
			if(coeff==NULL) return FALSE;
			memcpy(coeff,Coeff, filterLen*sizeof(COEFF_TYPE));			
		}

		if(data!=NULL&&FilterLen!=0)
		{
			// initialize data to init value
			if(InitDataVal==0) memset(data,0,filterLen*sizeof(DATA_TYPE));
			else for(int i=0;i<filterLen;i++)data[i] = InitDataVal;	
		}

		return TRUE;
	}

	virtual COEFF_TYPE __fastcall Write(DATA_TYPE Data)
	{
		COEFF_TYPE acc = 0;			// accimulator	
		data[currDataPtr] = Data;	

		// multiplication with accumulation
		// currDataPtr moving in circle and after cycle have currDataPtr=currDataPtr; value
		// so we decrementing counter of variables
		for(int i=0;i<filterLen;i++)
		{
			acc += (coeff[i] * data[currDataPtr]);
			currDataPtr=(currDataPtr+1)%filterLen;
		}	

		currDataPtr=(currDataPtr+1)%filterLen;
		return (DATA_TYPE)acc;
	}
};

//------------------------------------------------------------------------

template <typename DATA_TYPE, typename COEFF_TYPE=double>
class c_iir:c_fir<DATA_TYPE, COEFF_TYPE>
{
private:
	COEFF_TYPE * dataFeedBack;
	COEFF_TYPE * coeffFeedBack;

	int feedbackLen;
	int currFeedBackPtr;
public:
	c_iir(
        COEFF_TYPE * Coeff=NULL,
		int FilterLen=0,
		COEFF_TYPE * CoeffFeedBack=NULL,
		int FilterFeedBackLen=0,
		DATA_TYPE InitVal=0,
		DATA_TYPE InitFeedBackVal=0)
	{
		// we must initialize these fields to NULL, for correct class deletion
		dataFeedBack=NULL;
		coeffFeedBack=NULL;

		InitFilter( Coeff,FilterLen,
                    CoeffFeedBack, FilterFeedBackLen,
                    InitVal,InitFeedBackVal);
	};

	~c_iir()
	{
		if(dataFeedBack!=NULL) delete dataFeedBack;
		if(coeffFeedBack!=NULL) delete coeffFeedBack;
	}

	virtual BOOL InitFilter(
							COEFF_TYPE * Coeff=NULL,
							int FilterLen=0,			
							COEFF_TYPE * CoeffFeedBack=NULL,
							int FilterFeedBackLen=0,
							DATA_TYPE InitVal=0,
							DATA_TYPE InitFeedBackVal=0)
	{
		if(!c_fir<DATA_TYPE,COEFF_TYPE>::InitFilter(Coeff, FilterLen, InitVal)) return FALSE;

		if(CoeffFeedBack!=NULL&&FilterFeedBackLen!=0)		
		{			
			currFeedBackPtr=0;
			feedbackLen=FilterLen;

			dataFeedBack = new COEFF_TYPE[feedbackLen];
			if(dataFeedBack==NULL) return FALSE;

			coeffFeedBack = new COEFF_TYPE[feedbackLen];
			if(coeffFeedBack==NULL) return FALSE;
			memcpy(coeff,Coeff, filterLen*sizeof(COEFF_TYPE));			
		}

		if(data!=NULL&&FilterLen!=0)
		{
			// initialize data to init value
			if(InitFeedBackVal==0) memset(dataFeedBack,0,feedbackLen*sizeof(COEFF_TYPE));
			else for(int i=0;i<feedbackLen;i++)dataFeedBack[i] = InitFeedBackVal;	
		}
		return TRUE;
	}

	virtual DATA_TYPE __fastcall Write(DATA_TYPE Data)
	{
		COEFF_TYPE TempData = c_fir<DATA_TYPE,COEFF_TYPE>::Write(Data);		
		
		COEFF_TYPE acc = 0;			// accimulator

		// multiplication with accumulation
		// currDataPtr moving in circle and after cycle have currDataPtr=currDataPtr; value
		// so we decrementing counter of variables
		for(int i=0;i<(feedbackLen-1);i++)
		{
			acc += (coeffFeedBack[i] * dataFeedBack[currFeedBackPtr]);
			currFeedBackPtr=(currFeedBackPtr+1)%feedbackLen;
		}	

		currFeedBackPtr=(currFeedBackPtr+1)%feedbackLen;
		COEFF_TYPE res=TempData-acc;
		dataFeedBack[currFeedBackPtr] = res;
		currFeedBackPtr=(currFeedBackPtr+1)%feedbackLen;		

		return (DATA_TYPE)res;
	}
};

#endif //__C_FILTER_H__