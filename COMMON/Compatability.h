/*
this header is for compilator compatability
for example between borland and microsoft
*/

#ifndef __H_COMP_COMPAT__
#define __H_COMP_COMPAT__

#define WriteConstCharToArray(currArray, currString) memcpy(currArray, currString, (sizeof(currString)-1)) // -1 because \x0 final symbol

#ifdef _MSC_VER
#define sprintf_loc(buf,size,...) sprintf_s(buf,size,__VA_ARGS__)
#define strcpy_loc(dest,size,source) strcpy_s(dest,size,source)
#pragma warning( disable : 4996) // can't something do with sscanf
#define MIN(type,x,y)	min(x,y)
#define MAX(type,x,y)	max(x,y)
#else // BORLAND usual
#define strcpy_loc(dest,size,source) strcpy(dest,source)
#define sprintf_loc(buf,size,...) sprintf(buf,__VA_ARGS__)
#define MIN(type,x,y)	std::min<type>(x,y)
#define MAX(type,x,y)	std::max<type>(x,y)
#endif

// universal funcions for ansi and unicode, 
// comfortable in libraries wa - wide&ansi
#ifdef _UNICODE
#define	my_wa_strdup(x)					wcsdup(x)
#define	my_wa_strlen(x)					wcslen(x)
#define	my_wa_sprintf(x,format,...)		swprintf(x,format,__VA_ARGS__)
#define	my_wa_strcpy(x,y)				wcscpy(x,y)
#define	my_wa_strcpy_s(x,sizex,y)		wcscpy_s(x,sizex,y)
#define	my_wa_strcmp(x,y)				wcscmp(x,y)
#define	my_wa_vsprintf(x,format,arg)	vswprintf(x,format,arg)
#define	my_wa_vfprintf(file,format,arg)	vfwprintf(file,format,arg)

#define	my_wa_fopen(name,param)			_wfopen(name,param)
#define	my_wa_fprintf(file,format,...)	fwprintf(file,format,__VA_ARGS__)

#else 
#define	my_wa_strdup(x)					strdup(x)
#define	my_wa_strlen(x)					strlen(x)
#define	my_wa_sprintf(x,format,...)		sprintf(x,format,__VA_ARGS__)
#define	my_wa_strcpy(x,y)				strcpy(x,y)
#define	my_wa_strcpy_s(x,sizex,y)		strcpy_s(x,sizex,y)
#define	my_wa_strcmp(x,y)				strcmp(x,y)
#define	my_wa_vsprintf(x,format,arg)	vsprintf(x,format,arg)
#define	my_wa_vfprintf(file,format,arg)	vfprintf(file,format,arg)

#define	my_wa_fopen(name,param)			fopen(name,param)
#define	my_wa_fprintf(file,format,...)	fprintf(file,format,__VA_ARGS__)
#endif


#endif