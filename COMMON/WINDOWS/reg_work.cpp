/*
 * regwork support functions
 * Developed by Maxim Akristiniy <m_akristiniy@rambler.ru>
 * Maintained by Maxim Akristiniy library
 * See LICENSE file for copyright and license info
*/

#include "stdafx.h"
#include <windows.h>

#include "..\\add_log.h"
#include "reg_work.h"
#include "..\\Compatability.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
BOOL 
RegExit (
		 HKEY * v_hRegKey
		 )
{
	if (v_hRegKey!=NULL&&*v_hRegKey != NULL) 
	{
		RegCloseKey (*v_hRegKey);
		*v_hRegKey=(HKEY)NULL;
		return TRUE;
	}
	return FALSE;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
BOOL 
RegInit (
		 HKEY MainKey,	//HKEY_LOCAL_MACHINE
		 LPCTSTR szRegPath,
		 HKEY * v_hRegKey
		 )
{		
	DWORD disposition;

	LONG status = RegCreateKeyEx (MainKey, szRegPath, 0, 
		TEXT(""), 0, KEY_ALL_ACCESS, NULL,  
		v_hRegKey, &disposition);	
	if (status == ERROR_SUCCESS)
	{
		AddLog("(HLM\\%s), returned %d", szRegPath, status);        		
		return TRUE;
	}
	
	AddLogErr("RegCreateKeyEx(HLM\\%s), returned=%d", szRegPath, status);        
	*v_hRegKey = (HKEY) NULL;
	return FALSE;	
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
BOOL 
RegOpen (HKEY MainKey,	//HKEY_LOCAL_MACHINE
		 LPCTSTR szRegPath,
		 HKEY * v_hRegKey,
		 DWORD AccessMask)
{
	//
	// Get the device key from active device registry key
	//
	LONG status = RegOpenKeyEx(
		MainKey,
		szRegPath,
		0,
		AccessMask,
		v_hRegKey);
	if (status== ERROR_SUCCESS)
	{
		AddLog("(HLM\\%s), returned %d", szRegPath, status);        		
		return TRUE;
	}

	AddLogErr("RegOpenKeyEx(HLM\\%s), returned %d", szRegPath, status);        		
	*v_hRegKey = (HKEY) NULL;
	return FALSE;	
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
BOOL 
RegSetStr(
			  HKEY v_hRegKey,
			  LPCTSTR szValueName,
			  LPCTSTR szValue
			  )
{
	DWORD dwSize = (my_wa_strlen(szValue) + 1)*sizeof(TCHAR);

	LONG lStatus=RegSetValueEx (v_hRegKey, (LPCTSTR) szValueName, 0, REG_SZ,
		(LPBYTE) szValue, dwSize);

	if ((lStatus != ERROR_SUCCESS)) {     
		AddLogErr("RegSetValueEx(%s), returned %d", szValueName, lStatus);        
		return FALSE;
	} 
	AddLog("(%s) Value(%s) hKey: 0x%x", szValueName,szValue,v_hRegKey);
	return TRUE;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
BOOL 
RegSetStrMulty (
				   HKEY v_hRegKey,
				   LPCTSTR szValueName,
				   LPCTSTR szValue,
				   __int32 Size
				   )
{    
	LONG lStatus=RegSetValueEx (v_hRegKey, (LPCTSTR) szValueName, 0, REG_MULTI_SZ,
		(LPBYTE) szValue, Size);
	if ((lStatus != ERROR_SUCCESS)) {    
		AddLogErr("RegSetValueEx(%s) -returned %d", szValueName, lStatus);        
		return FALSE;
	} 	
	AddLog("(%s) Value(%s) size=%d, hKey: 0x%x", szValueName,szValue,Size,v_hRegKey);
	return TRUE;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
BOOL 
RegSetDWORD (
			 HKEY v_hRegKey,
			 LPCTSTR szValueName,
			 DWORD   dwValue
			 )
{
	DWORD dwSize = sizeof (DWORD);

	LONG lStatus=RegSetValueEx (v_hRegKey, (LPCTSTR) szValueName, 0, REG_DWORD,
		(LPBYTE) &dwValue, dwSize);

	if ((lStatus != ERROR_SUCCESS)) {   
		AddLogErr("RegSetValueEx(%s) -returned %d", szValueName, lStatus);        
		return FALSE;
	} 	

	AddLog("(%s) Value(0x%x) hKey: 0x%x", szValueName,dwValue,v_hRegKey);
	return TRUE;
}

// test, that value with these name exist
BOOL RegTestValueNameExist(HKEY hKey, PTSTR szValueName)
{	
	LONG lStatus = RegQueryValueEx( hKey, szValueName, NULL, NULL, NULL, NULL);

	if (lStatus != ERROR_SUCCESS) {      

		AddLogErr("RegQueryValueEx(%s) -returned %d", szValueName, lStatus);		
		return FALSE;
	} 
	AddLog("(%s) Value exist, hKey: 0x%x", szValueName,hKey);
	return TRUE;
}

// get DWORD registry value
BOOL RegGetDWORD(HKEY hKey, PTSTR szValueName, PDWORD pdwValue)
{

	DWORD	dwValType, 
			dwValLen = sizeof(DWORD);		

	LONG lStatus = RegQueryValueEx( hKey, szValueName, NULL, &dwValType, (PBYTE)pdwValue, &dwValLen);

	if ((lStatus != ERROR_SUCCESS) || (dwValType != REG_DWORD)) {      

		AddLogErr("RegQueryValueEx(%s) -returned %d", szValueName, lStatus);
		*pdwValue = (DWORD)-1;
		return FALSE;
	} 
	AddLog("(%s) Value(0x%x) hKey: 0x%x", szValueName,*pdwValue,hKey);
	return TRUE;
}

// get string registry value
BOOL RegGetStr(HKEY hKey, PTSTR szValueName, PTCHAR pdwValue, DWORD Size)
{

	DWORD dwValType, 
		  dwValLen = Size;		

	LONG lStatus = RegQueryValueEx( hKey, szValueName, NULL, &dwValType, (PBYTE)pdwValue, &dwValLen);

	if ((lStatus != ERROR_SUCCESS) || (dwValType != REG_SZ)) {     
		AddLogErr("RegQueryValueEx(%s) -returned %d",szValueName, lStatus);        
		*pdwValue = 0;
		return FALSE;
	} 

	AddLog("(%s) Value(%s) hKey: 0x%x", szValueName,pdwValue,hKey);    

	return TRUE;
}



BOOL RegGetStrMulty(HKEY hKey, PTSTR szValueName, PTCHAR pdwValue, DWORD  * Size)
{
	DWORD	dwValType;		

	LONG lStatus = RegQueryValueEx( hKey, szValueName, NULL, &dwValType, (PBYTE)pdwValue, Size);       
	if ((lStatus != ERROR_SUCCESS) || (dwValType != REG_MULTI_SZ)) {     
		AddLogErr("RegQueryValueEx(%s) -returned %d",szValueName, lStatus);        
		*pdwValue = 0;		
		return FALSE;
	} 
	AddLog("(%s) Value(%s) size=%d, hKey: 0x%x", szValueName,pdwValue,*Size,hKey);    
	return TRUE;
}

BOOL RegGetSubKeyNames(HKEY hKey, PTSTR szValueName, DWORD MaxSymbolCount)
{
	if(MaxSymbolCount<2)
	{
		AddLogErr("RegGetSubKeyNames wrong buffer size, minimum 2 mustbe, but is %d",MaxSymbolCount);        
		return FALSE;
	}
	DWORD NumKeys=0;
	DWORD MaxSubKeyLen=0;
	LONG lStatus = RegQueryInfoKey(hKey,NULL,NULL,NULL,&NumKeys,&MaxSubKeyLen,NULL,NULL,NULL,NULL,NULL,NULL);
	if (lStatus != ERROR_SUCCESS)
	{
		AddLogErr("RegQueryInfoKey -returned %d", lStatus);        
		return FALSE;
	}	
	
	int CurrCharCount=0;
	MaxSymbolCount--;// last zero must also be in array

	for(DWORD i=0;i<NumKeys;i++)
	{				
		if((MaxSymbolCount-CurrCharCount)>MaxSubKeyLen)
		{
			DWORD TempKeynameLen=MaxSymbolCount-CurrCharCount;
			// get the key name
			lStatus = RegEnumKeyEx(hKey,i,szValueName+CurrCharCount,&TempKeynameLen,
				NULL,NULL,NULL,NULL);
			if (lStatus != ERROR_SUCCESS)
			{
				AddLogErr("RegEnumKeyEx -returned %d", lStatus);        
				szValueName[CurrCharCount++]=0;	 
				szValueName[CurrCharCount]=0;	 
				return FALSE;
			}
			CurrCharCount+=TempKeynameLen;
			szValueName[CurrCharCount++]=0;
		}
		else
		{			
			AddLogErr("not enough space(%d) for names, current %d, maxkeylen %d",MaxSymbolCount,CurrCharCount, MaxSubKeyLen);        
			szValueName[CurrCharCount++]=0;	 
			szValueName[CurrCharCount]=0;	 
			return FALSE;
		}
	}
	szValueName[CurrCharCount]=0;	 	
	
	return TRUE;
}

BOOL RegGetValuesNames(HKEY hKey, PTSTR szValueName, DWORD MaxSymbolCount)
{
	if(MaxSymbolCount<2)
	{
		AddLogErr("RegGetSubKeyNames wrong buffer size, minimum 2 mustbe, but is %d",MaxSymbolCount);        
		return FALSE;
	}
	DWORD NumValues=0;
	DWORD MaxValuesLen=0;
	LONG lStatus = RegQueryInfoKey(hKey,NULL,NULL,NULL,NULL,NULL,NULL,&NumValues,&MaxValuesLen,NULL,NULL,NULL);
	if (lStatus != ERROR_SUCCESS)
	{
		AddLogErr("RegQueryInfoKey -returned %d", lStatus);        
		return FALSE;
	}	
	
	int CurrCharCount=0;
	MaxSymbolCount--;// last zero must also be in array

	for(DWORD i=0;i<NumValues;i++)
	{				
		if((MaxSymbolCount-CurrCharCount)>MaxValuesLen)
		{
			DWORD TempValueNameLen=MaxSymbolCount-CurrCharCount;
			// get the key name
			lStatus = RegEnumValue(hKey,i,szValueName+CurrCharCount,&TempValueNameLen,
				NULL,NULL,NULL,NULL);
			if (lStatus != ERROR_SUCCESS)
			{
				AddLogErr("RegEnumKeyEx -returned %d", lStatus);        
				szValueName[CurrCharCount++]=0;	 
				szValueName[CurrCharCount]=0;	 
				return FALSE;
			}
			CurrCharCount+=TempValueNameLen;
			szValueName[CurrCharCount++]=0;
		}
		else
		{			
			AddLogErr("not enough space(%d) for names, current %d, maxvaluelen %d",MaxSymbolCount,CurrCharCount, MaxValuesLen);        
			szValueName[CurrCharCount++]=0;	 
			szValueName[CurrCharCount]=0;	 
			return FALSE;
		}
	}
	szValueName[CurrCharCount]=0;	 	
	
	return TRUE;
}

BOOL RegDeleteSubKeysAndValues(HKEY hKey, 
							   TCHAR * TempArray, // array for all values or key names to recieve
							   DWORD TempArrayLen)
{
	LONG lStatus;
	
	if(!RegGetSubKeyNames(hKey,TempArray,TempArrayLen))
	{
		AddLogErr("RegGetSubKeyNames");        			
		return FALSE;
	}

	while(*TempArray!=0)
	{
		lStatus=RegDeleteKey(hKey,TempArray);
		if(lStatus!=ERROR_SUCCESS)
		{			
			AddLogErr("RegDeleteKey -returned %d", lStatus);        			
			return FALSE;
		}
		TempArray+=my_wa_strlen(TempArray)+1;
	}

	if(!RegGetValuesNames(hKey,TempArray,TempArrayLen))
	{
		AddLogErr("RegGetValuesNames");        			
		return FALSE;
	}

	while(*TempArray!=0)
	{
		lStatus=RegDeleteValue(hKey,TempArray);
		if(lStatus!=ERROR_SUCCESS)
		{
			AddLogErr("RegDeleteKey -returned %d", lStatus);        			
			return FALSE;
		}
		TempArray+=my_wa_strlen(TempArray)+1;
	}
	
	return TRUE;
}

