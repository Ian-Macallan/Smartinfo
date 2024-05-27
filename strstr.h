/*
 ************************************************************************************
 *
 *		File:			strstr.h
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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <WinNls.h>

//
extern	DWORD		ConsoleEventSignal;
extern	bool		ConsoleEventAborted;

//
extern	bool		WaitProcessMode;
extern	bool		RunAsAdminMode;
extern	bool		ReadMeTextMode;

//
#ifndef	_wsizeof
#define	_wsizeof(x)				(sizeof(x)/sizeof(WCHAR))
#endif

#ifndef	_wnameandsizeof
#define	_wnameandsizeof(x)		x,(sizeof(x)/sizeof(WCHAR))
#endif

#ifndef	_wnonameandsizeof
#define	_wnonameandsizeof()		NULL,0
#endif

#ifndef	_wsizeofandname
#define	_wsizeofandname(x)		(sizeof(x)/sizeof(WCHAR)),x
#endif

//
typedef int				SizeInBytes;
typedef int				SizeInChars;

//
//====================================================================================
//	UNICODE
//====================================================================================
#ifdef UNICODE
//	Search String Ignore Case
#define	__strstri				__strstriW
//	Search String With Case
#define	__strstr				__strstrW
//	Replace String Ignore Case
#define	__strrepi				__strrepiW
//	Replace String Ignore Case if it is at position 0
#define	__strrepxi				__strrepxiW
//	Replace String With Case
#define	__strrep				__strrepW
//	Replace String With Case if it is at position 0
#define	__strrepx				__strrepxW
//	Search an array of char in String
#define	_wcschrs				_wcschrsW
//	Compare String Ignore Case limited to length of substring
#define	__wcsnicmp1				__wcsnicmp1W
//	Compare String Ignore Case limited to length with an array of substring
#define	__wcsnicmpL				__wcsnicmpLW
//	Compare String Ignore Case with an array of substring
#define	__wcsicmpL				__wcsicmpLW
//	Replace Char
#define	ReplaceChar				ReplaceCharW
//	Get Argument
#define	GetArgument				GetArgumentW
//	Remove File Type
#define	RemoveFileType			RemoveFileTypeW
//	Remove File Name
#define	RemoveFileName			RemoveFileNameW
//	Remove Trailing CR/LF
#define	RemoveCRLF				RemoveCRLFW
//	Find File Type Ex : .txt
#define	FindFileType			FindFileTypeW
//	Find File Type Ex : txt
#define	GetFileType				GetFileTypeW
//	Find File Name
#define	FindFileName			FindFileNameW
//	Ends With
#define	EndsWithI				EndsWithIW
//	Ends With
#define	EndsWith				EndsWithW
//	Start With
#define	StartsWithI				StartsWithIW
//	Ends With
#define	StartsWith				StartsWithW
//	Remove Enclosing
#define	RemoveEnclosing			RemoveEnclosingW
//	Remove Enclosing Quote
#define	RemoveEnclosingDQuote	RemoveEnclosingDQuoteW
//	Remove Enclosing Quote
#define	RemoveEnclosingSQuote	RemoveEnclosingSQuoteW
//	Remove Enclosing Quote
#define	RemoveEnclosingQuote	RemoveEnclosingQuoteW
//	Remove one Leading byte
#define	RemoveLeadingByte		RemoveLeadingByteW
//	Remove all Leading byte
#define	RemoveLeadingBytes		RemoveLeadingBytesW
//	Remove one Trailing byte
#define	RemoveTrailingByte		RemoveTrailingByteW
//	Remove all Trailing byte
#define	RemoveTrailingBytes		RemoveTrailingBytesW
//	Capitalize
#define	CapitalizeString		CapitalizeStringW
//	Lower Case
#define	LowerCaseString			LowerCaseStringW
//	Upper Case
#define	UpperCaseString			UpperCaseStringW

//	Alphabetic
#define	IsAlphaBetic			IsAlphaBeticW
//	Numeric
#define	IsNumeric				IsNumericW
//	Numeric
#define	IsNumericSigned			IsNumericSignedW
//	Number of numeric
#define	InRangeLength			InRangeLengthW
//	Number of not numeric
#define	NotInRangeLength		NotInRangeLengthW
//	Next Not In
#define	NextNotIn				NextNotInW
//	Next Not In Range
#define	NextNotInRange			NextNotInRangeW
//	Next In
#define	NextIn					NextInW
//	Next In Range
#define	NextInRange				NextInRangeW

//	Is Floating
#define IsFloating				IsFloatingW
//	Is Decimal Or Integer
#define IsDecimal				IsDecimalW

//	Human Readable
#define	HumanReadable			HumanReadableW

//	Seaprated Numbers
#define	SeparatedNumber			SeparatedNumberW

//	Try Convert From UTF8 to ANSI (CP_ACP)
#define	TryConvertUtf8ToAnsi	Try5ConvertUtf8ToAnsiW

//	Check BOM Return the ccs : UTF-8 UTF-16LE UNICODE UTF-32LE UTF-32BE or empty or ERROR
#define	CheckBOMValue			CheckBOMValueW

//	BOM Directive
#define	AddBOMDirective			AddBOMDirectiveW
//	Build Read Directive for open
#define	GetReadDirective		GetReadDirectiveW
//	Build Write Directive for open
#define	GetWriteDirective		GetWriteDirectiveW
//	Build Append Directive for open
#define	GetAppendDirective		GetAppendDirectiveW

//	Open File : mode can be also w8 w16 wu r8 r16 ru a8 a16 au
#define	OpenFileCcs				OpenFileCcsW

//	Check File Exist
#define	CheckFileExist			CheckFileExistW

//	Check Path Exist
#define	CheckPathExist			CheckPathExistW

//	Run As Admin
#define	RunAsAdmin				RunAsAdminW

//	Treat Parameters
#define	TreatLaunchOptions		TreatLaunchOptionsW

//	Print Help
#define	PrintHelpLaunch			PrintHelpLaunchW

//	Read Me Option
#define	TreatReadMeOption		TreatReadMeOptionW

//	Print Help
#define	PrintHelpReadMe			PrintHelpReadMeW

//	Print Real Version
#define	PrintRealVersion		PrintRealVersionW

//
//====================================================================================
//	MBCS
//====================================================================================
#else
//	Search String Ignore Case
#define	__strstri				__strstriA
//	Search String With Case
#define	__strstr				__strstrA
//	Replace String Ignore Case
#define	__strrepi				__strrepiA
//	Replace String Ignore Case if it is at position 0
#define	__strrepxi				__strrepxiA
//	Replace String With Case
#define	__strrep				__strrepA
//	Replace String With Case if it is at position 0
#define	__strrepx				__strrepxA
//	Search an array of char in String
#define	_strchrs				_strchrsA
//	Compare String Ignore Case limited to length of substring
#define	__strnicmp1				__strnicmp1A
//	Compare String Ignore Case limited to length with an array of substring
#define	__strnicmpL				__strnicmpLA
//	Compare String Ignore Case with an array of substring
#define	__stricmpL				__stricmpLA
//	Replace Char
#define	ReplaceChar				ReplaceCharA
//	Get Argument
#define	GetArgument				GetArgumentA
//	Remove File Type
#define	RemoveFileType			RemoveFileTypeA
//	Remove File Name
#define	RemoveFileName			RemoveFileNameA
//	Remove Trailing CR/LF
#define	RemoveCRLF				RemoveCRLFA
//	Find File Type Ex : .txt
#define	FindFileType			FindFileTypeA
//	Get File Type without . Ex : txt
#define	GetFileType				GetFileTypeA
//	Find File Name
#define	FindFileName			FindFileNameA
//	Ends With
#define	EndsWithI				EndsWithIA
//	Ends With
#define	EndsWith				EndsWithA
//	Start With
#define	StartsWithI				StartsWithIA
//	Ends With
#define	StartsWith				StartsWithA
//	Remove Enclosing
#define	RemoveEnclosing			RemoveEnclosingA
//	Remove Enclosing Quote
#define	RemoveEnclosingDQuote	RemoveEnclosingSQuoteA
//	Remove Enclosing Quote
#define	RemoveEnclosingSQuote	RemoveEnclosingSQuoteA
//	Remove Enclosing Quote
#define	RemoveEnclosingQuote	RemoveEnclosingQuoteA
//	Remove one Leading byte
#define	RemoveLeadingByte		RemoveLeadingByteA
//	Remove all Leading byte
#define	RemoveLeadingBytes		RemoveLeadingBytesA
//	Remove one Trailing byte
#define	RemoveTrailingByte		RemoveTrailingByteA
//	Remove all Trailing byte
#define	RemoveTrailingBytes		RemoveTrailingBytesA
//	Capitalize
#define	CapitalizeString		CapitalizeStringA
//	Lower Case
#define	LowerCaseString			LowerCaseStringA
//	Upper Case
#define	UpperCaseString			UpperCaseStringA
//	Alphabetic
#define	IsAlphaBetic			IsAlphaBeticA
//	Numeric
#define	IsNumeric				IsNumericA
//	Numeric
#define	IsNumericSigned			IsNumericSignedA
//	Number of numeric
#define	InRangeLength			InRangeLengthA
//	Number of not numeric
#define	NotInRangeLength		NotInRangeLengthA
//	Next Not In
#define	NextNotIn				NextNotInA
//	Next Not In Range
#define	NextNotInRange			NextNotInRangeA
//	Next In
#define	NextIn					NextInA
//	Next In Range
#define	NextInRange				NextInRangeA
//	Is Floating Or Floating Strict
#define IsFloating				IsFloatingA
//	Is Decimal Or Integer
#define IsDecimal				IsDecimalA

//	Human Readable
#define	HumanReadable			HumanReadableA
//	Seaprated Numbers
#define	SeparatedNumber			SeparatedNumberA

//	Check BOM Return the ccs : UTF-8 UTF-16LE UNICODE UTF-32LE UTF-32BE or empty or ERROR
#define	CheckBOMValue			CheckBOMValueA

//	BOM Directive
#define	AddBOMDirective			AddBOMDirectiveA
//	Build Read Directive for open
#define	GetReadDirective		GetReadDirectiveA
//	Build Write Directive for open
#define	GetWriteDirective		GetWriteDirectiveA
//	Build Append Directive for open
#define	GetAppendDirective		GetAppendDirectiveA

//	Open File : mode can be also w8 w16 wu r8 r16 ru a8 a16 au
#define	OpenFileCcs				OpenFileCcsA

//	Check File Exist
#define	CheckFileExist			CheckFileExistA

//	Check Path Exist
#define	CheckPathExist			CheckPathExistA

//	Run As Admin
#define	RunAsAdmin				RunAsAdminA

//	Treat Parameters
#define	TreatLaunchOptions		TreatLaunchOptionsA

//	Print Help
#define	PrintHelpLaunch			PrintHelpLaunchA

//	Read Me Option
#define	TreatReadMeOption		TreatReadMeOptionA

//	Print Help
#define	PrintHelpReadMe			PrintHelpReadMeA

//	Print Real Version
#define	PrintRealVersion		PrintRealVersionA

#endif

//
//====================================================================================
//	External
//====================================================================================
//	Search String Ignore Case
extern WCHAR *__strstriW ( WCHAR *pString, const WCHAR *pSearched );

//	Search String Ignore Case
extern char *__strstriA ( char *pString, const char *pSearched );

//	Search String With Case
extern WCHAR *__strstrW ( WCHAR *pString, const WCHAR *pSearched );

//	Search String With Case
extern char *__strstrA ( char *pString, const char *pSearched );

//	Replace String ignore case
extern bool __strrepiW ( WCHAR *lineRead, size_t sizeType, const WCHAR *pSearched, const WCHAR *pReplaced, bool bGlobal );

//	Replace String ignore case
extern bool __strrepiA ( char *lineRead, size_t sizeType, const char *pSearched, const char *pReplaced, bool bGlobal );

//	Replace String ignore case if it is at the First Position
extern bool __strrepxiW ( WCHAR *lineRead, size_t sizeType, const WCHAR *pSearched, const WCHAR *pReplaced );

//	Replace String ignore case if it is at the First Position
extern bool __strrepxiA ( char *lineRead, size_t sizeType, const char *pSearched, const char *pReplaced );

//	Replace String match case
extern bool __strrepW ( WCHAR *lineRead, size_t sizeType, const WCHAR *pSearched, const WCHAR *pReplaced, bool bGlobal );

//	Replace String match case
extern bool __strrepA ( char *lineRead, size_t sizeType, const char *pSearched, const char *pReplaced, bool bGlobal );

//	Replace String match case if it is at the First Position
extern bool __strrepxW ( WCHAR *lineRead, size_t sizeType, const WCHAR *pSearched, const WCHAR *pReplaced );

//	Replace String match case if it is at the First Position
extern bool __strrepxA ( char *lineRead, size_t sizeType, const char *pSearched, const char *pReplaced );

//	Compare with a table of chars
extern WCHAR *_wcschrsW ( WCHAR *pString, const WCHAR *pArray );

//	Compare with a table of chars
extern char *_strchrsA ( char *pString, const char *pArray );

//	Compare with a substring with the length of the substring
extern int __wcsnicmp1W ( const WCHAR *pString, const WCHAR *pSubString );

//	Compare with a substring with the length of the substring
extern int __strnicmp1A ( const char *pString, const char *pSubString );

//	Compare with a table of substrings teminated with a NULL string with the length of the substring
extern int __wcsnicmpLW ( const WCHAR *pString, ... );

//	Compare with a table of substrings teminated with a NULL string with the length of the substring
extern int __strnicmpLA ( const char *pString, ... );

//	Compare with a table of substrings teminated with a NULL string full match
extern int __wcsicmpLW ( const WCHAR *pString, ... );

//	Compare with a table of substrings teminated with a NULL string full match
extern int __stricmpLA ( const char *pString, ... );

//	Replace Char
extern void ReplaceCharW ( WCHAR *pString, WCHAR searched, WCHAR replaced );

//	Replace Char
extern void ReplaceCharA ( char *pString, char searched, char replaced );

//	Get Argument from command line
extern WCHAR *GetArgumentW ( int iArgCount, WCHAR* pArgValues[], int *pi, const WCHAR *pSigns = L"=", bool bExit = true );

//	Get Argument from command line
extern char *GetArgumentA ( int iArgCount, char* pArgValues[], int *pi, const char *pSigns = "=", bool bExit = true );

//	Remove File Type
extern void RemoveFileTypeW ( WCHAR *pFilename );
//	Remove File Type
extern void RemoveFileTypeA ( char *pFilename );

//	Remove File Name
extern void RemoveFileNameW ( WCHAR *pFilename );
//	Remove File Name
extern void RemoveFileNameA ( char *pFilename );

//	Remove Trailing CR/LF
extern void RemoveCRLFW ( WCHAR *pLine );
//	Remove Trailing CR/LF
extern void RemoveCRLFA ( char *pLine );

//	Find File Type
extern const WCHAR * FindFileTypeW ( const WCHAR *line );
//	Find File Type
extern const char * FindFileTypeA ( const char *line );

//	Find File Name
extern const WCHAR * FindFileNameW ( const WCHAR *line );
//	Find File Name
extern const char * FindFileNameA ( const char *line );

//	Find File Type
extern const WCHAR * GetFileTypeW ( const WCHAR *line );
//	Find File Type
extern const char * FindFileTypeA ( const char *line );

//	Ends with
///////////////
extern bool EndsWithIW ( const WCHAR *line,  const WCHAR *string );

//	Ends with
extern bool EndsWithIA ( const char *line,  const char *string );

//	Ends with
extern bool EndsWithW ( const WCHAR *line,  const WCHAR *string );

//	Ends with
extern bool EndsWithA ( const char *line,  const char *string );

//	Ends with
///////////////
extern bool EndsWithW ( WCHAR *line,  const WCHAR byte );

//	Ends with
extern bool EndsWithA ( char *line,  const char byte );

//	Ends with
extern bool EndsWithIW ( WCHAR *line,  const WCHAR byte );

//	Ends with
extern bool EndsWithIA ( char *line,  const char byte );

//	Start with
///////////////
extern bool StartsWithIW ( const WCHAR *line,  const WCHAR *string );

//	Start with
extern bool StartsWithIA ( const char *line,  const char *string );

//	Start with
extern bool StartsWithW ( const WCHAR *line,  const WCHAR *string );

//	Start with
extern bool StartsWithA ( const char *line,  const char *string );

//	Start with
///////////////
extern bool StartsWithIW ( const WCHAR *line,  const WCHAR byte );

//	Start with
extern bool StartsWithIA ( const char *line,  const char byte );

//	Start with
extern bool StartsWithW ( const WCHAR *line,  const WCHAR byte );

//	Start with
extern bool StartsWithA ( const char *line,  const char byte );

//	Remove Enclosing quote or other
extern BOOL RemoveEnclosingW ( WCHAR *pText, WCHAR quote );
extern BOOL RemoveEnclosingW ( WCHAR *pText );
extern BOOL RemoveEnclosingSQuoteW ( WCHAR *pText );
extern BOOL RemoveEnclosingDQuoteW ( WCHAR *pText );
extern BOOL RemoveEnclosingQuoteW ( WCHAR *pText );
extern BOOL RemoveEnclosingW( WCHAR *line, WCHAR *tuple );

//	Remove Enclosing quote or other
extern BOOL RemoveEnclosingA ( char *pText, char quote );
extern BOOL RemoveEnclosingA ( char *pText );
extern BOOL RemoveEnclosingSQuoteA ( char *pText );
extern BOOL RemoveEnclosingDQuoteA ( char *pText );
extern BOOL RemoveEnclosingQuoteA ( char *pText );
extern BOOL RemoveEnclosingA( char *line, char *tuple );

//	Remove one Leading byte
extern void RemoveLeadingByteW ( WCHAR *line, WCHAR byte );
//	Remove all Leading byte
extern void RemoveLeadingBytesW ( WCHAR *line, WCHAR byte );
//	Remove one Trailing byte
extern void RemoveTrailingByteW ( WCHAR *line, WCHAR byte );
//	Remove all Trailing byte
extern void RemoveTrailingBytesW ( WCHAR *line, WCHAR byte );
//	Remove all Trailing byte
extern void RemoveTrailingBytesW ( WCHAR *line, WCHAR *bytes );

//	Remove one Leading byte
extern void RemoveLeadingByteA ( char *line, char byte );
//	Remove all Leading byte
extern void RemoveLeadingBytesA ( char *line, char byte );
//	Remove one Trailing byte
extern void RemoveTrailingByteA ( char *line, char byte );
//	Remove all Trailing byte
extern void RemoveTrailingBytesA ( char *line, char byte );
//	Remove all Trailing byte
extern void RemoveTrailingBytesA ( char *line, char *bytes );

//	Capitalize
extern void CapitalizeStringW ( WCHAR *line, size_t iSize );
extern void CapitalizeStringW ( WCHAR *line );
//	Capitalize
extern void CapitalizeStringA ( char *line, size_t iSize );
extern void CapitalizeStringA ( char *line );

//	Uppercase
extern void UpperCaseStringW ( WCHAR *line, size_t iSize );
extern void UpperCaseStringW ( WCHAR *line );
//	Uppercase
extern void UpperCaseStringA ( char *line, size_t iSize );
extern void UpperCaseStringA ( char *line );

//	Lowercase
extern void LowerCaseStringW ( WCHAR *line, size_t iSize );
extern void LowerCaseStringW ( WCHAR *line );
//	Lowercase
extern void LowerCaseStringA ( char *line, size_t iSize );
extern void LowerCaseStringA ( char *line );

//	Alphabetic
extern bool IsAlphaBeticW ( const WCHAR byte );
extern bool IsAlphaBeticW ( const WCHAR *pString );

//	Alphabetic
extern bool IsAlphaBeticA ( const char byte );
extern bool IsAlphaBeticA ( const char *pString );

//	Numeric
extern bool IsNumericW ( const WCHAR byte );
//	Numeric
extern bool IsNumericW ( const WCHAR *pString );
//	Numeric
extern bool IsNumericSignedW ( const WCHAR byte );
//	Numeric
extern bool IsNumericSignedW ( const WCHAR *pString );
//	Numeric
extern bool IsNumericA ( const char byte );
//	Numeric
extern bool IsNumericA ( const char *pString );
//	Numeric
extern bool IsNumericSignedA ( const char byte );
//	Numeric
extern bool IsNumericSignedA ( const char *pString );

//	Number Of Numeric
extern int InRangeLengthW ( const WCHAR *pString, const WCHAR *tuple1, const WCHAR *included = L"", const WCHAR *excluded = L"" );
//	Number Of Numeric
extern int InRangeLengthA ( const char *pString, const char *tuple1, const char *included = "", const char *excluded = "" );

//	Number Of not Numeric
extern int NotInRangeLengthW ( const WCHAR *pString, const WCHAR *tuple1, const WCHAR *included = L"", const WCHAR *excluded = L"" );
//	Number Of not Numeric
extern int NotInRangeLengthA ( const char *pString, const char *tuple1, const char *included = "", const char *excluded = "" );

//	Next Not In
extern WCHAR *NextNotInW ( WCHAR *pString, const WCHAR *pChars );
//	Next Not In
extern const WCHAR *NextNotInW ( const WCHAR *pString, const WCHAR *pChars );
//	Next Not In
extern char *NextNotInA ( char *pString, const char *pChars );
//	Next Not In
extern const char *NextNotInA ( const char *pString, const char *pChars );

//	Next Not In
extern WCHAR *NextNotInRangeW ( WCHAR *pString, const WCHAR *tuple1, const WCHAR *included = L"", const WCHAR *excluded = L"" );
//	Next Not In
extern const WCHAR *NextNotInRangeW ( const WCHAR *pString, const WCHAR *tuple1, const WCHAR *included = L"", const WCHAR *excluded = L"" );
//	Next Not In
extern char *NextNotInRangeA ( char *pString, const char *tuple1, const char *included = "", const char *excluded = "" );
//	Next Not In
extern const char *NextNotInRangeA ( const char *pString, const char *tuple1, const char *included = "", const char *excluded = "" );

//	Next In
extern WCHAR *NextInW ( WCHAR *pString, const WCHAR *pChars );
//	Next In
extern const WCHAR *NextInW ( const WCHAR *pString, const WCHAR *pChars );
//	Next Not In
extern char *NextInA ( char *pString, const char *pChars );
//	Next Not In
extern const char *NextInA ( const char *pString, const char *pChars );

//	Is In Range
extern bool IsInRangeW ( const WCHAR byte, const WCHAR *tuple1, const WCHAR *included = L"", const WCHAR *excluded = L"" );
//	Is In Range
extern bool IsInRangeA ( const char byte, const char *tuple1, const char *included = "", const char *excluded = "" );

//	Is Not In Range
extern bool IsNotInRangeW ( const WCHAR byte, const WCHAR *tuple1, const WCHAR *included = L"", const WCHAR *excluded = L"" );
//	Is Not In Range
extern bool IsNotInRangeA ( const char byte, const char *tuple1, const char *included = "", const char *excluded = "" );

//	Next In Between Tuple
extern WCHAR *NextInRangeW ( WCHAR *pString, const WCHAR *tuple1, const WCHAR *included = L"", const WCHAR *excluded = L"" );
//	Next In Between Tuple
extern const WCHAR *NextInRangeW (const  WCHAR *pString, const WCHAR *tuple1, const WCHAR *included = L"", const WCHAR *excluded = L"" );
//	Next In Between Tuple
extern char *NextInRangeA ( char *pString, const char *tuple1, const char *included = "", const char *excluded = ""  );
//	Next In Between Tuple
extern const char *NextInRangeA ( const char *pString, const char *tuple1, const char *included = "", const char *excluded = "" );

//	Is Floating
extern bool IsFloatingW ( const WCHAR *pText, bool &bFloatingDecimal, int &iNumberDecimal, bool bStrict );

//	Is Floating
extern bool IsFloatingA ( const char *pText, bool &bFloatingDecimal, int &iNumberDecimal, bool bStrict );

//	Is Floating
extern bool IsFloatingW ( const WCHAR *pText );

//	Is Floating
extern bool IsFloatingA ( const char *pText );

//	Is Decimal Or Integer
extern bool IsDecimalW ( const WCHAR *pText );

//	Is Decimal Or Integer
extern bool IsDecimalA ( const char *pText );

//	Human Readable 1.2G : Base Routine
extern WCHAR *HumanReadableW ( WCHAR *pResult, size_t sizeInChars, double largeInt, int decimal = 1 );
//	Human Readable 1.2G
extern WCHAR *HumanReadableW ( WCHAR *pResult, size_t sizeInChars, unsigned long long largeInt, int decimal = 1 );
//	Human Readable 1.2G
extern WCHAR *HumanReadableW ( WCHAR *pResult, size_t sizeInChars, long long largeInt, int decimal = 1 );
//	Human Readable 1.2G
extern WCHAR *HumanReadableW ( WCHAR *pResult, size_t sizeInChars, LARGE_INTEGER largeInt, int decimal = 1 );

//	Human Readable 1.2G : Base Routine
extern char *HumanReadableA ( char *pResult, size_t sizeInChars, double largeInt, int decimal = 1 );
//	Human Readable 1.2G
extern char *HumanReadableA ( char *pResult, size_t sizeInChars, unsigned long long largeInt, int decimal = 1 );
//	Human Readable 1.2G
extern char *HumanReadableA ( char *pResult, size_t sizeInChars, long long largeInt, int decimal = 1 );
//	Human Readable 1.2G
extern char *HumanReadableA ( char *pResult, size_t sizeInChars, LARGE_INTEGER largeInt, int decimal = 1 );

//	Separated 1,234
extern WCHAR *SeparatedNumberW ( WCHAR *pResult, size_t sizeInChars, unsigned long long largeInt, WCHAR separator = L',' );
//	Separated 1,234
extern WCHAR *SeparatedNumberW ( WCHAR *pResult, size_t sizeInChars, long long largeInt, WCHAR separator = L',' );
//	Separated 1,234
extern WCHAR *SeparatedNumberW ( WCHAR *pResult, size_t sizeInChars, LARGE_INTEGER largeInt, char separator = L',' );

//	Separated 1,234
extern char *SeparatedNumberA ( char *pResult, size_t sizeInChars, unsigned long long largeInt, char separator = ',' );
//	Separated 1,234
extern char *SeparatedNumberA ( char *pResult, size_t sizeInChars, long long largeInt, char separator = ',' );
//	Separated 1,234
extern char *SeparatedNumberA ( char *pResult, size_t sizeInChars, LARGE_INTEGER largeInt, char separator = ',' );

//	Set WideCharToMultiByte Flag
extern DWORD SetWCFlag ( INT codePage, const char *DefaultChar, LPCSTR &pDefaultChar, BOOL &UsedDefaultChar, LPBOOL &pUsedDefaultChar );

//	Set MultiByteToWideChar Flag
extern DWORD SetMBFlag ( INT codePage );

//	Simple Conversion
extern SizeInBytes ConvertWCToMBCS (	const WCHAR *wcLineZero, SizeInChars iWcLineZero, 
										char *mbLineFinal, SizeInBytes iMbLineFinal, INT CodePageTo, BOOL &bErrors );

//	Simple Conversion
extern SizeInBytes ConvertMBCSToWC (	const char *mbLineZero, SizeInBytes iMbLineZero,
										WCHAR *wcLineFinal, SizeInBytes iWcLineFinal, INT CodePageTo, BOOL &bErrors );

extern SizeInBytes ConvertWCToWC (	WCHAR *wcLine, SizeInChars iWcLine, SizeInChars iWcLineLength,
									INT CodePageTo, INT CodePageFrom, BOOL &bErrors );

extern SizeInBytes ConvertMBToMB (	char *mbLine, SizeInBytes iMbLine, SizeInBytes iMbLineLength,
									INT CodePageFrom, INT CodePageTo, BOOL &bErrors );

//	Try Convert From UTF8 to ANSI (CP_ACP)
extern bool Try5ConvertUtf8ToAnsiW ( WCHAR *pSource, int iSizeByte, bool bDisplay = false );

//	Try Convert From UTF8 to ANSI (CP_ACP)
extern bool Try4ConvertUtf8ToAnsiW ( WCHAR *pSource, int iSizeByte, bool bDisplay = false );

//	Check BOM Return the ccs : UTF-8 UTF-16LE UNICODE UTF-32LE UTF-32BE or empty or ERROR
extern const WCHAR *CheckBOMValueW ( const WCHAR *pFilename, bool display = false );

//	Check BOM Return the ccs : UTF-8 UTF-16LE UNICODE UTF-32LE UTF-32BE or empty or ERROR
extern const char *CheckBOMValueA ( const char *pFilename, bool display = false );

//	BOM Directive
extern bool AddBOMDirectiveW ( WCHAR *pText, size_t iSizeInCHars, const WCHAR *pBOMValue );

//	BOM Directive
extern bool AddBOMDirectiveA ( char *pText, size_t iSizeInCHars, const char *pBOMValue );

//	Open File : mode can be also w8 w16 wu r8 r16 ru a8 a16 au
extern FILE *OpenFileCcsW ( const WCHAR *filename, const WCHAR *mode = L"r", bool binary = false, bool display = false, const WCHAR *bomFile = L"" );

//	Open File : mode can be also w8 w16 wu r8 r16 ru a8 a16 au
extern FILE *OpenFileCcsA ( const char *filename, const char *mode = "r", bool binary = false, bool display = false, const char *bomFile = "" );

//	Open File : mode can be also w8 w16 wu r8 r16 ru a8 a16 au
extern errno_t OpenFileCcsW ( FILE **hFile, const WCHAR *filename, const WCHAR *mode, bool display = false, const WCHAR *bomFile = L"" );

//	Open File : mode can be also w8 w16 wu r8 r16 ru a8 a16 au
extern errno_t OpenFileCcsA ( FILE **hFile, const char *filename, const char *mode, bool display = false, const char *bomFile = ""  );

//	Check File Exist
extern bool CheckFileExistW ( const WCHAR *pathname );

//	Check File Exist
extern bool CheckFileExistA ( const char *pathname );

//	Check Path Exist
extern bool CheckPathExistW ( const WCHAR *pathname );

//	Check Path Exist
extern bool CheckPathExistA ( const char *pathname );

//
//	Set Event Handler
extern void SetConsoleEventHandler ( bool *pIsAborted, bool bTerminate = false );

//	Is This User Admin
extern BOOL IsThisUserAdmin();

//	Run As Admiun
extern BOOL RunAsAdminA (int iArgCount, char * pArgValues[], bool bWait = true );

//	Run As Admiun
extern BOOL RunAsAdminW(int iArgCount, WCHAR * pArgValues[], bool bWait = true );

//	Treat Parameters
extern BOOL TreatLaunchOptionsA(int iArgCount, char* pArgValues[], const char* pOption, int &iX);

//	Treat Parameters
extern BOOL TreatLaunchOptionsW(int iArgCount, WCHAR* pArgValues[], const WCHAR* pOption, int &iX);

//	Print Help
extern void PrintHelpLaunchA ( int iWidth, const char *programName, int iArgCount, char* pArgValues[], bool bLong );

//	Print Help
extern void PrintHelpLaunchW ( int iWidth, const WCHAR *programName, int iArgCount, WCHAR* pArgValues[], bool bLong );

//	Read README.md
extern char *GetReadMeResouce ( UINT iResource );

//	Treat Parameters
extern BOOL TreatReadMeOptionA(int iArgCount, char* pArgValues[], const char* pOption, int &iX);

//	Treat Parameters
extern BOOL TreatReadMeOptionW(int iArgCount, WCHAR* pArgValues[], const WCHAR* pOption, int &iX);

//	Print Help
extern void PrintHelpReadMeA ( int iWidth, const char *programName, int iArgCount, char* pArgValues[], bool bLong );

//	Print Help
extern void PrintHelpReadMeW ( int iWidth, const WCHAR *programName, int iArgCount, WCHAR* pArgValues[], bool bLong );

//	Print Real Version
extern void PrintRealVersionA ( int iWidth = 16 );

//	Print Real Version
extern void PrintRealVersionW ( int iWidth = 16 );

//
//====================================================================================
//	Macro
//====================================================================================

//	Is Alpha NUmeric
#define	IsAlphaNumericW(x)			IsAlphaBeticW(x)||IsNumericW(x)

//	Is Alpha NUmeric
#define	IsAlphaNumericA(x)			IsAlphaBeticA(x)||IsNumericA(x)

//	Next Spaces or Tab
#define	NextSpacesW(x)				NextInW(x,L" \t")
//	Next Spaces or Tab
#define	NextSpacesA(x)				NextInA(x," \t")

//	Next Not Spaces or Tab
#define	NextNotSpacesW(x)			NextNotInW(x,L" \t")
//	Next Not Spaces or Tab
#define	NextNotSpacesA(x)			NextNotInA(x," \t")

//	Next Numeric
#define	NumericLengthW(x)			InRangeLengthW(x,L"09")
//	Next Numeric
#define	NumericLengthA(x)			InRangeLengthA(x,"09")

//	Next Numeric
#define	NotNumericLengthW(x)		NotInRangeLengthW(x,L"09")
//	Next Numeric
#define	NotNumericLengthA(x)		NotInRangeLengthA(x,"09")

//	Next Numeric
#define	NextNumericW(x)				NextInRangeW(x,L"09")
//	Next Numeric
#define	NextNumericA(x)				NextInRangeA(x,"09")

//	Next Not Numeric
#define	NextNotNumericW(x)			NextNotInRangeW(x,L"09")
//	Next Not Numeric
#define	NextNotNumericA(x)			NextNotInRangeA(x,"09")

//	Next Alphabetic
#define	NextAlphabeticW(x)			NextInRangeW(x,L"azAZ")
//	Next Alphabetic
#define	NextAlphabeticA(x)			NextInRangeA(x,"azAZ")

//	Next Alphabetic
#define	NextAlphaExtendW(x)			NextInRangeW(x,L"azAZ\xc0\xff")
//	Next Alphabetic
#define	NextAlphaExtendA(x)			NextInRangeA(x,"azAZ\xc0\xff")

//	Next AlphaNumeric
#define	NextAlphaNumericW(x)		NextInRangeW(x,L"azAZ09")
//	Next AlphaNumeric
#define	NextAlphaNumericA(x)		NextInRangeA(x,"azAZ09")

//	Next Not Alphabetic
#define	NextNotAlphabeticW(x)		NextNotInRangeW(x,L"azAZ")
//	Next Not Alphabetic
#define	NextNotAlphabeticA(x)		NextNotInRangeA(x,"azAZ")

//	Next Not Alphabetic
#define	NextNotAlphaNumericW(x)		NextNotInRangeW(x,L"azAZ09")
//	Next Not Alphabetic
#define	NextNotAlphaNumericA(x)		NextNotInRangeA(x,"azAZ09")

//	UNICODE
#ifdef UNICODE
//	Is Alpha NUmeric
#define	IsAlphaNumeric			IsAlphaNumericW

//	Next Spaces or Tab
#define	NextSpaces				NextSpacesW
//	Next Not Spaces or Tab
#define	NextNotSpaces			NextNotSpacesW
//	Next Numeric
#define	NextNumeric				NextNumericW
//	Next Not Numeric
#define	NextNotNumeric			NextNotNumericW
//	Next Alphabetic
#define	NextAlphabetic			NextAlphabeticW
//	Next Not Alphabetic
#define	NextNotAlphabetic		NextNotAlphabeticW
//	MBCS
#else
//	Is Alpha NUmeric
#define	IsAlphaNumeric			IsAlphaNumericA

//	Next Spaces or Tab
#define	NextSpaces				NextSpacesA
//	Next Not Spaces or Tab
#define	NextNotSpaces			NextNotSpacesA
//	Next Numeric
#define	NextNumeric				NextNumericA
//	Next Not Numeric
#define	NextNotNumeric			NextNotNumericA
//	Next Alphabetic
#define	NextAlphabetic			NextAlphabeticA
//	Next Not Alphabetic
#define	NextNotAlphabetic		NextNotAlphabeticA

#endif

