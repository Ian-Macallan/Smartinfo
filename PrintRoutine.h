/*
 ************************************************************************************
 *
 *		File:			PrintRoutine.h
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
#pragma once

#include <Windows.h>
#include <locale.h>
#include <stdarg.h>

//
//////////////////////////////////////////////////////////////////////////////
//	Colors
//////////////////////////////////////////////////////////////////////////////
#ifndef FOREGROUND_YELLOW
#define	FOREGROUND_YELLOW		FOREGROUND_RED|FOREGROUND_GREEN
#endif // !FOREGROUND_YELLOW

#ifndef FOREGROUND_CYAN
#define	FOREGROUND_CYAN			FOREGROUND_GREEN|FOREGROUND_BLUE
#endif // !FOREGROUND_CYAN

#ifndef FOREGROUND_MAGENTA
#define	FOREGROUND_MAGENTA		FOREGROUND_RED|FOREGROUND_BLUE
#endif // !FOREGROUND_MAGENTA

#ifndef FOREGROUND_WHITE
#define	FOREGROUND_WHITE		FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE
#endif // !FOREGROUND_WHITE

//
#ifndef BACKGROUND_YELLOW
#define	BACKGROUND_YELLOW		BACKGROUND_RED|BACKGROUND_GREEN
#endif // !BACKGROUND_YELLOW

#ifndef BACKGROUND_CYAN
#define	BACKGROUND_CYAN			BACKGROUND_GREEN|BACKGROUND_BLUE
#endif // !BACKGROUND_CYAN

#ifndef BACKGROUND_MAGENTA
#define	BACKGROUND_MAGENTA		BACKGROUND_RED|BACKGROUND_BLUE
#endif // !BACKGROUND_MAGENTA

#ifndef BACKGROUND_WHITE
#define	BACKGROUND_WHITE		BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE
#endif // !BACKGROUND_WHITE

//
//	Intensity
#ifndef FOREGROUND_HRED
#define	FOREGROUND_HRED			FOREGROUND_RED|FOREGROUND_INTENSITY
#endif // !FOREGROUND_HRED

//
#ifndef FOREGROUND_HGREEN
#define	FOREGROUND_HGREEN		FOREGROUND_GREEN|FOREGROUND_INTENSITY
#endif // !FOREGROUND_HGREEN

//
#ifndef FOREGROUND_HBLUE
#define	FOREGROUND_HBLUE		FOREGROUND_BLUE|FOREGROUND_INTENSITY
#endif // !FOREGROUND_HBLUE

//
#ifndef BACKGROUND_HRED
#define	BACKGROUND_HRED			BACKGROUND_RED|BACKGROUND_INTENSITY
#endif // !BACKGROUND_HRED

//
#ifndef BACKGROUND_HGREEN
#define	BACKGROUND_HGREEN		BACKGROUND_GREEN|BACKGROUND_INTENSITY
#endif // !BACKGROUND_HGREEN

//
#ifndef BACKGROUND_HBLUE
#define	BACKGROUND_HBLUE		BACKGROUND_BLUE|BACKGROUND_INTENSITY
#endif // !BACKGROUND_HBLUE

//
//	Intensity C/M/Y
#ifndef FOREGROUND_HYELLOW
#define	FOREGROUND_HYELLOW		FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_INTENSITY
#endif // !FOREGROUND_HYELLOW

#ifndef FOREGROUND_HCYAN
#define	FOREGROUND_HCYAN		FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY
#endif // !FOREGROUND_HCYAN

#ifndef FOREGROUND_HMAGENTA
#define	FOREGROUND_HMAGENTA		FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY
#endif // !FOREGROUND_HMAGENTA

#ifndef FOREGROUND_HWHITE
#define	FOREGROUND_HWHITE		FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY
#endif // !FOREGROUND_HWHITE

//	Background
#ifndef BACKGROUND_HYELLOW
#define	BACKGROUND_HYELLOW		BACKGROUND_RED|BACKGROUND_GREEN|FOREGROUND_INTENSITY
#endif // !BACKGROUND_HYELLOW

#ifndef BACKGROUND_HCYAN
#define	BACKGROUND_HCYAN		BACKGROUND_GREEN|BACKGROUND_BLUE|FOREGROUND_INTENSITY
#endif // !BACKGROUND_CYAN

#ifndef BACKGROUND_HMAGENTA
#define	BACKGROUND_HMAGENTA		BACKGROUND_RED|BACKGROUND_BLUE|FOREGROUND_INTENSITY
#endif // !BACKGROUND_MAGENTA

#ifndef BACKGROUND_HWHITE
#define	BACKGROUND_HWHITE		BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE|FOREGROUND_INTENSITY
#endif // !BACKGROUND_WHITE


//
//====================================================================================
//
//====================================================================================
#ifdef UNICODE

//	Print ... If Verbose or stderr
#define	PrintStd					PrintStdW

//	Print ... If Verbose
#define	PrintStdout					PrintStdoutW

//	Print ... anyway
#define	PrintStderr					PrintStderrW


//	Print anyway to stdout
#define	PrintDirect					PrintDirectW

//	Print ... If Not Quiet
#define	PrintNormal					PrintNormalW

//	Print ... If Not Quiet
#define	PrintStandard				PrintStandardW

//	Print ... If Verbose
#define	PrintVerbose				PrintVerboseW

//	Print ... If Verbose
#define	PrintStdoutLocale			PrintStdoutLocaleW

//	Print ... If Debug
#define	PrintDebug					PrintDebugW

//	Print ... If Debug
#define	PrintDebugFL				PrintDebugFLW

//	Print ... If Performance
#define	PrintPerformance			PrintPerformanceW

//	Print ... If Trace
#define	PrintTrace					PrintTraceW

//	Print Help Line width, option, text
#define	PrintHelpLine				PrintHelpLineW

#define	CreateLocalePointer			CreateLocalePointerW

//	MBCS
#else

//	Print ... If Verbose or stderr
#define	PrintStd					PrintStdA

//	Print ... If Verbose
#define	PrintStdout					PrintStdoutA

//	Print anyway
#define	PrintStderr					PrintStderrA

//	Print anyway to stdout
#define	PrintDirect					PrintDirectA

//	Print ... If Not Quiet
#define	PrintNormal					PrintNormalA

//	Print ... If Not Quiet
#define	PrintStandard				PrintStandardA

//	Print ... If Verbose
#define	PrintVerbose				PrintVerboseA

//	Print ... If Verbose
#define	PrintStdoutLocale			PrintStdoutLocaleA

//	Print ... If Performance
#define	PrintPerformance			PrintPerformanceA

//	Print ... If Debug
#define	PrintDebug					PrintDebugA

//	Print ... If Debug
#define	PrintDebugFL				PrintDebugFLA

//	Print ... If Trace
#define	PrintTrace					PrintTraceA

//	Print Help Line width, option, text
#define	PrintHelpLine				PrintHelpLineA

#define	CreateLocalePointer			CreateLocalePointerA
#endif

extern void InitStdHandlers ();
extern void SetStdoutIntensity ();

//
extern BOOL SetForeGroundZero ( bool bStdout = true );
extern BOOL SetForeGroundColor ( bool bStdout = true, WORD attribute = 0, bool bErase = false );
extern BOOL SetForeGroundIntensity ( bool bStdout = true, bool bErase = false );

#define	SetForeGroundRed(x,y)		SetForeGroundColor(x,FOREGROUND_RED,y);
#define	SetForeGroundGreen(x,y)		SetForeGroundColor(x,FOREGROUND_GREEN,y);
#define	SetForeGroundBlue(x,y)		SetForeGroundColor(x,FOREGROUND_BLUE,y);

extern BOOL SetBackGroundZero ( bool bStdout = true );
extern BOOL SetBackGroundColor ( bool bStdout = true, WORD attribute = 0, bool bErase = false );
extern BOOL SetBackGroundIntensity ( bool bStdout = true, bool bErase = false );

#define	SetBackGroundRed(x,y)		SetBackGroundColor(x,FOREGROUND_RED,y);
#define	SetBackGroundGreen(x,y)		SetBackGroundColor(x,FOREGROUND_GREEN,y);
#define	SetBackGroundBlue(x,y)		SetBackGroundColor(x,FOREGROUND_BLUE,y);

extern BOOL SetAllGroundRestore ( bool bStdout = true );

//
extern void SetStdoutRestore ();
extern BOOL SetStdoutTextAttribute ( WORD wCurrentAttribute );
extern CONSOLE_SCREEN_BUFFER_INFO *GetStdoutScreenInfo();

//	Print va_list If Do Print or stderr
//	Base Routine
extern int PrintVW ( bool localize, bool verbose, FILE *file, const WCHAR *format, va_list argptr );

//	Print va_list If Do Print or stderr
//	Base Routine
extern int PrintVA ( bool localize, bool verbose, FILE *file, const char *format, va_list argptr );

//	Print va_list 
//	Base Routine
extern int PrintVW ( FILE *file, const WCHAR *format, va_list argptr );

//	Print va_list 
//	Base Routine
extern int PrintVA ( FILE *file, const char *format, va_list argptr );

//	Print va_list to stdout
extern int PrintVW ( const WCHAR *format, va_list argptr );

//	Print va_list to stdout
extern int PrintVA ( const char *format, va_list argptr );

//	Print ... If Do Print or stderr
extern int PrintW ( bool localize, bool verbose, FILE *file, const WCHAR *format, ... );

//	Print ... If Do Print or stderr
extern int PrintA ( bool localize, bool verbose, FILE *file, const char *format, ... );

//	Print ... If Verbose or stderr
extern int PrintStdW ( FILE *file, const WCHAR *format, ... );

//	Print ... If Verbose or stderr
extern int PrintStdA ( FILE *file, const char *format, ... );

//	Print ... If Verbose
extern int PrintStdoutW ( const WCHAR *format, ... );

//	Print ... If Verbose
extern int PrintStdoutA ( const char *format, ... );

//	Print ... If Verbose
extern int PrintVerboseW ( const WCHAR *format, ... );

//	Print ... If Verbose
extern int PrintVerboseA ( const char *format, ... );

//	Print To Stdout
extern int PrintDirectW ( const WCHAR *format, ... );

//	Print To Stdout
extern int PrintDirectA ( const char *format, ... );

//	Print ... If Not Quiet
extern int PrintNormalW ( const WCHAR *format, ... );

//	Print ... If Not Quiet
extern int PrintNormalA ( const char *format, ... );

//	Print ... If Not Quiet
extern int PrintStandardW ( const WCHAR *format, ... );

//	Print ... If Not Quiet
extern int PrintStandardA ( const char *format, ... );

//	Print ... If Verbose
extern int PrintStdoutLocaleW ( const WCHAR *format, ... );

//	Print ... If Verbose
extern int PrintStdoutLocaleA ( const char *format, ... );

//	Print ... anyway
extern int PrintStderrW ( const WCHAR *format, ... );

//	Print ... anyway
extern int PrintStderrA ( const char *format, ... );

//	Print ... Only If Debug
extern int PrintDebugW ( const WCHAR *format, ... );

//	Print ... Only If Debug
extern int PrintDebugA ( const char *format, ... );

//	Print ... Only If Performance
extern int PrintPerformanceW ( const WCHAR *format, ... );

//	Print ... Only If Performance
extern int PrintPerformanceA ( const char *format, ... );

//	Print ... Only If Debug
extern int PrintDebugFLW ( const char *filename, int linenumber, const WCHAR *format, ... );

//	Print ... Only If Debug
extern int PrintDebugFLA ( const char *filename, int linenumber, const char *format, ... );

//	Print ... Only If Trace
extern int PrintTraceW ( const WCHAR *format, ... );

//	Print ... Only If Trace
extern int PrintTraceA ( const char *format, ... );

//	Print Help Line
extern int PrintHelpLineW ( const WCHAR *format, ... );
extern int PrintHelpLineW ( int iWidth, const WCHAR *pOption, const WCHAR *pText );
extern int PrintHelpLineW ( int iWidth, const WCHAR *pOption, const WCHAR *pText1, const WCHAR *pText2 );
extern int PrintHelpLineW ( );

//	Print Help Line
extern int PrintHelpLineA ( const char *format, ... );
extern int PrintHelpLineA ( int iWidth, const char *pOption, const char *pText );
extern int PrintHelpLineA ( int iWidth, const char *pOption, const char *pText1, const char *pText2 );
extern int PrintHelpLineA ( );


//	Create Locale Pointer and set it as LC_ALL
extern BOOL CreateLocalePointerW ( const WCHAR *pLocaleString, int lcValue = LC_ALL );

//	Create Locale Pointer and set it as LC_ALL
extern BOOL CreateLocalePointerA ( const char *pLocaleString, int lcValue = LC_ALL );

//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
extern	_locale_t		LocalePointer;

extern	bool			LocaleMode;
extern	bool			VerboseMode;
extern	bool			QuietMode;
extern	bool			DebugMode;
extern	bool			TraceMode;
extern	bool			PerformanceMode;


