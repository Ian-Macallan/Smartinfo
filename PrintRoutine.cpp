/*
 ************************************************************************************
 *
 *		File:			PrintRoutine.cpp
 *		Class:			
 *
 ************************************************************************************
 *		Description:
 *
 ************************************************************************************
 *		Notes:
 *
 ************************************************************************************
 *
 *		E/Mail: ian.macallan@gmail.com
 *		(c) 2003 - 2023 / Macallan 
 *		
 ************************************************************************************
 *
 */
#include "stdafx.h"

#include "PrintRoutine.h"

#include "GlobalVariables.h"

#include "PrintRoutine.h"

//
static HANDLE			hStdOut = INVALID_HANDLE_VALUE;
static HANDLE			hStdErr = INVALID_HANDLE_VALUE;

static CONSOLE_SCREEN_BUFFER_INFO	StdOutInfo;
static CONSOLE_SCREEN_BUFFER_INFO	StdErrInfo;
static CONSOLE_SCREEN_BUFFER_INFO	StdOutInfoCurrent;
static CONSOLE_SCREEN_BUFFER_INFO	StdErrInfoCurrent;

//
/////////////////////////////////////////////////////////////////////////////
//	Global
/////////////////////////////////////////////////////////////////////////////
_locale_t		LocalePointer				= NULL;

bool	LocaleMode							= false;
bool	VerboseMode							= false;
bool	QuietMode							= false;
bool	DebugMode							= false;
bool	TraceMode							= false;
bool	PerformanceMode						= false;

//
//====================================================================================
//
//====================================================================================
static BOOL lastWasCR	= FALSE;
static BOOL lastWasLF	= FALSE;

//
/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void InitStdHandlers ()
{
	//
	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	hStdErr = GetStdHandle(STD_ERROR_HANDLE);

	//
	ZeroMemory ( &StdOutInfo, sizeof(StdOutInfo) );
	ZeroMemory ( &StdErrInfo, sizeof(StdErrInfo) );

	//
	ZeroMemory ( &StdOutInfoCurrent, sizeof(StdOutInfoCurrent) );
	ZeroMemory ( &StdErrInfoCurrent, sizeof(StdErrInfoCurrent) );

	//
	if ( hStdOut != INVALID_HANDLE_VALUE )
	{
		BOOL bGet			=  GetConsoleScreenBufferInfo ( hStdOut, &StdOutInfo );
		StdOutInfoCurrent	= StdOutInfo;
	}

	if ( hStdErr != INVALID_HANDLE_VALUE )
	{
		BOOL bGet			=  GetConsoleScreenBufferInfo ( hStdErr, &StdErrInfo );
		StdErrInfoCurrent	= StdErrInfo;
	}
}

//
/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void SetStdoutIntensity ()
{
	if ( hStdOut != INVALID_HANDLE_VALUE )
	{
		WORD wAttribute = StdOutInfo.wAttributes | FOREGROUND_INTENSITY;
		SetConsoleTextAttribute ( hStdOut, wAttribute );
	}
}

//
/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
BOOL SetForeGroundZero ( bool bStdout )
{
	HANDLE hStd									= NULL;
	CONSOLE_SCREEN_BUFFER_INFO	*pConsoleInfo	= NULL;

	if ( bStdout && hStdOut != INVALID_HANDLE_VALUE )
	{
		hStd			= hStdOut;
		pConsoleInfo	= &StdOutInfoCurrent;
	}

	if ( ! bStdout && hStdErr != INVALID_HANDLE_VALUE )
	{
		hStd			= hStdErr;
		pConsoleInfo	= &StdErrInfoCurrent;
	}

	if ( pConsoleInfo != NULL && hStd != NULL )
	{
		pConsoleInfo->wAttributes	&= 0xFFF0;
		BOOL bResult				= SetConsoleTextAttribute ( hStd, pConsoleInfo->wAttributes );
		return bResult;
	}

	//
	return FALSE;
}

//
/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
BOOL SetForeGroundColor ( bool bStdout, WORD attribute, bool bErase )
{
	HANDLE hStd									= NULL;
	CONSOLE_SCREEN_BUFFER_INFO	*pConsoleInfo	= NULL;

	if ( bStdout && hStdOut != INVALID_HANDLE_VALUE )
	{
		hStd			= hStdOut;
		pConsoleInfo	= &StdOutInfoCurrent;
	}

	if ( ! bStdout && hStdErr != INVALID_HANDLE_VALUE )
	{
		hStd			= hStdErr;
		pConsoleInfo	= &StdErrInfoCurrent;
	}

	if ( pConsoleInfo != NULL && hStd != NULL )
	{
		if ( bErase )
		{
			SetForeGroundZero ( bStdout );
		}

		pConsoleInfo->wAttributes	|= attribute;
		BOOL bResult				= SetConsoleTextAttribute ( hStd, pConsoleInfo->wAttributes );
		return bResult;
	}

	//
	return FALSE;
}

//
/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
BOOL SetForeGroundIntensity ( bool bStdout, bool bErase )
{
	HANDLE hStd									= NULL;
	CONSOLE_SCREEN_BUFFER_INFO	*pConsoleInfo	= NULL;

	if ( bStdout && hStdOut != INVALID_HANDLE_VALUE )
	{
		hStd			= hStdOut;
		pConsoleInfo	= &StdOutInfoCurrent;
	}

	if ( ! bStdout && hStdErr != INVALID_HANDLE_VALUE )
	{
		hStd			= hStdErr;
		pConsoleInfo	= &StdErrInfoCurrent;
	}

	if ( pConsoleInfo != NULL && hStd != NULL )
	{
		if ( bErase )
		{
			SetForeGroundZero ( bStdout );
		}

		pConsoleInfo->wAttributes	|= FOREGROUND_INTENSITY;
		BOOL bResult				= SetConsoleTextAttribute ( hStd, pConsoleInfo->wAttributes );
		return bResult;
	}

	//
	return FALSE;
}

//
/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
BOOL SetBackGroundZero ( bool bStdout )
{
	HANDLE hStd									= NULL;
	CONSOLE_SCREEN_BUFFER_INFO	*pConsoleInfo	= NULL;

	if ( bStdout && hStdOut != INVALID_HANDLE_VALUE )
	{
		hStd			= hStdOut;
		pConsoleInfo	= &StdOutInfoCurrent;
	}

	if ( ! bStdout && hStdErr != INVALID_HANDLE_VALUE )
	{
		hStd			= hStdErr;
		pConsoleInfo	= &StdErrInfoCurrent;
	}

	if ( pConsoleInfo != NULL && hStd != NULL )
	{
		pConsoleInfo->wAttributes	&=	0xFF0F;
		BOOL bResult				= SetConsoleTextAttribute ( hStd, pConsoleInfo->wAttributes );
		return bResult;
	}

	//
	return FALSE;
}

//
/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
BOOL SetBackGroundColor ( bool bStdout, WORD attribute, bool bErase )
{
	HANDLE hStd									= NULL;
	CONSOLE_SCREEN_BUFFER_INFO	*pConsoleInfo	= NULL;

	if ( bStdout && hStdOut != INVALID_HANDLE_VALUE )
	{
		hStd			= hStdOut;
		pConsoleInfo	= &StdOutInfoCurrent;
	}

	if ( ! bStdout && hStdErr != INVALID_HANDLE_VALUE )
	{
		hStd			= hStdErr;
		pConsoleInfo	= &StdErrInfoCurrent;
	}

	if ( pConsoleInfo != NULL && hStd != NULL )
	{
		if ( bErase )
		{
			SetBackGroundZero ( bStdout );
		}

		pConsoleInfo->wAttributes	|=	attribute;
		BOOL bResult				= SetConsoleTextAttribute ( hStd, pConsoleInfo->wAttributes );
		return bResult;
	}

	//
	return FALSE;
}

//
/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
BOOL SetBackGroundIntensity ( bool bStdout, bool bErase )
{
	HANDLE hStd									= NULL;
	CONSOLE_SCREEN_BUFFER_INFO	*pConsoleInfo	= NULL;

	if ( bStdout && hStdOut != INVALID_HANDLE_VALUE )
	{
		hStd			= hStdOut;
		pConsoleInfo	= &StdOutInfoCurrent;
	}

	if ( ! bStdout && hStdErr != INVALID_HANDLE_VALUE )
	{
		hStd			= hStdErr;
		pConsoleInfo	= &StdErrInfoCurrent;
	}

	if ( pConsoleInfo != NULL && hStd != NULL )
	{
		if ( bErase )
		{
			SetBackGroundZero ( bStdout );
		}

		pConsoleInfo->wAttributes |= BACKGROUND_INTENSITY;
		BOOL bResult		= SetConsoleTextAttribute ( hStd, pConsoleInfo->wAttributes );
		return bResult;
	}

	//
	return FALSE;
}

//
/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
BOOL SetAllGroundRestore ( bool bStdout )
{
	HANDLE hStd									= NULL;
	CONSOLE_SCREEN_BUFFER_INFO	*pConsoleInfo	= NULL;

	if ( bStdout && hStdOut != INVALID_HANDLE_VALUE )
	{
		hStd			= hStdOut;
		pConsoleInfo	= &StdOutInfo;
	}

	if ( ! bStdout && hStdErr != INVALID_HANDLE_VALUE )
	{
		hStd			= hStdErr;
		pConsoleInfo	= &StdErrInfo;
	}

	if ( pConsoleInfo != NULL && hStd != NULL )
	{
		BOOL bResult		= SetConsoleTextAttribute ( hStd, pConsoleInfo->wAttributes );
		return bResult;
	}

	//
	return FALSE;
}

//
/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void SetStdoutRestore ()
{
	if ( hStdOut != INVALID_HANDLE_VALUE )
	{
		SetConsoleTextAttribute ( hStdOut, StdOutInfo.wAttributes );
	}
}

//
//====================================================================================
//
//====================================================================================
BOOL SetStdoutTextAttribute ( WORD wCurrentAttribute )
{
	if ( hStdOut != INVALID_HANDLE_VALUE )
	{
		return SetConsoleTextAttribute ( hStdOut, wCurrentAttribute );
	}

	return FALSE;
}

//
//====================================================================================
//
//====================================================================================
CONSOLE_SCREEN_BUFFER_INFO *GetStdoutScreenInfo()
{
	return &StdOutInfo;
}

//
//====================================================================================
//
//====================================================================================
void FlushInverse ( FILE *file )
{
	if ( file == stderr )
	{
		fflush ( stdout );
	}

	if ( file == stdout )
	{
		fflush ( stderr );
	}
}

//
//====================================================================================
//
//====================================================================================
void CheckFormatW ( const WCHAR *format )
{
	//
	if ( format != NULL )
	{
		if ( wcschr ( format , L'\n' ) != NULL )
		{
			lastWasLF = TRUE;
		}
		else
		{
			lastWasLF = FALSE;
		}

		if ( wcschr ( format , L'\r' ) != NULL )
		{
			lastWasCR = TRUE;
		}
		else
		{
			lastWasCR = FALSE;
		}
	}
}

//
//====================================================================================
//
//====================================================================================
void CheckFormatA ( const char *format )
{
	//
	if ( format != NULL )
	{
		if ( strchr ( format , '\n' ) != NULL )
		{
			lastWasLF = TRUE;
		}
		else
		{
			lastWasLF = FALSE;
		}

		if ( strchr ( format , '\r' ) != NULL )
		{
			lastWasCR = TRUE;
		}
		else
		{
			lastWasCR = FALSE;
		}
	}
}

//
//====================================================================================
//	Print va_list If Do Print or stderr
//	Base Routine
//====================================================================================
int PrintVW ( bool localize, bool bDoPrint, FILE *file, const WCHAR *format, va_list argptr )
{
	bool toPrint = false;

	FlushInverse ( file );

	if ( file == stderr )
	{
		toPrint = true;
	}
	else if ( file == stdout )
	{
		if ( bDoPrint )
		{
			toPrint = true;
		}
		else
		{
			toPrint = false;
		}
	}

	int result = 0;
	if ( toPrint ) 
	{
		if ( ( localize ) && ( LocalePointer != NULL ) && ( ( file == stdout ) || ( file == stderr ) ) )
		{
			result = _vfwprintf_l ( file, format, LocalePointer, argptr );
		}
		else
		{
			result = vfwprintf ( file, format, argptr );
		}
	}

	return result;
}

//
//====================================================================================
//	Print va_list
//	Base Routine
//====================================================================================
int PrintVW ( FILE *file, const WCHAR *format, va_list argptr )
{
	FlushInverse ( file );

	int result = 0;
	if ( ( LocaleMode ) && ( LocalePointer != NULL ) && ( ( file == stdout ) || ( file == stderr ) ) )
	{
		result = _vfwprintf_l ( file, format, LocalePointer, argptr );
	}
	else
	{
		result = vfwprintf ( file, format, argptr );
	}

	return result;
}

//
//====================================================================================
//	Print va_list
//====================================================================================
int PrintVW ( const WCHAR *format, va_list argptr )
{
	return PrintVW ( stdout, format, argptr );
}

//
//====================================================================================
//	Print va_list If Do Print or stderr
//	Base Routine
//====================================================================================
int PrintVA ( bool localize, bool bDoPrint, FILE *file, const char *format, va_list argptr )
{
	bool toPrint = false;

	FlushInverse ( file );

	if ( file == stderr )
	{
		toPrint = true;
	}
	else if ( file == stdout )
	{
		if ( bDoPrint )
		{
			toPrint = true;
		}
		else
		{
			toPrint = false;
		}
	}

	int result = 0;
	if ( toPrint ) 
	{
		if ( ( localize ) && ( LocalePointer != NULL ) && ( ( file == stdout ) || ( file == stderr ) ) )
		{
			result = _vfprintf_l ( file, format, LocalePointer, argptr );
		}
		else
		{
			result = vfprintf ( file, format, argptr );
		}
	}

	return result;
}

//
//====================================================================================
//	Print va_list
//	Base Routine
//====================================================================================
int PrintVA ( FILE *file, const char *format, va_list argptr )
{
	FlushInverse ( file );

	int result = 0;
	if ( ( LocaleMode ) && ( LocalePointer != NULL ) && ( ( file == stdout ) || ( file == stderr ) ) )
	{
		result = _vfprintf_l ( file, format, LocalePointer, argptr );
	}
	else
	{
		result = vfprintf ( file, format, argptr );
	}

	return result;
}

//
//====================================================================================
//	Print va_list to stdout
//====================================================================================
int PrintVA ( const char *format, va_list argptr )
{
	return PrintVA ( stdout, format, argptr );
}

//
//====================================================================================
//	Print ... If Do Print or stderr
//====================================================================================
int PrintW ( bool localize, bool bDoPrint, FILE *file, const WCHAR *format, ... )
{
	va_list argptr;
	va_start(argptr, format);
	int result = PrintVW ( localize, bDoPrint, file, format, argptr );
	va_end(argptr);
	return result;
}

//
//====================================================================================
//	Print ... If Do Print or stderr
//====================================================================================
int PrintA ( bool localize, bool bDoPrint, FILE *file, const char *format, ... )
{
	va_list argptr;
	va_start(argptr, format);
	int result = PrintVA ( localize, bDoPrint, file, format, argptr );
	va_end(argptr);
	return result;
}

//
//====================================================================================
//	Print ... If Verbose or stderr
//====================================================================================
int PrintStdW ( FILE *file, const WCHAR *format, ... )
{
	va_list argptr;
	va_start(argptr, format);
	int result = PrintVW ( LocaleMode, VerboseMode, file, format, argptr );
	va_end(argptr);
	return result;
}

//
//====================================================================================
//	Print ... If Verbose or stderr
//====================================================================================
int PrintStdA ( FILE *file, const char *format, ... )
{
	va_list argptr;
	va_start(argptr, format);
	int result = PrintVA ( LocaleMode, VerboseMode, file, format, argptr );
	va_end(argptr);
	return result;
}

//
//====================================================================================
//	Print ... If Verbose
//====================================================================================
int PrintStdoutW ( const WCHAR *format, ... )
{
	va_list argptr;
	va_start(argptr, format);
	int result = PrintVW ( LocaleMode, VerboseMode, stdout, format, argptr );
	va_end(argptr);
	return result;
}

//
//====================================================================================
//	Print ... If Verbose
//====================================================================================
int PrintStdoutA ( const char *format, ... )
{
	va_list argptr;
	va_start(argptr, format);
	int result = PrintVA ( LocaleMode, VerboseMode, stdout, format, argptr );
	va_end(argptr);
	return result;
}

//
//====================================================================================
//	Print ... If Verbose
//====================================================================================
int PrintVerboseW ( const WCHAR *format, ... )
{
	if ( VerboseMode )
	{
		va_list argptr;
		va_start(argptr, format);
		int result = PrintVW ( LocaleMode, true, stderr, format, argptr );
		va_end(argptr);
		return result;
	}

	return 0;
}

//
//====================================================================================
//	Print ... If Verbose
//====================================================================================
int PrintVerboseA ( const char *format, ... )
{
	if ( VerboseMode )
	{
		va_list argptr;
		va_start(argptr, format);
		int result = PrintVA ( LocaleMode, true, stderr, format, argptr );
		va_end(argptr);
		return result;
	}

	return 0;
}

//
//====================================================================================
//	Print ... To Stdout
//====================================================================================
int PrintDirectW ( const WCHAR *format, ... )
{
	va_list argptr;
	va_start(argptr, format);
	int result = PrintVW ( LocaleMode, true, stdout, format, argptr );
	va_end(argptr);
	return result;
}

//
//====================================================================================
//	Print ... To Stdout
//====================================================================================
int PrintDirectA ( const char *format, ... )
{
	va_list argptr;
	va_start(argptr, format);
	int result = PrintVA ( LocaleMode, true, stdout, format, argptr );
	va_end(argptr);
	return result;
}

//
//====================================================================================
//	Print ... If Not Quiet
//====================================================================================
int PrintNormalW ( const WCHAR *format, ... )
{
	if ( ! QuietMode )
	{
		va_list argptr;
		va_start(argptr, format);
		int result = PrintVW ( LocaleMode, true, stdout, format, argptr );
		va_end(argptr);
		return result;
	}

	return 0;
}

//
//====================================================================================
//	Print ... If Not Quiet
//====================================================================================
int PrintNormalA ( const char *format, ... )
{
	if ( ! QuietMode )
	{
		va_list argptr;
		va_start(argptr, format);
		int result = PrintVA ( LocaleMode, true, stdout, format, argptr );
		va_end(argptr);
		return result;
	}

	return 0;
}

//
//====================================================================================
//	Print ... If Not Quiet
//====================================================================================
int PrintStandardW ( const WCHAR *format, ... )
{
	if ( ! QuietMode )
	{
		va_list argptr;
		va_start(argptr, format);
		int result = PrintVW ( LocaleMode, true, stdout, format, argptr );
		va_end(argptr);
		return result;
	}

	return 0;
}

//
//====================================================================================
//	Print ... If Not Quiet
//====================================================================================
int PrintStandardA ( const char *format, ... )
{
	if ( ! QuietMode )
	{
		va_list argptr;
		va_start(argptr, format);
		int result = PrintVA ( LocaleMode, true, stdout, format, argptr );
		va_end(argptr);
		return result;
	}

	return 0;
}

//
//====================================================================================
//	Print ... If Verbose
//====================================================================================
int PrintStdoutLocaleW ( const WCHAR *format, ... )
{
	va_list argptr;
	va_start(argptr, format);
	int result = PrintVW ( true, VerboseMode, stdout, format, argptr );
	va_end(argptr);
	return result;
}

//
//====================================================================================
//	Print ... If Verbose
//====================================================================================
int PrintStdoutLocaleA ( const char *format, ... )
{
	va_list argptr;
	va_start(argptr, format);
	int result = PrintVA ( true, VerboseMode, stdout, format, argptr );
	va_end(argptr);
	return result;
}

//
//====================================================================================
//	Print ... anyway
//====================================================================================
int PrintStderrW ( const WCHAR *format, ... )
{
	va_list argptr;
	va_start(argptr, format);
	int result = PrintVW ( LocaleMode, VerboseMode, stderr, format, argptr );
	va_end(argptr);
	return result;
}

//
//====================================================================================
//	Print ... anyway
//====================================================================================
int PrintStderrA ( const char *format, ... )
{
	va_list argptr;
	va_start(argptr, format);
	int result = PrintVA ( LocaleMode, VerboseMode, stderr, format, argptr );
	va_end(argptr);
	return result;
}

//
//====================================================================================
//	Print ... Only If Debug
//====================================================================================
int PrintDebugW ( const WCHAR *format, ... )
{
	if ( DebugMode )
	{
		va_list argptr;
		va_start(argptr, format);
		int result = PrintVW ( LocaleMode, true, stderr, format, argptr );
		va_end(argptr);
		return result;
	}

	return 0;
}

//
//====================================================================================
//	Print ... Only If Debug
//====================================================================================
int PrintDebugA ( const char *format, ... )
{
	if ( DebugMode )
	{
		va_list argptr;
		va_start(argptr, format);
		int result = PrintVA ( LocaleMode, true, stderr, format, argptr );
		va_end(argptr);
		return result;
	}

	return 0;
}

//
//====================================================================================
//	Print ... Only If Debug
//====================================================================================
int PrintDebugFLW ( const char *filename, int linenumber, const WCHAR *format, ... )
{
	if ( DebugMode )
	{
		PrintA ( LocaleMode, true, stderr, "%s:%d - ", filename, linenumber );
		va_list argptr;
		va_start(argptr, format);
		int result = PrintVW ( LocaleMode, true, stderr, format, argptr );
		va_end(argptr);
		return result;
	}

	return 0;
}

//
//====================================================================================
//	Print ... Only If Debug
//====================================================================================
int PrintDebugFLA ( const char *filename, int linenumber, const char *format, ... )
{
	if ( DebugMode )
	{
		PrintA ( LocaleMode, true, stderr, "%s:%d ", filename, linenumber );
		va_list argptr;
		va_start(argptr, format);
		int result = PrintVA ( LocaleMode, true, stderr, format, argptr );
		va_end(argptr);
		return result;
	}

	return 0;
}

//
//====================================================================================
//	Print ... Only If Performance
//====================================================================================
int PrintPerformanceW ( const WCHAR *format, ... )
{
	if ( PerformanceMode )
	{
		va_list argptr;
		va_start(argptr, format);
		int result = PrintVW ( LocaleMode, true, stderr, format, argptr );
		va_end(argptr);
		return result;
	}

	return 0;
}

//
//====================================================================================
//	Print ... Only If Performance
//====================================================================================
int PrintPerformanceA ( const char *format, ... )
{
	if ( PerformanceMode )
	{
		va_list argptr;
		va_start(argptr, format);
		int result = PrintVA ( LocaleMode, true, stderr, format, argptr );
		va_end(argptr);
		return result;
	}

	return 0;
}

//
//====================================================================================
//	Print ... Only If Trace
//====================================================================================
int PrintTraceW ( const WCHAR *format, ... )
{
	if ( TraceMode )
	{
		va_list argptr;
		va_start(argptr, format);
		int result = PrintVW ( LocaleMode, true, stderr, format, argptr );
		va_end(argptr);
		return result;
	}

	return 0;
}

//
//====================================================================================
//	Print ... Only If Trace
//====================================================================================
int PrintTraceA ( const char *format, ... )
{
	if ( TraceMode )
	{
		va_list argptr;
		va_start(argptr, format);
		int result = PrintVA ( LocaleMode, true, stderr, format, argptr );
		va_end(argptr);
		return result;
	}

	return 0;
}

//
//====================================================================================
//	Print
//====================================================================================
int PrintHelpLineW ( const WCHAR *format, ... )
{
	va_list argptr;
	va_start(argptr, format);
	int result = PrintVW ( LocaleMode, true, stdout, format, argptr );
	va_end(argptr);
	return result;
}

//
//====================================================================================
//	Print ... Only If Trace
//====================================================================================
int PrintHelpLineA ( const char *format, ... )
{
	va_list argptr;
	va_start(argptr, format);
	int result = PrintVA ( LocaleMode, true, stdout, format, argptr );
	va_end(argptr);
	return result;
}

//
//====================================================================================
//	Print Help Line
//====================================================================================
int PrintHelpLineW ( int iWidth, const WCHAR *pOption, const WCHAR *pText )
{
	PrintHelpLineW ( L"%*s : %s\n", iWidth, pOption, pText );

	return 0;
}

//
//====================================================================================
//	Print Help Line
//====================================================================================
int PrintHelpLineA ( int iWidth, const char *pOption, const char *pText )
{
	PrintHelpLineA ( "%*s : %s\n", iWidth, pOption, pText );

	return 0;
}

//
//====================================================================================
//	Print Help Line
//====================================================================================
int PrintHelpLineW ( int iWidth, const WCHAR *pOption, const WCHAR *pText1, const WCHAR *pText2 )
{
	if ( wcslen(pText2) == 1 )
	{
		PrintHelpLineW ( L"%*s : %s \"%s\"\n", iWidth, pOption, pText1, pText2 );
	}
	else
	{
		PrintHelpLineW ( L"%*s : %s %s\n", iWidth, pOption, pText1, pText2 );
	}

	return 0;
}

//
//====================================================================================
//	Print Help Line
//====================================================================================
int PrintHelpLineA ( int iWidth, const char *pOption, const char *pText1, const char *pText2 )
{
	if ( strlen(pText2) == 1 )
	{
		PrintHelpLineA ( "%*s : %s \"%s\"\n", iWidth, pOption, pText1, pText2 );
	}
	else
	{
		PrintHelpLineA ( "%*s : %s %s\n", iWidth, pOption, pText1, pText2 );
	}

	return 0;
}

//
//====================================================================================
//	Print Help Line
//====================================================================================
int PrintHelpLineW ( )
{
	PrintHelpLineW ( L"\n" );

	return 0;
}

//
//====================================================================================
//	Print Help Line
//====================================================================================
int PrintHelpLineA ( )
{
	PrintHelpLineA ( "\n" );

	return 0;
}

//
//====================================================================================
//
//====================================================================================
BOOL CreateLocalePointerW ( const WCHAR *pLocaleString, int lcValue )
{
	//
	if ( pLocaleString != NULL && wcslen(pLocaleString) > 0 )
	{
		LocalePointer = _wcreate_locale ( LC_ALL, pLocaleString);
		if ( LocalePointer != NULL )
		{
			if ( lcValue >= 0 )
			{
				_wsetlocale(lcValue, pLocaleString);
			}
			LocaleMode = true;
			return TRUE;
		}
	}

	LocaleMode = false;
	return FALSE;
}

//
//====================================================================================
//
//====================================================================================
BOOL CreateLocalePointerA ( char *pLocaleString, int lcValue )
{
	//
	if ( pLocaleString != NULL && strlen(pLocaleString) > 0 )
	{
		LocalePointer = _create_locale ( LC_ALL, pLocaleString);
		if ( LocalePointer != NULL )
		{
			if ( lcValue >= 0 )
			{
				setlocale(lcValue, pLocaleString);
			}
			LocaleMode = true;
			return TRUE;
		}
	}

	LocaleMode = false;
	return FALSE;
}