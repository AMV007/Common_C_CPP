/*
 * regwork support functions
 * Developed by Maxim Akristiniy <m_akristiniy@rambler.ru>
 * Maintained by Maxim Akristiniy library
 * See LICENSE file for copyright and license info
*/

#ifndef __C_REG_WORK_H__
#define __C_REG_WORK_H__

#ifndef PTCHAR
typedef TCHAR * PTCHAR;
#endif

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
BOOL RegExit (HKEY *v_hRegKey);
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
BOOL RegInit ( HKEY MainKey,	//HKEY_LOCAL_MACHINE
		 LPCTSTR szRegPath,
		 HKEY * v_hRegKey
		 );
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
BOOL 
RegOpen (HKEY MainKey,	//HKEY_LOCAL_MACHINE
		 LPCTSTR szRegPath,
		 HKEY * v_hRegKey,
#ifdef _WIN32_WCE
		 DWORD AccessMask =0
#else
		 DWORD AccessMask =KEY_READ
#endif
		 );
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
BOOL 
RegSetStr(
			  HKEY v_hRegKey,
			  LPCTSTR szValueName,
			  LPCTSTR szValue
			  );

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
BOOL 
RegSetStrMulty (
				   HKEY v_hRegKey,
				   LPCTSTR szValueName,
				   LPCTSTR szValue,
				   __int32 Size
				   );
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
BOOL 
RegSetDWORD (
			 HKEY v_hRegKey,
			 LPCTSTR szValueName,
			 DWORD   dwValue
			 );

BOOL RegTestValueNameExist(HKEY hKey, PTSTR szValueName);
BOOL RegGetDWORD(HKEY hKey, PTSTR szValueName, PDWORD pdwValue);
BOOL RegGetStr(HKEY hKey, PTSTR szValueName, PTCHAR pdwValue, DWORD Size);
BOOL RegGetStrMulty(HKEY hKey, PTSTR szValueName, PTCHAR pdwValue, DWORD * Size);
BOOL RegGetSubKeyNames(HKEY hKey, PTSTR szValueName, DWORD MaxSymbolCount);
BOOL RegGetValuesNames(HKEY hKey, PTSTR szValueName, DWORD MaxSymbolCount);
BOOL RegDeleteSubKeysAndValues(HKEY hKey, 
							   TCHAR * TempArray, // array for all values or key names to recieve
							   DWORD TempArrayLen);

#endif //__C_REG_WORK_H__