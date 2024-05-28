/*
 ************************************************************************************
 *
 *      File:           strstr.cpp
 *      Class:          
 *
 ************************************************************************************
 *      Description:
 *
 ************************************************************************************
 *      Notes:
 *
 ************************************************************************************
 *
 *      E/Mail: ian.macallan@gmail.com
 *      (c) 2003 - 2023 / Macallan
 *      
 ************************************************************************************
 *
 */
#include "stdafx.h"

#include <share.h>
#include <stdio.h>

#include <Shlwapi.h>
#include <Shellapi.h>

#pragma comment (lib, "Shlwapi.lib")
#pragma comment (lib, "advapi32.lib")
#pragma comment (lib, "Shell32.lib")
#pragma comment (lib, "Version.lib")

//
#include "strstr.h"
#include "PrintRoutine.h"

//
static  const WCHAR *CCS_UTF_8_W        = L"UTF-8";
static  const WCHAR *CCS_UTF_16LE_W     = L"UTF-16LE";
static  const WCHAR *CCS_UNICODE_W      = L"UNICODE";
static  const WCHAR *CCS_UTF_32LE_W     = L"UTF-32LE";
static  const WCHAR *CCS_UTF_32BE_W     = L"UTF-32BE";

static  const char  *CCS_UTF_8_A        = "UTF-8";
static  const char  *CCS_UTF_16LE_A     = "UTF-16LE";
static  const char  *CCS_UNICODE_A      = "UNICODE";
static  const char  *CCS_UTF_32LE_A     = "UTF-32LE";
static  const char  *CCS_UTF_32BE_A     = "UTF-32BE";

static WCHAR    ErrorTextW [ 1024 ]     = L"";
static char     ErrorTextA [ 1024 ]     = "";

//
#ifndef _wsizeof
#define _wsizeof(x) (sizeof(x)/sizeof(WCHAR))
#endif

//
//  For Version Lower than Windows 7
#ifndef WC_ERR_INVALID_CHARS
#define WC_ERR_INVALID_CHARS    NULL
#endif // !WC_ERR_INVALID_CHARS

//
const bool  ForceShareMode          = true;

//
bool    WaitProcessMode             = true;
bool    RunAsAdminMode              = true;
bool    ReadMeTextMode              = true;

//
DWORD   ConsoleEventSignal          = -1;
bool    ConsoleEventAborted         = false;
bool    ConsoleEventTerminate       = false;
bool    *IsAbortedPointer           = &ConsoleEventAborted;

//
//====================================================================================
//  Search String Ignore Case
//====================================================================================
WCHAR *__strstriW ( WCHAR *pString, const WCHAR *pSearched )
{
    if ( ( pString == NULL ) || ( pSearched == NULL ) )
    {
        return NULL;
    }

    if ( ( wcslen ( pString ) == 0 ) || ( wcslen ( pSearched ) == 0 ) )
    {
        return NULL;
    }

    size_t  iLen = wcslen ( pSearched );
    if ( iLen == 0 )
    {
        return NULL;
    }

    //
    while ( *pString != L'\0' )
    {
        if ( _wcsnicmp ( pString, pSearched, iLen ) == 0 )
        {
            return pString;
        }

        pString++;
    }

    return NULL;
}

//
//====================================================================================
//
//====================================================================================
char *__strstriA ( char *pString, const char *pSearched )
{
    if ( ( pString == NULL ) || ( pSearched == NULL ) )
    {
        return NULL;
    }

    if ( ( strlen ( pString ) == 0 ) || ( strlen ( pSearched ) == 0 ) )
    {
        return NULL;
    }

    size_t  iLen = strlen ( pSearched );
    if ( iLen == 0 )
    {
        return NULL;
    }

    //
    while ( *pString != L'\0' )
    {
        if ( _strnicmp ( pString, pSearched, iLen ) == 0 )
        {
            return pString;
        }

        pString++;
    }

    return NULL;
}

//
//====================================================================================
//  Search String
//====================================================================================
WCHAR *__strstrW ( WCHAR *pString, const WCHAR *pSearched )
{
    if ( ( pString == NULL ) || ( pSearched == NULL ) )
    {
        return NULL;
    }

    if ( ( wcslen ( pString ) == 0 ) || ( wcslen ( pSearched ) == 0 ) )
    {
        return NULL;
    }

    size_t  iLen = wcslen ( pSearched );
    if ( iLen == 0 )
    {
        return NULL;
    }

    //
    while ( *pString != L'\0' )
    {
        if ( wcsncmp ( pString, pSearched, iLen ) == 0 )
        {
            return pString;
        }

        pString++;
    }

    return NULL;
}

//
//====================================================================================
//  Search String
//====================================================================================
char *__strstrA ( char *pString, const char *pSearched )
{
    if ( ( pString == NULL ) || ( pSearched == NULL ) )
    {
        return NULL;
    }

    if ( ( strlen ( pString ) == 0 ) || ( strlen ( pSearched ) == 0 ) )
    {
        return NULL;
    }

    size_t  iLen = strlen ( pSearched );
    if ( iLen == 0 )
    {
        return NULL;
    }

    //
    while ( *pString != L'\0' )
    {
        if ( strncmp ( pString, pSearched, iLen ) == 0 )
        {
            return pString;
        }

        pString++;
    }

    return NULL;
}

//
//====================================================================================
//  Search String and Replace Ignore Case
//====================================================================================
bool __strrepiW (   WCHAR *lineRead, size_t sizeType,
                    const WCHAR *pSearched, const WCHAR *pReplaced, bool bGlobal )
{
    bool replaced   = false;

    if ( lineRead == NULL || wcslen ( lineRead ) == 0 )
    {
        return replaced;
    }

    if ( pSearched == NULL || wcslen ( pSearched ) == 0 )
    {
        return replaced;
    }

    if ( pReplaced == NULL )
    {
        return replaced;
    }

    //
    while ( sizeType > 0 && *lineRead != L'\0' )
    {
        int compare = _wcsnicmp ( lineRead, pSearched, wcslen ( pSearched ) );
        if ( compare == 0 )
        {
            if ( sizeType >= wcslen ( pReplaced ) )
            {
                size_t  lenSearched = wcslen ( pSearched );
                size_t  lenReplaced = wcslen ( pReplaced );
                size_t  lenLine     = wcslen ( lineRead );

                //
                //      Move
                int deltaType   = ( int ) lenSearched - ( int ) lenReplaced;

                int sizeToMove = ( int ) lenLine - deltaType + 1;
                if ( ( lenSearched != lenReplaced ) && ( sizeToMove > 0 ) )
                {
                    wmemmove_s ( lineRead + lenReplaced, sizeType, lineRead + lenSearched, sizeToMove );
                }
                else
                {
                    //
                    //  Zero End Of Line
//                  wmemset ( lineRead, NULL, lenReplaced + 1 );
                }
                //
                //  Copy String
                wmemcpy_s ( lineRead, sizeType, pReplaced, lenReplaced );

                //
                lineRead    += lenReplaced;
                sizeType    -= lenReplaced;

                //
                //      Say it has been replaced
                replaced    = true;

                if ( ! bGlobal )
                {
                    return replaced;
                }
            }
            else
            {
                sizeType    = 0;
                return  replaced;
            }
        }
        else
        {
            lineRead++;
            sizeType--;
        }
    }

    return replaced;
}

//
//====================================================================================
//  Search String and Replace Ignore Case
//====================================================================================
bool __strrepiA (   char *lineRead, size_t sizeType,
                    const char *pSearched, const char *pReplaced, bool bGlobal )
{
    bool replaced   = false;

    if ( lineRead == NULL || strlen ( lineRead ) == 0 )
    {
        return replaced;
    }

    if ( pSearched == NULL || strlen ( pSearched ) == 0 )
    {
        return replaced;
    }

    if ( pReplaced == NULL )
    {
        return replaced;
    }

    //
    while ( sizeType > 0 && *lineRead != L'\0' )
    {
        int compare = _strnicmp ( lineRead, pSearched, strlen ( pSearched ) );
        if ( compare == 0 )
        {
            if ( sizeType >= strlen ( pReplaced ) )
            {
                size_t  lenSearched = strlen ( pSearched );
                size_t  lenReplaced = strlen ( pReplaced );
                size_t  lenLine     = strlen ( lineRead );

                //
                //      Move
                int deltaType   = ( int ) lenSearched - ( int ) lenReplaced;

                int sizeToMove = ( int ) lenLine - deltaType + 1;
                if ( ( lenSearched != lenReplaced ) && ( sizeToMove > 0 ) )
                {
                    memmove_s ( lineRead + lenReplaced, sizeType, lineRead + lenSearched, sizeToMove );
                }
                else
                {
                    //
                    //  Zero End Of Line
//                  wmemset ( lineRead, NULL, lenReplaced + 1 );
                }
                //
                //  Copy String
                memcpy_s ( lineRead, sizeType, pReplaced, lenReplaced );

                //
                lineRead    += lenReplaced;
                sizeType    -= lenReplaced;

                //
                //      Say it has been replaced
                replaced    = true;

                if ( ! bGlobal )
                {
                    return replaced;
                }
            }
            else
            {
                sizeType    = 0;
                return  replaced;
            }
        }
        else
        {
            lineRead++;
            sizeType--;
        }
    }

    return replaced;
}

//
//////////////////////////////////////////////////////////////////////////////
//  Replace only if the string match first position
//////////////////////////////////////////////////////////////////////////////
bool __strrepxiW (  WCHAR *lineRead, size_t sizeType, const WCHAR *pSearched, const WCHAR *pReplaced )
{
    if ( _wcsnicmp ( lineRead, pSearched, wcslen(pSearched) ) == 0 )
    {
        return __strrepiW ( lineRead, sizeType, pSearched, pReplaced, false );
    }

    return false;
}

//
//////////////////////////////////////////////////////////////////////////////
//  Replace only if the string match first position
//////////////////////////////////////////////////////////////////////////////
bool __strrepxiA (  char *lineRead, size_t sizeType, const char *pSearched, const char *pReplaced )
{
    if ( _strnicmp ( lineRead, pSearched, strlen(pSearched) ) == 0 )
    {
        return __strrepiA ( lineRead, sizeType, pSearched, pReplaced, false );
    }

    return false;
}

//
//////////////////////////////////////////////////////////////////////////////
//  Replace String With Case
//////////////////////////////////////////////////////////////////////////////
bool __strrepW (    WCHAR *lineRead, size_t sizeType,
                    const WCHAR *pSearched, const WCHAR *pReplaced, bool bGlobal )
{
    bool replaced   = false;

    if ( lineRead == NULL || wcslen ( lineRead ) == 0 )
    {
        return replaced;
    }

    if ( pSearched == NULL || wcslen ( pSearched ) == 0 )
    {
        return replaced;
    }

    if ( pReplaced == NULL )
    {
        return replaced;
    }

    //
    while ( sizeType > 0 && *lineRead != L'\0' )
    {
        int compare = wcsncmp ( lineRead, pSearched, wcslen ( pSearched ) );
        if ( compare == 0 )
        {
            if ( sizeType >= wcslen ( pReplaced ) )
            {
                size_t  lenSearched = wcslen ( pSearched );
                size_t  lenReplaced = wcslen ( pReplaced );
                size_t  lenLine     = wcslen ( lineRead );

                //
                //      Move
                int deltaType   = ( int ) lenSearched - ( int ) lenReplaced;

                int sizeToMove = ( int ) lenLine - deltaType + 1;
                if ( ( lenSearched != lenReplaced ) && ( sizeToMove > 0 ) )
                {
                    wmemmove_s ( lineRead + lenReplaced, sizeType, lineRead + lenSearched, sizeToMove );
                }
                else
                {
                    //
                    //  Zero End Of Line
//                  wmemset ( lineRead, NULL, lenReplaced + 1 );
                }
                //
                //  Copy String
                wmemcpy_s ( lineRead, sizeType, pReplaced, lenReplaced );

                //
                lineRead    += lenReplaced;
                sizeType    -= lenReplaced;

                //
                //      Say it has been replaced
                replaced    = true;

                if ( ! bGlobal )
                {
                    return replaced;
                }
            }
            else
            {
                sizeType    = 0;
                return  replaced;
            }
        }
        else
        {
            lineRead++;
            sizeType--;
        }
    }

    return replaced;
}

//
//////////////////////////////////////////////////////////////////////////////
//  Replace String With Case
//////////////////////////////////////////////////////////////////////////////
bool __strrepA (    char *lineRead, size_t sizeType,
                    const char *pSearched, const char *pReplaced, bool bGlobal )
{
    bool replaced   = false;

    if ( lineRead == NULL || strlen ( lineRead ) == 0 )
    {
        return replaced;
    }

    if ( pSearched == NULL || strlen ( pSearched ) == 0 )
    {
        return replaced;
    }

    if ( pReplaced == NULL )
    {
        return replaced;
    }

    //
    while ( sizeType > 0 && *lineRead != L'\0' )
    {
        int compare = strncmp ( lineRead, pSearched, strlen ( pSearched ) );
        if ( compare == 0 )
        {
            if ( sizeType >= strlen ( pReplaced ) )
            {
                size_t  lenSearched = strlen ( pSearched );
                size_t  lenReplaced = strlen ( pReplaced );
                size_t  lenLine     = strlen ( lineRead );

                //
                //      Move
                int deltaType   = ( int ) lenSearched - ( int ) lenReplaced;

                int sizeToMove = ( int ) lenLine - deltaType + 1;
                if ( ( lenSearched != lenReplaced ) && ( sizeToMove > 0 ) )
                {
                    memmove_s ( lineRead + lenReplaced, sizeType, lineRead + lenSearched, sizeToMove );
                }
                else
                {
                    //
                    //  Zero End Of Line
//                  wmemset ( lineRead, NULL, lenReplaced + 1 );
                }
                //
                //  Copy String
                memcpy_s ( lineRead, sizeType, pReplaced, lenReplaced );

                //
                lineRead    += lenReplaced;
                sizeType    -= lenReplaced;

                //
                //      Say it has been replaced
                replaced    = true;

                if ( ! bGlobal )
                {
                    return replaced;
                }
            }
            else
            {
                sizeType    = 0;
                return  replaced;
            }
        }
        else
        {
            lineRead++;
            sizeType--;
        }
    }

    return replaced;
}

//
//////////////////////////////////////////////////////////////////////////////
//  Replace only if the string match first position
//////////////////////////////////////////////////////////////////////////////
bool __strrepxW (   WCHAR *lineRead, size_t sizeType, const WCHAR *pSearched, const WCHAR *pReplaced )
{
    if ( wcsncmp ( lineRead, pSearched, wcslen(pSearched) ) == 0 )
    {
        return __strrepW ( lineRead, sizeType, pSearched, pReplaced, false );
    }

    return false;
}

//
//////////////////////////////////////////////////////////////////////////////
//  Replace only if the string match first position
//////////////////////////////////////////////////////////////////////////////
bool __strrepxA (   char *lineRead, size_t sizeType, const char *pSearched, const char *pReplaced )
{
    if ( strncmp ( lineRead, pSearched, strlen(pSearched) ) == 0 )
    {
        return __strrepA ( lineRead, sizeType, pSearched, pReplaced, false );
    }

    return false;
}

//
//====================================================================================
//  Search an array of char in String
//====================================================================================
WCHAR *_wcschrsW ( WCHAR *pString, const WCHAR *pArray )
{
    for ( size_t c = 0; c < wcslen(pString); c++ )
    {
        for ( size_t a = 0; a < wcslen(pArray); a++ )
        {
            if ( pString [ c ] == pArray [ a ] )
            {
                return pString + c;
            }
        }
    }
    return NULL;
}

//
//====================================================================================
//  Search an array of char in String
//====================================================================================
char *_strchrsA ( char *pString, const char *pArray )
{
    for ( size_t c = 0; c < strlen(pString); c++ )
    {
        for ( size_t a = 0; a < strlen(pArray); a++ )
        {
            if ( pString [ c ] == pArray [ a ] )
            {
                return pString + c;
            }
        }
    }
    return NULL;
}

//
//====================================================================================
//  Compare String Ignore Case limited to length of substring
//====================================================================================
int __wcsnicmp1W ( const WCHAR *pString, const WCHAR *pSubString )
{
    return _wcsnicmp ( pString, pSubString, wcslen(pSubString) );
}

//
//====================================================================================
//  Compare with a substring with the length of the substring
//====================================================================================
int __strnicmp1A ( const char *pString, const char *pSubString )
{
    return _strnicmp ( pString, pSubString, strlen(pSubString) );
}

//
//====================================================================================
//  Compare with a table of substrings teminated with a NULL string with the length of the substring
//====================================================================================
int __wcsnicmpLW ( const WCHAR *pString, ... )
{
    int iCmp    = -1;

    va_list argptr;
    va_start(argptr, pString);
    WCHAR* argVal = va_arg(argptr,WCHAR*);
    while ( argVal != NULL )
    {
        iCmp = _wcsnicmp ( pString, argVal, wcslen(argVal) );
        if ( iCmp == 0 )
        {
            va_end(argptr);
            return iCmp;
        }
        argVal = va_arg(argptr,WCHAR*);
    }
    va_end(argptr);
    return iCmp;
}

//
//====================================================================================
//  Compare with a table of substrings teminated with a NULL string with the length of the substring
//====================================================================================
int __strnicmpLA ( const char *pString, ... )
{
    int iCmp    = -1;

    va_list argptr;
    va_start(argptr, pString);
    char* argVal = va_arg(argptr,char*);
    while ( argVal != NULL )
    {
        iCmp = _strnicmp ( pString, argVal, strlen(argVal) );
        if ( iCmp == 0 )
        {
            va_end(argptr);
            return iCmp;
        }
        argVal = va_arg(argptr,char*);
    }
    va_end(argptr);
    return iCmp;
}

//
//====================================================================================
//  Compare with a table of substrings teminated with a NULL string full match
//====================================================================================
int __wcsicmpLW ( const WCHAR *pString, ... )
{
    int iCmp    = -1;

    va_list argptr;
    va_start(argptr, pString);
    WCHAR* argVal = va_arg(argptr,WCHAR*);
    while ( argVal != NULL )
    {
        iCmp = _wcsicmp ( pString, argVal );
        if ( iCmp == 0 )
        {
            va_end(argptr);
            return iCmp;
        }
        argVal = va_arg(argptr,WCHAR*);
    }
    va_end(argptr);
    return iCmp;
}

//
//====================================================================================
//  Compare with a table of substrings teminated with a NULL string full match
//====================================================================================
int __stricmpLA ( const char *pString, ... )
{
    int iCmp    = -1;

    va_list argptr;
    va_start(argptr, pString);
    char* argVal = va_arg(argptr,char*);
    while ( argVal != NULL )
    {
        iCmp = _stricmp ( pString, argVal );
        if ( iCmp == 0 )
        {
            va_end(argptr);
            return iCmp;
        }
        argVal = va_arg(argptr,char*);
    }
    va_end(argptr);
    return iCmp;
}

//
//====================================================================================
//  Replace On Char by Another
//====================================================================================
void ReplaceCharW ( WCHAR *pString, WCHAR searched, WCHAR replaced )
{
    for ( size_t i = 0; i < wcslen(pString); i++ )
    {
        if ( pString [ i ] == searched )
        {
            pString [ i ] = replaced;
        }
    }
}

//
//====================================================================================
//  Replace On Char by Another
//====================================================================================
void ReplaceCharA ( char *pString, char searched, char replaced )
{
    for ( size_t i = 0; i < strlen(pString); i++ )
    {
        if ( pString [ i ] == searched )
        {
            pString [ i ] = replaced;
        }
    }
}

//
//====================================================================================
//  Get Argument
//====================================================================================
WCHAR *GetArgumentW ( int iArgCount, WCHAR * pArgValues[], int *pi, const WCHAR *pSigns, bool bExit )
{
    int     i = *pi;
    WCHAR *pCode = NULL;
    WCHAR *pEqual = _wcschrsW ( pArgValues [ i ], pSigns );
    if ( pEqual != NULL )
    {
        pCode = pEqual + 1;
        if ( wcslen(pCode) == 0 )
        {
            if ( bExit )
            {
                PrintStderrW ( L"The Value for '%s' is not defined\n", pArgValues [ i ] );
                exit(1);
            }
        }
        return pCode;
    }
    else
    {
        *pi = i + 1;
        if ( *pi >= iArgCount )
        {
            if ( bExit )
            {
                PrintStderrW ( L"The Value for '%s' is not defined\n", pArgValues [ i ] );
                exit(1);
            }
            return pArgValues [ i ] + wcslen( pArgValues [ i ] );
        }
        pCode = pArgValues [ *pi ];
    }

    return pCode;
}

//
//====================================================================================
//  Get Argument
//====================================================================================
char *GetArgumentA ( int iArgCount, char * pArgValues[], int *pi, const char *pSigns, bool bExit )
{
    int     i = *pi;
    char *pCode = NULL;
    char *pEqual = _strchrsA ( pArgValues [ i ], pSigns );
    if ( pEqual != NULL )
    {
        pCode = pEqual + 1;
        if ( strlen(pCode) == 0 )
        {
            if ( bExit )
            {
                PrintStderrA ( "The Value for '%s' is not defined\n", pArgValues [ i ] );
                exit(1);
            }
        }
        return pCode;
    }
    else
    {
        *pi = i + 1;
        if ( *pi >= iArgCount )
        {
            PrintStderrA ( "The Value for '%s' is not defined\n", pArgValues [ i ] );
            if ( bExit )
            {
                exit(1);
            }
            return pArgValues [ i ] + strlen(pArgValues [ i ]);
        }
        pCode = pArgValues [ *pi ];
    }

    return pCode;
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
void RemoveFileTypeW ( WCHAR *pFilename )
{
    size_t  len = wcslen (pFilename);
    if ( len > 0 )
    {
        for ( int i = ( int ) len - 1; i >= 0; i-- )
        {
            if ( pFilename [ i ] == L'.' )
            {
                pFilename [ i ] = L'\0';
                return;
            }

            if ( pFilename [ i ] == L'\\' )
            {
                return;
            }

            if ( pFilename [ i ] == L':' )
            {
                return;
            }
        }
    }
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
void RemoveFileTypeA ( char *pFilename )
{
    size_t  len = strlen (pFilename);
    if ( len > 0 )
    {
        for ( int i = ( int) len - 1; i >= 0; i-- )
        {
            if ( pFilename [ i ] == '.' )
            {
                pFilename [ i ] = '\0';
                return;
            }

            if ( pFilename [ i ] == '\\' )
            {
                return;
            }

            if ( pFilename [ i ] == ':' )
            {
                return;
            }
        }
    }
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
void RemoveFileNameW ( WCHAR *pFilename )
{
    size_t  len = wcslen (pFilename);
    if ( len > 0 )
    {
        for ( int i = ( int) len - 1; i >= 0; i-- )
        {
            if ( pFilename [ i ] == L'\\' )
            {
                pFilename [ i + 1 ] = NULL;
                return;
            }

            if ( pFilename [ i ] == L':' )
            {
                pFilename [ i + 1 ] = NULL;
                return;
            }
        }
    }

    //
    //      Only A Filename : Reset to nothing
    *pFilename = L'\0';
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
void RemoveFileNameA ( char *pFilename )
{
    size_t  len = strlen (pFilename);
    if ( len > 0 )
    {
        for ( int i = ( int) len - 1; i >= 0; i-- )
        {
            if ( pFilename [ i ] == '\\' )
            {
                pFilename [ i + 1 ] = NULL;
                return;
            }

            if ( pFilename [ i ] == ':' )
            {
                pFilename [ i + 1 ] = NULL;
                return;
            }
        }
    }

    //
    //      Only A Filename : Reset to nothing
    *pFilename = '\0';
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
void RemoveCRLFW ( WCHAR *pLine )
{
    size_t  len = wcslen (pLine);
    if ( len > 0 )
    {
        for ( int i = ( int ) len - 1; i >= 0; i-- )
        {
            if ( pLine [ i ] == L'\r' || pLine [ i ] == L'\n' )
            {
                pLine [ i ] = L'\0';
            }
            else
            {
                return;
            }
        }
    }
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
void RemoveCRLFA ( char *pLine )
{
    size_t  len = strlen (pLine);
    if ( len > 0 )
    {
        for ( int i = ( int ) len - 1; i >= 0; i-- )
        {
            if ( pLine [ i ] == '\r' || pLine [ i ] == '\n' )
            {
                pLine [ i ] = L'\0';
            }
            else
            {
                return;
            }
        }
    }
}

//
////////////////////////////////////////////////////////////////////////
//  Find Type
////////////////////////////////////////////////////////////////////////
const WCHAR * FindFileTypeW ( const WCHAR *line )
{
    for ( int iX = ( int ) wcslen ( line ) - 1; iX >= 0; iX-- )
    {
        if ( line [ iX ] == L'.' )
        {
            return line + iX;
        }

        if ( line [ iX ] == L':' )
        {
            return L".";
        }

        if ( line [ iX ] == L'\\' )
        {
            return L".";
        }
    }

    return L".";
}

//
////////////////////////////////////////////////////////////////////////
//  Find Type
////////////////////////////////////////////////////////////////////////
const char * FindFileTypeA ( const char *line )
{
    for ( int iX = ( int ) strlen ( line ) - 1; iX >= 0; iX-- )
    {
        if ( line [ iX ] == L'.' )
        {
            return line + iX;
        }

        if ( line [ iX ] == L':' )
        {
            return ".";
        }

        if ( line [ iX ] == L'\\' )
        {
            return ".";
        }
    }

    return ".";
}

//
////////////////////////////////////////////////////////////////////////
//  Find Type
////////////////////////////////////////////////////////////////////////
const WCHAR * GetFileTypeW ( const WCHAR *line )
{
    for ( int iX = ( int ) wcslen ( line ) - 1; iX >= 0; iX-- )
    {
        if ( line [ iX ] == L'.' )
        {
            return line + iX + 1;
        }

        if ( line [ iX ] == L':' )
        {
            return L"";
        }

        if ( line [ iX ] == L'\\' )
        {
            return L"";
        }
    }

    return L"";
}

//
////////////////////////////////////////////////////////////////////////
//  Find Type
////////////////////////////////////////////////////////////////////////
const char * GetFileTypeA ( const char *line )
{
    for ( int iX = ( int ) strlen ( line ) - 1; iX >= 0; iX-- )
    {
        if ( line [ iX ] == L'.' )
        {
            return line + iX + 1;
        }

        if ( line [ iX ] == L':' )
        {
            return "";
        }

        if ( line [ iX ] == L'\\' )
        {
            return "";
        }
    }

    return "";
}

//
////////////////////////////////////////////////////////////////////////
//  Find Name
////////////////////////////////////////////////////////////////////////
const WCHAR * FindFileNameW ( const WCHAR *line )
{
    for ( int iX = ( int ) wcslen ( line ) - 1; iX >= 0; iX-- )
    {
        if ( line [ iX ] == L':' )
        {
            return line + iX + 1;
        }

        if ( line [ iX ] == L'\\' )
        {
            return line + iX + 1;
        }
    }

    return line;
}

//
////////////////////////////////////////////////////////////////////////
//  Find Name
////////////////////////////////////////////////////////////////////////
const char * FindFileNameA ( const char *line )
{
    for ( int iX = ( int ) strlen ( line ) - 1; iX >= 0; iX-- )
    {
        if ( line [ iX ] == ':' )
        {
            return line + iX + 1;
        }

        if ( line [ iX ] == '\\' )
        {
            return line + iX + 1;
        }
    }

    return line;
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
bool EndsWithIW ( const WCHAR *line,  const WCHAR *string )
{
    if ( wcslen ( line ) >= wcslen ( string ) )
    {
        size_t delta = wcslen ( line ) - wcslen ( string );
        if ( _wcsicmp ( line + delta, string ) == 0 )
        {
            return true;
        }
    }

    return false;
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
bool EndsWithIA ( const char *line,  const char *string )
{
    if ( strlen ( line ) >= strlen ( string ) )
    {
        size_t delta = strlen ( line ) - strlen ( string );
        if ( _stricmp ( line + delta, string ) == 0 )
        {
            return true;
        }
    }

    return false;
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
bool EndsWithW ( const WCHAR *line,  const WCHAR *string )
{
    if ( wcslen ( line ) >= wcslen ( string ) )
    {
        size_t delta = wcslen ( line ) - wcslen ( string );
        if ( wcscmp ( line + delta, string ) == 0 )
        {
            return true;
        }
    }

    return false;
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
bool EndsWithA ( const char *line,  const char *string )
{
    if ( strlen ( line ) >= strlen ( string ) )
    {
        size_t delta = strlen ( line ) - strlen ( string );
        if ( strcmp ( line + delta, string ) == 0 )
        {
            return true;
        }
    }

    return false;
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
bool EndsWithW ( WCHAR *line,  const WCHAR byte )
{
    if ( wcslen ( line ) >= 1 )
    {
        size_t last = wcslen ( line ) - 1;
        if ( line [ last ] == byte )
        {
            return true;
        }
    }

    return false;
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
bool EndsWithA ( char *line,  const char byte )
{
    if ( strlen ( line ) >= 1 )
    {
        size_t last = strlen ( line ) - 1;
        if ( line [ last ] == byte )
        {
            return true;
        }
    }

    return false;
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
bool EndsWithIW ( WCHAR *line,  const WCHAR byte )
{
    if ( wcslen ( line ) >= 1 )
    {
        size_t last = wcslen ( line ) - 1;
        if ( _totupper(line [ last ] ) == _totupper(byte) )
        {
            return true;
        }
    }

    return false;
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
bool EndsWithIA ( char *line,  const char byte )
{
    if ( strlen ( line ) >= 1 )
    {
        size_t last = strlen ( line ) - 1;
        if ( line [ last ] == byte )
        {
            return true;
        }
    }

    return false;
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
bool StartsWithIW ( const WCHAR *line,  const WCHAR *string )
{
    if ( _wcsnicmp ( line, string, wcslen(string) ) == 0 )
    {
        return true;
    }

    return false;
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
bool StartsWithIA ( const char *line,  const char *string )
{
    if ( _strnicmp ( line, string, strlen(string) ) == 0 )
    {
        return true;
    }

    return false;
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
bool StartsWithW ( const WCHAR *line,  const WCHAR *string )
{
    if ( wcsncmp ( line, string, wcslen(string) ) == 0 )
    {
        return true;
    }

    return false;
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
bool StartsWithA ( const char *line,  const char *string )
{
    if ( strncmp ( line, string, strlen(string) ) == 0 )
    {
        return true;
    }

    return false;
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
bool StartsWithIW ( const WCHAR *line,  const WCHAR byte )
{
    if ( _totupper(*line) == _totupper(byte)  )
    {
        return true;
    }

    return false;
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
bool StartsWithIA ( const char *line,  const char byte )
{
    if ( toupper(*line) == toupper(byte)  )
    {
        return true;
    }

    return false;
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
bool StartsWithW ( const WCHAR *line,  const WCHAR byte )
{
    if ( *line == byte )
    {
        return true;
    }

    return false;
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
bool StartsWithA ( const char *line,  const char byte )
{
    if ( *line == byte )
    {
        return true;
    }

    return false;
}

//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
BOOL RemoveEnclosingW ( WCHAR *pText, WCHAR quote )
{
    if ( pText != NULL && wcslen(pText) >= 2 && pText [ 0 ] == quote && pText [ wcslen(pText) - 1 ] == quote )
    {
        wmemmove_s ( pText, wcslen(pText) + 1, pText + 1, wcslen ( pText ) );
        pText [ wcslen(pText) - 1 ] = L'\0';
        return TRUE;
    }

    return FALSE;
}

//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
BOOL RemoveEnclosingW ( WCHAR *pText )
{
    return RemoveEnclosingW ( pText, L'\'' );
}

//
//////////////////////////////////////////////////////////////////////////////
//  Remove Simple QUote
//////////////////////////////////////////////////////////////////////////////
BOOL RemoveEnclosingSQuoteW ( WCHAR *pText )
{
    return RemoveEnclosingW ( pText, L'\'' );
}

//
//////////////////////////////////////////////////////////////////////////////
//  Remove Double QUote
//////////////////////////////////////////////////////////////////////////////
BOOL RemoveEnclosingDQuoteW ( WCHAR *pText )
{
    return RemoveEnclosingW ( pText, L'"' );
}

//
//////////////////////////////////////////////////////////////////////////////
//  Remove Simple and Double QUote
//////////////////////////////////////////////////////////////////////////////
BOOL RemoveEnclosingQuoteW ( WCHAR *pText )
{
    BOOL bResult = RemoveEnclosingSQuoteW( pText );
    if ( ! bResult )
    {
        bResult |= RemoveEnclosingDQuoteW( pText );
    }
    return TRUE;
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
BOOL RemoveEnclosingW( WCHAR *line, WCHAR *tuple )
{
    BOOL bResult = FALSE;
    
    if ( line != NULL )
    {
        if ( StartsWithW ( line, tuple[0] ) && EndsWithW ( line, tuple[1] ) )
        {
            size_t  len = wcslen ( line );

            //
            //  The case where at least a start and an ending quote
            if ( len >= 2 )
            {
                wmemmove_s ( line, len + 1, line + 1, len );
                line [ len - 2 ] = '\0';
                line [ len - 1 ] = '\0';
                bResult = TRUE;
            }
        }
    }
    return bResult;
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
BOOL RemoveEnclosingA( char *line, char *tuple )
{
    BOOL bResult = FALSE;
    if ( line != NULL )
    {
        if ( StartsWithA ( line, tuple[0] ) && EndsWithA ( line, tuple[1] ) )
        {
            size_t  len = strlen ( line );

            //
            //  The case where at least a start and an ending quote
            if ( len >= 2 )
            {
                memmove_s ( line, len + 1, line + 1, len );
                line [ len - 2 ] = '\0';
                line [ len - 1 ] = '\0';
                bResult = TRUE;
            }
        }
    }
    return bResult;
}

//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
BOOL RemoveEnclosingA ( char *pText, char quote )
{
    if ( pText != NULL && strlen(pText) >= 2 && pText [ 0 ] == quote && pText [ strlen(pText) - 1 ] == quote )
    {
        memmove_s ( pText, strlen(pText) + 1, pText + 1, strlen ( pText ) );
        pText [ strlen(pText) - 1 ] = '\0';
        return TRUE;
    }

    return FALSE;
}

//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
BOOL RemoveEnclosingA ( char *pText )
{
    return RemoveEnclosingA ( pText, '\'' );
}

//
//////////////////////////////////////////////////////////////////////////////
//  Remove Simple QUote
//////////////////////////////////////////////////////////////////////////////
BOOL RemoveEnclosingSQuoteA ( char *pText )
{
    return RemoveEnclosingA ( pText, '\'' );
}

//
//////////////////////////////////////////////////////////////////////////////
//  Remove Double QUote
//////////////////////////////////////////////////////////////////////////////
BOOL RemoveEnclosingDQuoteA ( char *pText )
{
    return RemoveEnclosingA ( pText, '"' );
}

//
//////////////////////////////////////////////////////////////////////////////
//  Remove Simple and Double QUote
//////////////////////////////////////////////////////////////////////////////
BOOL RemoveEnclosingQuoteA ( char *pText )
{
    BOOL bResult = RemoveEnclosingSQuoteA ( pText );
    if ( ! bResult )
    {
        bResult |= RemoveEnclosingDQuoteA ( pText );
    }
    return TRUE;
}

//
////////////////////////////////////////////////////////////////////////
//  Remove Leading Byte
////////////////////////////////////////////////////////////////////////
void RemoveLeadingByteW ( WCHAR *line, WCHAR byte )
{
    if ( line != NULL )
    {
        if ( line [ 0 ] != L'\0' && line [ 0 ] == byte )
        {
            wmemmove_s ( line, wcslen ( line ) + 1, line + 1, wcslen ( line + 1 ) + 1 );
        }
    }
}

//
////////////////////////////////////////////////////////////////////////
//  Remove All Leading Bytes
////////////////////////////////////////////////////////////////////////
void RemoveLeadingBytesW ( WCHAR *line, WCHAR byte )
{
    if ( line != NULL )
    {
        while ( line [ 0 ] != L'\0' && line [ 0 ] == byte )
        {
            wmemmove_s ( line, wcslen ( line ) + 1, line + 1, wcslen ( line + 1 ) + 1 );
        }
    }
}

//
////////////////////////////////////////////////////////////////////////
//  Remove Traiting Byte
////////////////////////////////////////////////////////////////////////
void RemoveTrailingByteW ( WCHAR *line, WCHAR byte )
{
    for ( int iX =  ( int ) wcslen ( line ) - 1; iX >= 0; iX-- )
    {
        if ( line [ iX ] != byte )
        {
            return;
        }

        line [ iX ] = L'\0';

        return;
    }
}

//
////////////////////////////////////////////////////////////////////////
//  Remove All Traiting Bytes
////////////////////////////////////////////////////////////////////////
void RemoveTrailingBytesW ( WCHAR *line, WCHAR byte )
{
    for ( int iX =  ( int ) wcslen ( line ) - 1; iX >= 0; iX-- )
    {
        if ( line [ iX ] != byte )
        {
            return;
        }

        line [ iX ] = L'\0';
    }
}

//
////////////////////////////////////////////////////////////////////////
//  Remove All Traiting Bytes
////////////////////////////////////////////////////////////////////////
void RemoveTrailingBytesW ( WCHAR *line, WCHAR *bytes )
{
    for ( int iX = ( int ) wcslen ( line ) - 1; iX >= 0; iX-- )
    {
        bool bFound = false;
        for ( size_t i = 0; i < wcslen(bytes); i++ )
        {
            if ( line [ iX ] == bytes [ i ] )
            {
                bFound = true;
                break;
            }
        }

        //
        if ( ! bFound )
        {
            return;
        }

        //
        line [ iX ] = L'\0';
    }
}

//
////////////////////////////////////////////////////////////////////////
//  Remove Leading byte
////////////////////////////////////////////////////////////////////////
void RemoveLeadingByteA ( char *line, char byte )
{
    if ( line != NULL )
    {
        if ( line [ 0 ] != '\0' && line [ 0 ] == byte )
        {
            memmove_s ( line, strlen ( line ) + 1, line + 1, strlen ( line + 1 ) + 1 );
        }
    }
}

//
////////////////////////////////////////////////////////////////////////
//  Remove All Leading bytes
////////////////////////////////////////////////////////////////////////
void RemoveLeadingBytesA ( char *line, char byte )
{
    if ( line != NULL )
    {
        while ( line [ 0 ] != '\0' && line [ 0 ] == byte )
        {
            memmove_s ( line, strlen ( line ) + 1, line + 1, strlen ( line + 1 ) + 1 );
        }
    }
}

//
////////////////////////////////////////////////////////////////////////
//  Remove Trailing  byte
////////////////////////////////////////////////////////////////////////
void RemoveTrailingByteA ( char *line, char byte )
{
    for ( int iX = ( int ) strlen ( line ) - 1; iX >= 0; iX-- )
    {
        if ( line [ iX ] != byte )
        {
            return;
        }

        line [ iX ] = '\0';

        //  After One return
        return;
    }
}

//
////////////////////////////////////////////////////////////////////////
//  Remove All Trailing  bytes
////////////////////////////////////////////////////////////////////////
void RemoveTrailingBytesA ( char *line, char byte )
{
    for ( int iX = ( int ) strlen ( line ) - 1; iX >= 0; iX-- )
    {
        if ( line [ iX ] != byte )
        {
            return;
        }

        line [ iX ] = '\0';
    }
}

//
////////////////////////////////////////////////////////////////////////
//  Remove All Traiting Bytes
////////////////////////////////////////////////////////////////////////
void RemoveTrailingBytesA ( char *line, char *bytes )
{
    for ( int iX = ( int ) strlen ( line ) - 1; iX >= 0; iX-- )
    {
        bool bFound = false;
        for ( size_t i = 0; i < strlen(bytes); i++ )
        {
            if ( line [ iX ] == bytes [ i ] )
            {
                bFound = true;
                break;
            }
        }

        //
        if ( ! bFound )
        {
            return;
        }

        //
        line [ iX ] = L'\0';
    }
}

//
////////////////////////////////////////////////////////////////////////
//  Capitalize
////////////////////////////////////////////////////////////////////////
void CapitalizeStringW ( WCHAR *line, size_t iSize )
{
    bool    toUpper = true;
    while ( *line != 0 && iSize > 0 )
    {
        WCHAR octet = *line;

        if ( toUpper )
        {
            *line = _totupper ( octet );
        }
        else
        {
            *line = _totlower ( octet );
        }

        //
        //      If it is a character
        //      next byte will be lower
        if ( ( octet >= L'A' && octet >= L'Z' ) || ( octet >= L'a' && octet >= L'z' ) || octet == L'\'' )
        {
            toUpper = false;
        }
        else
        {
            toUpper = true;
        }

        line++;
        iSize++;
    }
}

//
////////////////////////////////////////////////////////////////////////
//  Capitalize
////////////////////////////////////////////////////////////////////////
void CapitalizeStringW ( WCHAR *line )
{
    CapitalizeStringW ( line, wcslen(line) );
}

//
////////////////////////////////////////////////////////////////////////
//  Capitalize
////////////////////////////////////////////////////////////////////////
void CapitalizeStringA ( char *line, size_t iSize )
{
    bool    toUpper = true;
    while ( *line != 0 && iSize > 0 )
    {
        char octet = *line;

        if ( toUpper )
        {
            *line = toupper ( octet );
        }
        else
        {
            *line = tolower ( octet );
        }

        //
        //      If it is a character
        //      next byte will be lower
        if ( ( octet >= 'A' && octet >= 'Z' ) || ( octet >= 'a' && octet >= 'z' ) || octet == '\'' )
        {
            toUpper = false;
        }
        else
        {
            toUpper = true;
        }

        line++;
        iSize++;
    }
}

//
////////////////////////////////////////////////////////////////////////
//  Capitalize
////////////////////////////////////////////////////////////////////////
void CapitalizeStringA ( char *line )
{
    CapitalizeStringA ( line, strlen(line) );
}

//
////////////////////////////////////////////////////////////////////////
//  Uppercase
////////////////////////////////////////////////////////////////////////
void UpperCaseStringW ( WCHAR *line, size_t iSize )
{
    while ( *line != 0 && iSize > 0 )
    {
        *line = _totupper ( *line );
        line++;
        iSize++;
    }
}

//
////////////////////////////////////////////////////////////////////////
//  Uppercase
////////////////////////////////////////////////////////////////////////
void UpperCaseStringW ( WCHAR *line )
{
    UpperCaseStringW ( line, wcslen(line) );
}

//
////////////////////////////////////////////////////////////////////////
//  Lowercase
////////////////////////////////////////////////////////////////////////
void LowerCaseStringW ( WCHAR *line, size_t iSize )
{
    while ( *line != 0 && iSize > 0 )
    {
        *line = _totlower ( *line );
        line++;
        iSize++;
    }
}

//
////////////////////////////////////////////////////////////////////////
//  Uppercase
////////////////////////////////////////////////////////////////////////
void LowerCaseStringW ( WCHAR *line )
{
    LowerCaseStringW ( line, wcslen(line) );
}

//
////////////////////////////////////////////////////////////////////////
//  Uppercase
////////////////////////////////////////////////////////////////////////
void UpperCaseStringA ( char *line, size_t iSize )
{
    while ( *line != 0 && iSize > 0 )
    {
        *line = toupper ( *line );
        line++;
        iSize++;
    }
}

//
////////////////////////////////////////////////////////////////////////
//  Uppercase
////////////////////////////////////////////////////////////////////////
void UpperCaseStringA ( char *line )
{
    UpperCaseStringA ( line, strlen(line) );
}

//
////////////////////////////////////////////////////////////////////////
//  Lowercase
////////////////////////////////////////////////////////////////////////
void LowerCaseStringA ( char *line, size_t iSize )
{
    while ( *line != 0 && iSize > 0 )
    {
        *line = tolower ( *line );
        line++;
        iSize++;
    }
}

//
////////////////////////////////////////////////////////////////////////
//  Lowercase
////////////////////////////////////////////////////////////////////////
void LowerCaseStringA ( char *line )
{
    LowerCaseStringA ( line, strlen(line) );
}

//
////////////////////////////////////////////////////////////////////////
//  Is Alphabetic
////////////////////////////////////////////////////////////////////////
bool IsAlphaBeticW ( const WCHAR byte )
{
    if (    ( byte >= L'A' && byte <= L'Z' ) ||
            ( byte >= L'a' && byte <= L'z' )    )
    {
        return true;
    }

    return false;
}

//
////////////////////////////////////////////////////////////////////////
//  Is Alphabetic
////////////////////////////////////////////////////////////////////////
bool IsAlphaBeticW ( const WCHAR *pString )
{
    while ( *pString )
    {
        if ( ! IsAlphaBeticW ( *pString )   )
        {
            return false;
        }
        pString++;
    }

    return true;
}

//
////////////////////////////////////////////////////////////////////////
//  Is Alphabetic
////////////////////////////////////////////////////////////////////////
bool IsAlphaBeticA ( const char byte )
{
    if (    ( byte >= 'A' && byte <= 'Z' ) ||
            ( byte >= 'a' && byte <= 'z' )  )
    {
        return true;
    }

    return false;
}

//
////////////////////////////////////////////////////////////////////////
//  Is Alphabetic
////////////////////////////////////////////////////////////////////////
bool IsAlphaBeticA ( const char *pString )
{
    while ( *pString )
    {
        if ( ! IsAlphaBeticA ( *pString )   )
        {
            return false;
        }
        pString++;
    }

    return true;
}

//
////////////////////////////////////////////////////////////////////////
//  Is Numeric
////////////////////////////////////////////////////////////////////////
bool IsNumericW ( const WCHAR byte )
{
    if ( byte >= L'0' && byte <= L'9' )
    {
        return true;
    }

    return false;
}

//
////////////////////////////////////////////////////////////////////////
//  Is Numeric
////////////////////////////////////////////////////////////////////////
bool IsNumericW ( const WCHAR *pString )
{
    while ( *pString  != L'\0' )
    {
        if ( ! IsNumericW ( *pString )  )
        {
            return false;
        }
        pString++;
    }

    return true;
}

//
////////////////////////////////////////////////////////////////////////
//  Is Numeric Signed
////////////////////////////////////////////////////////////////////////
bool IsNumericSignedW ( const WCHAR byte )
{
    if ( ( byte >= L'0' && byte <= L'9' ) || byte == L'+' || byte == L'-' )
    {
        return true;
    }

    return false;
}

//
////////////////////////////////////////////////////////////////////////
//  Is Numeric Signed
////////////////////////////////////////////////////////////////////////
bool IsNumericSignedW ( const WCHAR *pString )
{
    if ( *pString == L'+' || *pString == L'-' )
    {
        pString++;
    }

    while ( *pString  != L'\0' )
    {
        if ( ! IsNumericW ( *pString )  )
        {
            return false;
        }
        pString++;
    }

    return true;
}

//
////////////////////////////////////////////////////////////////////////
//  Is Numeric
////////////////////////////////////////////////////////////////////////
bool IsNumericA ( const char byte )
{
    if ( byte >= '0' && byte <= '9' )
    {
        return true;
    }

    return false;
}

//
////////////////////////////////////////////////////////////////////////
//  Is Numeric
////////////////////////////////////////////////////////////////////////
bool IsNumericA ( const char *pString )
{
    while ( *pString  != '\0' )
    {
        if ( ! IsNumericA ( *pString )  )
        {
            return false;
        }
        pString++;
    }

    return true;
}

//
////////////////////////////////////////////////////////////////////////
//  Is Numeric Signed
////////////////////////////////////////////////////////////////////////
bool IsNumericSignedA ( const char byte )
{
    if ( ( byte >= '0' && byte <= '9' ) || byte == '+' || byte == '-' )
    {
        return true;
    }

    return false;
}

//
////////////////////////////////////////////////////////////////////////
//  Is Numeric Signed
////////////////////////////////////////////////////////////////////////
bool IsNumericSignedA ( const char *pString )
{
    if ( *pString == '+' || *pString == '-' )
    {
        pString++;
    }

    while ( *pString != '\0' )
    {
        if ( ! IsNumericA ( *pString )  )
        {
            return false;
        }
        pString++;
    }

    return true;
}

//
////////////////////////////////////////////////////////////////////////
//  Is In Range : return true if it is inside
////////////////////////////////////////////////////////////////////////
bool IsInRangeW ( const WCHAR byte, const WCHAR *tuple1, const WCHAR *included, const WCHAR *excluded )
{
    if ( excluded != NULL && wcslen(excluded) > 0 )
    {
        for ( size_t i = 0; i < wcslen(excluded); i++ )
        {
            if ( byte == excluded [ i ] )
            {
                return false;
            }
        }
    }

    bool bInside = false;
    size_t nbRanges = wcslen(tuple1) / 2;
    if ( nbRanges > 0 )
    {
        for ( size_t i = 0; i < nbRanges; i++ )
        {
            size_t index = 2 * i;
            if ( byte >= tuple1 [ index ] && byte <= tuple1 [ index + 1 ] )
            {
                bInside = true;
                break;
            }
        }
    }

    if ( ! bInside && included != NULL && wcslen(included) > 0 )
    {
        for ( size_t i = 0; i < wcslen(included); i++ )
        {
            if ( byte == included [ i ] )
            {
                bInside = true;
                break;
            }
        }
    }

    return bInside;
}

//
////////////////////////////////////////////////////////////////////////
//  Is In Range : return true if it is inside
////////////////////////////////////////////////////////////////////////
bool IsInRangeA ( const char byte, const char *tuple1, const char *included, const char *excluded )
{
    if ( excluded != NULL && strlen(excluded) > 0 )
    {
        for ( size_t i = 0; i < strlen(excluded); i++ )
        {
            if ( byte == excluded [ i ] )
            {
                return false;
            }
        }
    }

    bool bInside = false;
    size_t nbRanges = strlen(tuple1) / 2;
    if ( nbRanges > 0 )
    {
        for ( size_t i = 0; i < nbRanges; i++ )
        {
            size_t index = 2 * i;
            if ( byte >= tuple1 [ index ] && byte <= tuple1 [ index + 1 ] )
            {
                bInside = true;
            }
        }
    }

    if ( ! bInside && included != NULL && strlen(included) > 0 )
    {
        for ( size_t i = 0; i < strlen(included); i++ )
        {
            if ( byte == included [ i ] )
            {
                bInside = true;
                break;
            }
        }
    }

    return bInside;
}

//
////////////////////////////////////////////////////////////////////////
//  Is Not In Range : return true if it is outside
////////////////////////////////////////////////////////////////////////
bool IsNotInRangeW ( const WCHAR byte, const WCHAR *tuple1, const WCHAR *included, const WCHAR *excluded )
{
    if ( excluded != NULL && wcslen(excluded) > 0 )
    {
        for ( size_t i = 0; i < wcslen(excluded); i++ )
        {
            if ( byte == excluded [ i ] )
            {
                return true;
            }
        }
    }

    bool bOutside = true;
    size_t nbRanges = wcslen(tuple1) / 2;
    if ( nbRanges > 0 )
    {
        for ( size_t i = 0; i < nbRanges; i++ )
        {
            size_t index = 2 * i;
            if ( byte >= tuple1 [ index ] && byte <= tuple1 [ index + 1 ] )
            {
                bOutside = false;
                break;
            }
        }
    }

    if ( bOutside && included != NULL && wcslen(included) > 0 )
    {
        for ( size_t i = 0; i < wcslen(included); i++ )
        {
            if ( byte == included [ i ] )
            {
                bOutside = false;
                break;
            }
        }
    }

    return bOutside;
}

//
////////////////////////////////////////////////////////////////////////
//  Is Not In Range : return true if it is outside
////////////////////////////////////////////////////////////////////////
bool IsNotInRangeA ( const char byte, const char *tuple1, const char *included, const char *excluded )
{
    if ( excluded != NULL && strlen(excluded) > 0 )
    {
        for ( size_t i = 0; i < strlen(excluded); i++ )
        {
            if ( byte == excluded [ i ] )
            {
                return true;
            }
        }
    }

    bool bOutside = true;
    size_t nbRanges = strlen(tuple1) / 2;
    if ( nbRanges > 0 )
    {
        for ( size_t i = 0; i < nbRanges; i++ )
        {
            size_t index = 2 * i;
            if ( byte >= tuple1 [ index ] && byte <= tuple1 [ index + 1 ] )
            {
                bOutside = false;
                break;
            }
        }
    }

    if ( bOutside && included != NULL && strlen(included) > 0 )
    {
        for ( size_t i = 0; i < strlen(included); i++ )
        {
            if ( byte == included [ i ] )
            {
                bOutside = false;
                break;
            }
        }
    }
    return bOutside;
}

//
////////////////////////////////////////////////////////////////////////
//  Number of NUmeric
////////////////////////////////////////////////////////////////////////
int InRangeLengthW ( const WCHAR *pString, const WCHAR *tuple1, const WCHAR *included, const WCHAR *excluded )
{
    int iLength = 0;
    while ( *pString != L'\0' )
    {
        bool bInside = IsInRangeW ( *pString, tuple1, included, excluded );

        //  Not Inside So return
        if ( ! bInside  )
        {
            return iLength;
        }

        iLength++;
        pString++;
    }

    return iLength;
}

//
////////////////////////////////////////////////////////////////////////
//  Number of NUmeric
////////////////////////////////////////////////////////////////////////
int InRangeLengthA ( const char *pString, const char *tuple1, const char *included, const char *excluded )
{
    int iLength = 0;
    while ( *pString != '\0' )
    {
        bool bInside = IsInRangeA ( *pString, tuple1, included, excluded );

        //  Not Inside So return
        if ( ! bInside  )
        {
            return iLength;
        }

        iLength++;
        pString++;
    }

    return iLength;
}

//
////////////////////////////////////////////////////////////////////////
//  Number of Not Inside a range
////////////////////////////////////////////////////////////////////////
int NotInRangeLengthW ( const WCHAR *pString, const WCHAR *tuple1, const WCHAR *included, const WCHAR *excluded )
{
    int iLength = 0;
    while ( *pString != L'\0' )
    {
        bool bOutside = IsNotInRangeW ( *pString, tuple1, included, excluded );

        //  Not Outside so return
        if ( ! bOutside )
        {
            return iLength;
        }

        iLength++;
        pString++;
    }

    return iLength;
}

//
////////////////////////////////////////////////////////////////////////
//  Number of Not Inside a range
////////////////////////////////////////////////////////////////////////
int NotInRangeLengthA ( const char *pString, const char *tuple1, const char *included, const char *excluded )
{
    int iLength = 0;
    while ( *pString != '\0' )
    {
        bool bOutside = IsNotInRangeA ( *pString, tuple1, included, excluded );

        //  Not Outside : return
        if ( ! bOutside )
        {
            return iLength;
        }

        iLength++;
        pString++;
    }

    return iLength;
}

//
////////////////////////////////////////////////////////////////////////
//  Next Not In the Set of chars
////////////////////////////////////////////////////////////////////////
WCHAR *NextNotInW ( WCHAR *pString, const WCHAR *pChars )
{
    return NextNotInRangeW ( pString, L"", pChars, L"" );
}

//
////////////////////////////////////////////////////////////////////////
//  Next Not In the Set of chars
////////////////////////////////////////////////////////////////////////
const WCHAR *NextNotInW ( const WCHAR *pString, const WCHAR *pChars )
{
    return NextNotInRangeW ( pString, L"", pChars, L"" );
}

//
////////////////////////////////////////////////////////////////////////
//  Next Not In the Set of chars
////////////////////////////////////////////////////////////////////////
char *NextNotInA ( char *pString, const char *pChars )
{
    return NextNotInRangeA ( pString, "", pChars, "" );
}

//
////////////////////////////////////////////////////////////////////////
//  Next Not In the Set of chars
////////////////////////////////////////////////////////////////////////
const char *NextNotInA ( const char *pString, const char *pChars )
{
    return NextNotInRangeA ( pString, "", pChars, "" );
}

//
////////////////////////////////////////////////////////////////////////
//  Net Not In range of tuple1
////////////////////////////////////////////////////////////////////////
WCHAR *NextNotInRangeW ( WCHAR *pString, const WCHAR *tuple1, const WCHAR *included, const WCHAR *excluded )
{
    //  Number of Tuples
    size_t nTuples = wcslen(tuple1) / 2;

    while ( *pString != L'\0' )
    {
        bool bOutside = IsNotInRangeW ( *pString, tuple1, included, excluded );

        //  Not In Range
        if ( bOutside )
        {
            return pString;
        }
        pString++;
    }
    return pString;
}

//
////////////////////////////////////////////////////////////////////////
//  Net Not In range of tuple1
////////////////////////////////////////////////////////////////////////
const WCHAR *NextNotInRangeW ( const WCHAR *pString, const WCHAR *tuple1, const WCHAR *included, const WCHAR *excluded  )
{
    //  Number of Tuples
    size_t nTuples = wcslen(tuple1) / 2;

    while ( *pString != L'\0' )
    {
        bool bOutside = IsNotInRangeW ( *pString, tuple1, included, excluded );

        //  Not In Range
        if ( bOutside )
        {
            return pString;
        }
        pString++;
    }
    return pString;
}

//
////////////////////////////////////////////////////////////////////////
//  Net Not In range of tuple1
////////////////////////////////////////////////////////////////////////
char *NextNotInRangeA ( char *pString, const char *tuple1, const char *included, const char *excluded )
{
    //  Number of Tuples
    size_t nTuples = strlen(tuple1) / 2;

    //
    while ( *pString != '\0' )
    {
        bool bOutside = IsNotInRangeA ( *pString, tuple1, included, excluded );

        //  Not In Range
        if ( bOutside )
        {
            return pString;
        }
        pString++;
    }
    return pString;
}

//
////////////////////////////////////////////////////////////////////////
//  Net Not In range of tuple1
////////////////////////////////////////////////////////////////////////
const char *NextNotInRangeA ( const char *pString, const char *tuple1, const char *included, const char *excluded )
{
    //  Number of Tuples
    size_t nTuples = strlen(tuple1) / 2;

    //
    while ( *pString != '\0' )
    {
        bool bOutside = IsNotInRangeA ( *pString, tuple1, included, excluded );

        //  Not In Range
        if ( bOutside )
        {
            return pString;
        }
        pString++;
    }
    return pString;
}

//
////////////////////////////////////////////////////////////////////////
//  Next In the Set of chars
////////////////////////////////////////////////////////////////////////
WCHAR *NextInW ( WCHAR *pString, const WCHAR *pChars )
{
    return NextInRangeW ( pString, L"", pChars );
}

//
////////////////////////////////////////////////////////////////////////
//  Next In the Set of chars
////////////////////////////////////////////////////////////////////////
const WCHAR *NextInW ( const WCHAR *pString, const WCHAR *pChars )
{
    return NextInRangeW ( pString, L"", pChars );
}

//
////////////////////////////////////////////////////////////////////////
//  Next In the Set of chars
////////////////////////////////////////////////////////////////////////
char *NextInA ( char *pString, const char *pChars )
{
    return NextInRangeA ( pString, "", pChars );
}

//
////////////////////////////////////////////////////////////////////////
//  Next In the Set of chars
////////////////////////////////////////////////////////////////////////
const char *NextInA ( const char *pString, const char *pChars )
{
    return NextInRangeA ( pString, "", pChars );
}

//
////////////////////////////////////////////////////////////////////////
//  Next In Range of tuple1
////////////////////////////////////////////////////////////////////////
WCHAR *NextInRangeW ( WCHAR *pString, const WCHAR *tuple1, const WCHAR *included, const WCHAR *excluded  )
{
    //  Number of Tuples
    size_t nTuples = wcslen(tuple1) / 2;

    //
    while ( *pString != L'\0' )
    {
        bool bInside = IsInRangeW ( *pString, tuple1, included, excluded );

        //  In Range
        if ( bInside )
        {
            return pString;
        }
        pString++;
    }
    return pString;
}

//
////////////////////////////////////////////////////////////////////////
//  Next In Range of tuple1
////////////////////////////////////////////////////////////////////////
const WCHAR *NextInRangeW ( const WCHAR *pString, const WCHAR *tuple1, const WCHAR *included, const WCHAR *excluded )
{
    //  Number of Tuples
    size_t nTuples = wcslen(tuple1) / 2;

    //
    while ( *pString != L'\0' )
    {
        bool bInside = IsInRangeW ( *pString, tuple1, included, excluded );

        //  In Range
        if ( bInside )
        {
            return pString;
        }
        pString++;
    }
    return pString;
}

//
////////////////////////////////////////////////////////////////////////
//  Next In Range of tuple1
////////////////////////////////////////////////////////////////////////
char *NextInRangeA ( char *pString, const char *tuple1, const char *included, const char *excluded)
{
    //  Number of Tuples
    size_t nTuples = strlen(tuple1) / 2;

    while ( *pString != '\0' )
    {
        bool bInside = IsInRangeA ( *pString, tuple1, included, excluded );

        //  In Range
        if ( bInside )
        {
            return pString;
        }
        pString++;
    }

    return pString;
}

//
////////////////////////////////////////////////////////////////////////
//  Next In Range of tuple1
////////////////////////////////////////////////////////////////////////
const char *NextInRangeA ( const char *pString, const char *tuple1, const char *included, const char *excluded)
{
    //  Number of Tuples
    size_t nTuples = strlen(tuple1) / 2;

    while ( *pString != '\0' )
    {
        bool bInside = IsInRangeA ( *pString, tuple1, included, excluded );

        //  In Range
        if ( bInside )
        {
            return pString;
        }
        pString++;
    }

    return pString;
}

//
//====================================================================================
//  Is Floating
//====================================================================================
bool IsFloatingW ( const WCHAR *pText, bool &bFloatingDecimal, int &iNumberDecimal, bool bStrict )
{
    bFloatingDecimal        = false;
    iNumberDecimal          = 0;

    if ( *pText == L'-' )
    {
        pText++;
    }

    //
    int     iDecimal                = 0;

    while ( *pText )
    {
        if ( ! bFloatingDecimal && *pText == L'.' )
        {
            pText++;
            bFloatingDecimal    = true;
        }
        else if ( *pText >= L'0' && *pText <= L'9' )
        {
            if ( bFloatingDecimal )
            {
                iNumberDecimal++;
            }
            pText++;
        }
        else
        {
            return false;
        }
    }

    if ( bStrict )
    {
        return bFloatingDecimal;
    }

    return true;
}

//
//====================================================================================
//  Is Floating
//====================================================================================
bool IsFloatingA ( const char *pText, bool &bFloatingDecimal, int &iNumberDecimal, bool bStrict )
{
    bFloatingDecimal        = false;
    iNumberDecimal          = 0;

    if ( *pText == L'-' )
    {
        pText++;
    }

    //
    bool    bFloatingPointFound     = false;
    int     iDecimal                = 0;

    while ( *pText )
    {
        if ( ! bFloatingDecimal && *pText == '.' )
        {
            pText++;
            bFloatingDecimal    = true;
        }
        else if ( *pText >= '0' && *pText <= '9' )
        {
            if ( bFloatingPointFound )
            {
                iDecimal++;
                if ( iNumberDecimal < iDecimal )
                {
                    iNumberDecimal = iDecimal;
                }
            }
            pText++;
        }
        else
        {
            return false;
        }
    }

    if ( bStrict )
    {
        return bFloatingDecimal;
    }

    return true;
}

//
//====================================================================================
//  Is Floating Strict
//====================================================================================
bool IsFloatingW ( const WCHAR *pText )
{
    bool bFloatingDecimal       = false;
    int iNumberDecimal          = 0;

    return IsFloatingW ( pText, bFloatingDecimal, iNumberDecimal, true );
}

//
//====================================================================================
//  Is Floating Strict
//====================================================================================
bool IsFloatingA ( const char *pText )
{
    bool bFloatingDecimal       = false;
    int iNumberDecimal          = 0;

    return IsFloatingA ( pText, bFloatingDecimal, iNumberDecimal, true );
}

//
//====================================================================================
//  Is Decimal Or Integer
//====================================================================================
bool IsDecimalW ( const WCHAR *pText )
{
    bool bFloatingDecimal       = false;
    int iNumberDecimal          = 0;

    return IsFloatingW ( pText, bFloatingDecimal, iNumberDecimal, false );
}

//
//====================================================================================
//  Is Decimal Or Integer
//====================================================================================
bool IsDecimalA ( const char *pText )
{
    bool bFloatingDecimal       = false;
    int iNumberDecimal          = 0;

    return IsFloatingA ( pText, bFloatingDecimal, iNumberDecimal, false );
}

//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
double RoundValue ( double dfValue )
{
    dfValue += 0.5;
    return dfValue;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
double HumanReadableValueW ( double dfLargeInt, WCHAR *pUnits )
{
    *pUnits = L'b';
    unsigned long long llKilos  = ( unsigned long long ) ( RoundValue ( dfLargeInt / 1024.0f ) );
    double dfKilos              = dfLargeInt / 1024.0f;
    if ( llKilos > 0 )
    {
        *pUnits = L'K';
        dfLargeInt  = dfKilos;
        unsigned long long llMega   =  ( unsigned long long ) ( RoundValue ( dfLargeInt / 1024.0f ) );
        double dfMega               = dfLargeInt / 1024.0f;
        if ( llMega > 0 )
        {
            dfLargeInt  = dfMega;
            *pUnits = L'M';
            unsigned long long llGigas  =  ( unsigned long long ) (  RoundValue ( dfLargeInt / 1024.0f ) );
            double dfGigas              = dfLargeInt / 1024.0f;
            if ( llGigas > 0 )
            {
                dfLargeInt  = dfGigas;
                *pUnits = L'G';
                unsigned long long llTeras  =  ( unsigned long long ) (  RoundValue ( dfLargeInt / 1024.0f ) );
                double dfTeras              = dfLargeInt / 1024.0f;
                if ( llTeras > 0 )
                {
                    dfLargeInt  = dfTeras;
                    *pUnits = L'T';
                }
            }
        }
    }
    return dfLargeInt;
}

//
///////////////////////////////////////////////////////////////////////////////
//  Human Readable
///////////////////////////////////////////////////////////////////////////////
WCHAR *HumanReadableW ( WCHAR *pResult, size_t sizeInChars, double largeInt, int decimal )
{
    ZeroMemory ( pResult, sizeInChars * sizeof(WCHAR) );

    WCHAR Units [ 2 ] = L"b";
    double dfLargeInt = HumanReadableValueW ( (double) largeInt, Units );
    WCHAR szFormat [ 32 ];
    swprintf_s ( szFormat, sizeof(szFormat), L"%%1.%df%%s", decimal );
    swprintf_s ( pResult, sizeInChars, szFormat, dfLargeInt, Units );
    return pResult;
}

//
///////////////////////////////////////////////////////////////////////////////
//  Human Readable
///////////////////////////////////////////////////////////////////////////////
WCHAR *HumanReadableW ( WCHAR *pResult, size_t sizeInChars, unsigned long long largeInt, int decimal )
{
    ZeroMemory ( pResult, sizeInChars * sizeof(WCHAR) );

    WCHAR Units [ 2 ] = L"b";
    double dfLargeInt = HumanReadableValueW ( (double) largeInt, Units );
    WCHAR szFormat [ 32 ];
    swprintf_s ( szFormat, sizeof(szFormat), L"%%1.%df%%s", decimal );
    swprintf_s ( pResult, sizeInChars, szFormat, dfLargeInt, Units );
    return pResult;
}

//
///////////////////////////////////////////////////////////////////////////////
//  Human Readable
///////////////////////////////////////////////////////////////////////////////
WCHAR *HumanReadableW ( WCHAR *pResult, size_t sizeInChars, long long largeInt, int decimal )
{
    ZeroMemory ( pResult, sizeInChars * sizeof(WCHAR) );

    WCHAR   cSign = L' ';
    if ( largeInt < 0 )
    {
        largeInt = abs(largeInt);
        cSign = L'-';
    }

    WCHAR Units [ 2 ] = L"b";
    double dfLargeInt = HumanReadableValueW ( (double) largeInt, Units );
    WCHAR szFormat [ 32 ];
    swprintf_s ( szFormat, sizeof(szFormat) / sizeof(WCHAR), L"%%c%%1.%df%%s", decimal );
    swprintf_s ( pResult, sizeInChars, szFormat, cSign, dfLargeInt, Units );
    return pResult;
}

//
///////////////////////////////////////////////////////////////////////////////
//  Human Readable
///////////////////////////////////////////////////////////////////////////////
WCHAR *HumanReadableW ( WCHAR *pResult, size_t sizeInChars, LARGE_INTEGER largeInt, int decimal )
{
    return HumanReadableW ( pResult, sizeInChars, (double) largeInt.QuadPart, decimal );
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
double HumanReadableValueA ( double dfLargeInt, char *pUnits )
{
    *pUnits = 'b';
    unsigned long long llKilos  = ( unsigned long long ) ( RoundValue ( dfLargeInt / 1024.0f ) );
    double dfKilos              = dfLargeInt / 1024.0f;
    if ( llKilos > 0 )
    {
        *pUnits = 'K';
        dfLargeInt  = dfKilos;
        unsigned long long llMega   =  ( unsigned long long ) ( RoundValue ( dfLargeInt / 1024.0f ) );
        double dfMega               = dfLargeInt / 1024.0f;
        if ( llMega > 0 )
        {
            dfLargeInt  = dfMega;
            *pUnits = 'M';
            unsigned long long llGigas  =  ( unsigned long long ) (  RoundValue ( dfLargeInt / 1024.0f ) );
            double dfGigas              = dfLargeInt / 1024.0f;
            if ( llGigas > 0 )
            {
                dfLargeInt  = dfGigas;
                *pUnits = 'G';
                unsigned long long llTeras  =  ( unsigned long long ) (  RoundValue ( dfLargeInt / 1024.0f ) );
                double dfTeras              = dfLargeInt / 1024.0f;
                if ( llTeras > 0 )
                {
                    dfLargeInt  = dfTeras;
                    *pUnits = 'T';
                }
            }
        }
    }
    return dfLargeInt;
}

//
///////////////////////////////////////////////////////////////////////////////
//  Human Readable
///////////////////////////////////////////////////////////////////////////////
char *HumanReadableA ( char *pResult, size_t sizeInChars, double largeInt, int decimal )
{
    ZeroMemory ( pResult, sizeInChars );
    
    char Units [ 2 ] = "b";
    double dfLargeInt = HumanReadableValueA ( largeInt, Units );
    char szFormat [ 32 ];
    sprintf_s ( szFormat, sizeof(szFormat), "%%1.%df%%s", decimal );
    sprintf_s ( pResult, sizeInChars, szFormat, dfLargeInt, Units );
    return pResult;
}

//
///////////////////////////////////////////////////////////////////////////////
//  Human Readable
///////////////////////////////////////////////////////////////////////////////
char *HumanReadableA ( char *pResult, size_t sizeInChars, unsigned long long largeInt, int decimal )
{
    ZeroMemory ( pResult, sizeInChars );

    char Units [ 2 ] = "b";
    double dfLargeInt = HumanReadableValueA ( ( double) largeInt, Units );
    char szFormat [ 32 ];
    sprintf_s ( szFormat, sizeof(szFormat), "%%1.%df%%s", decimal );
    sprintf_s ( pResult, sizeInChars, szFormat, dfLargeInt, Units );
    return pResult;
}

//
///////////////////////////////////////////////////////////////////////////////
//  Human Readable
///////////////////////////////////////////////////////////////////////////////
char *HumanReadableA ( char *pResult, size_t sizeInChars, long long largeInt, int decimal )
{
    ZeroMemory ( pResult, sizeInChars );

    char    cSign = ' ';
    if ( largeInt < 0 )
    {
        largeInt = abs(largeInt);
        cSign = '-';
    }

    char Units [ 2 ] = "b";
    double dfLargeInt = HumanReadableValueA ( (double) largeInt, Units );
    char szFormat [ 32 ];
    sprintf_s ( szFormat, sizeof(szFormat), "%%c%%1.%df%%s", decimal );
    sprintf_s ( pResult, sizeInChars, "%c%f%s", cSign, dfLargeInt, Units );
    return pResult;

}

//
///////////////////////////////////////////////////////////////////////////////
//  Human Readable
///////////////////////////////////////////////////////////////////////////////
char *HumanReadableA ( char *pResult, size_t sizeInChars, LARGE_INTEGER largeInt, int decimal )
{
    return HumanReadableA ( pResult, sizeInChars, (double) largeInt.QuadPart, decimal );
}

//
///////////////////////////////////////////////////////////////////////////////
//  Separated Number
///////////////////////////////////////////////////////////////////////////////
WCHAR *SeparatedNumberW ( WCHAR *pResult, size_t sizeInChars, unsigned long long largeInt, WCHAR separator )
{
    ZeroMemory ( pResult, sizeInChars * sizeof(WCHAR) );

    const int lenFraction = 10;
    unsigned int fraction  [ lenFraction ];
    for ( int i = 0; i < lenFraction; i++ )
    {
        fraction [ i ] = 0;
    }

    //
    int count = 0;
    while ( largeInt > 0 )
    {
        fraction [ count ]  = largeInt % 1000;
        largeInt            = largeInt / 1000;
        count++;
    }

    //
    bool bFirst = true;
    if ( count > 0 )
    {
        for ( int i = count - 1; i >= 0; i-- )
        {
            if ( bFirst )
            {
                swprintf_s ( pResult + wcslen(pResult), sizeInChars - wcslen(pResult), L"%d", fraction [ i ] );
                bFirst = false;
            }
            else
            {
                swprintf_s ( pResult + wcslen(pResult), sizeInChars - wcslen(pResult), L"%c%03d", separator, fraction [ i ] );
            }
        }
    }
    else
    {
        swprintf_s ( pResult + wcslen(pResult), sizeInChars - wcslen(pResult), L"%d", 0 );
    }

    //
    return pResult;
}

//
///////////////////////////////////////////////////////////////////////////////
//  Separated Number
///////////////////////////////////////////////////////////////////////////////
WCHAR *SeparatedNumberW ( WCHAR *pResult, size_t sizeInChars, long long largeInt, WCHAR separator )
{
    ZeroMemory ( pResult, sizeInChars * sizeof(WCHAR) );

    const int lenFraction = 10;
    unsigned int fraction  [ lenFraction ];
    for ( int i = 0; i < lenFraction; i++ )
    {
        fraction [ i ] = 0;
    }

    //
    int count = 0;
    long long positiveValue = abs(largeInt);
    while ( positiveValue > 0 )
    {
        fraction [ count ]  = positiveValue % 1000;
        positiveValue       = positiveValue / 1000;
        count++;
    }

    //
    bool bFirst = true;
    if ( count > 0 )
    {
        if ( largeInt < 0  )
        {
            wcscat_s ( pResult + wcslen(pResult), sizeInChars - wcslen(pResult), L"-" );
        }

        for ( int i = count - 1; i >= 0; i-- )
        {
            if ( bFirst )
            {
                swprintf_s ( pResult + wcslen(pResult), sizeInChars - wcslen(pResult), L"%d", fraction [ i ] );
                bFirst = false;
            }
            else
            {
                swprintf_s ( pResult + wcslen(pResult), sizeInChars - wcslen(pResult), L"%c%03d", separator, fraction [ i ] );
            }
        }
    }
    else
    {
        swprintf_s ( pResult + wcslen(pResult), sizeInChars - wcslen(pResult), L"%d", 0 );
    }

    //
    return pResult;
}

//
///////////////////////////////////////////////////////////////////////////////
//  Separated Number
///////////////////////////////////////////////////////////////////////////////
WCHAR *SeparatedNumberW ( WCHAR *pResult, size_t sizeInChars, LARGE_INTEGER largeInt, char separator )
{
    return SeparatedNumberW ( pResult, sizeInChars, largeInt.QuadPart, separator );
}

//
///////////////////////////////////////////////////////////////////////////////
//  Separated Number
///////////////////////////////////////////////////////////////////////////////
char *SeparatedNumberA ( char *pResult, size_t sizeInChars, unsigned long long largeInt, char separator )
{
    ZeroMemory ( pResult, sizeInChars );

    const int lenFraction = 10;
    unsigned int fraction  [ lenFraction ];
    for ( int i = 0; i < lenFraction; i++ )
    {
        fraction [ i ] = 0;
    }

    //
    int count = 0;
    while ( largeInt > 0 )
    {
        fraction [ count ]  = largeInt % 1000;
        largeInt            = largeInt / 1000;
        count++;
    }

    //
    bool bFirst = true;
    if ( count > 0 )
    {
        for ( int i = count - 1; i >= 0; i-- )
        {
            if ( bFirst )
            {
                sprintf_s ( pResult + strlen(pResult), sizeInChars - strlen(pResult), "%d", fraction [ i ] );
                bFirst = false;
            }
            else
            {
                sprintf_s ( pResult + strlen(pResult), sizeInChars - strlen(pResult), "%c%03d", separator, fraction [ i ] );
            }
        }
    }
    else
    {
        sprintf_s ( pResult + strlen(pResult), sizeInChars - strlen(pResult), "%d", 0 );
    }

    //
    return pResult;
}

//
///////////////////////////////////////////////////////////////////////////////
//  Separated Number
///////////////////////////////////////////////////////////////////////////////
char *SeparatedNumberA ( char *pResult, size_t sizeInChars, long long largeInt, char separator )
{
    ZeroMemory ( pResult, sizeInChars );

    const int lenFraction = 10;
    unsigned int fraction  [ lenFraction ];
    for ( int i = 0; i < lenFraction; i++ )
    {
        fraction [ i ] = 0;
    }

    //
    int count = 0;
    long long positiveValue = abs(largeInt);
    while ( positiveValue > 0 )
    {
        fraction [ count ]  = positiveValue % 1000;
        positiveValue       = positiveValue / 1000;
        count++;
    }

    //
    bool bFirst = true;
    if ( count > 0 )
    {
        if ( largeInt < 0  )
        {
            strcat_s ( pResult + strlen(pResult), sizeInChars - strlen(pResult), "-" );
        }

        for ( int i = count - 1; i >= 0; i-- )
        {
            if ( bFirst )
            {
                sprintf_s ( pResult + strlen(pResult), sizeInChars - strlen(pResult), "%d", fraction [ i ] );
                bFirst = false;
            }
            else
            {
                sprintf_s ( pResult + strlen(pResult), sizeInChars - strlen(pResult), "%c%03d", separator, fraction [ i ] );
            }
        }
    }
    else
    {
        sprintf_s ( pResult + strlen(pResult), sizeInChars - strlen(pResult), "%d", 0 );
    }

    //
    return pResult;
}

//
///////////////////////////////////////////////////////////////////////////////
//  Separated Number
///////////////////////////////////////////////////////////////////////////////
char *SeparatedNumberA ( char *pResult, size_t sizeInChars, LARGE_INTEGER largeInt, char separator )
{
    return SeparatedNumberA ( pResult, sizeInChars, largeInt.QuadPart, separator );
}

//
//====================================================================================
//  Set WideCharToMultiByte Flag
//====================================================================================
DWORD SetWCFlag ( INT codePage, const char *DefaultChar, LPCSTR &pDefaultChar, BOOL &UsedDefaultChar, LPBOOL &pUsedDefaultChar )
{
    //  WC_COMPOSITECHECK   WC_ERR_INVALID_CHARS    WC_NO_BEST_FIT_CHARS
    //  WC_DEFAULTCHAR
    DWORD   dwFlag  = NULL;

    //  Set Used Default Char
    UsedDefaultChar     = FALSE;
    pUsedDefaultChar    = &UsedDefaultChar;

    //  Set Default Char
    pDefaultChar        = DefaultChar;

    if ( codePage == CP_UTF8 || codePage == CP_UTF7 )
    {
        pDefaultChar            = NULL;
        pUsedDefaultChar        = NULL;
    }

    if ( codePage == CP_UTF7 || codePage == CP_SYMBOL || codePage == 50220 || codePage == 50221 || codePage == 50222 )
    {
        dwFlag  = NULL;
    }

    if ( codePage == 50225 || codePage == 50227 || codePage == 50229 )
    {
        dwFlag  = NULL;
    }

    if ( codePage >= 57002 && codePage <= 57011 )
    {
        dwFlag  = NULL;
    }

    if ( codePage == CP_UTF8 || codePage == 54936 )
    {
        dwFlag  = WC_ERR_INVALID_CHARS;
    }
    
    return dwFlag;  
}

//
//====================================================================================
//  Set MultiByteToWideChar Flag
//  Normally MB_PRECOMPOSED is the default
//  but here we are generally MB_ERR_INVALID_CHARS
//====================================================================================
DWORD SetMBFlag ( INT codePage )
{
    DWORD   dwFlag = NULL;

    //  MB_COMPOSITE    MB_ERR_INVALID_CHARS    MB_PRECOMPOSED  MB_USEGLYPHCHARS
    dwFlag = MB_ERR_INVALID_CHARS;
    if ( codePage == CP_UTF8 || codePage == 54936 )
    {
        dwFlag  = MB_ERR_INVALID_CHARS;
    }

    if ( codePage == CP_UTF7 || codePage == CP_SYMBOL || codePage == 50220 || codePage == 50221 || codePage == 50222 )
    {
        dwFlag  = NULL;
    }

    if ( codePage == 50225 || codePage == 50227 || codePage == 50229 )
    {
        dwFlag  = NULL;
    }

    if ( codePage >= 57002 && codePage <= 57011 )
    {
        dwFlag  = NULL;
    }

    return dwFlag;
}

//
//====================================================================================
//  Return size in bytes
//====================================================================================
SizeInBytes ConvertWCToMBCS (   const WCHAR *wcLineZero, SizeInChars iWcLineZero,
                                char *mbLineFinal, SizeInBytes iMbLineFinal,
                                INT CodePageTo, BOOL &bErrors )
{
    bErrors = FALSE;
    SetLastError ( NULL );

    if ( iMbLineFinal > 0 )
    {
        ZeroMemory ( mbLineFinal, iMbLineFinal );
    }

    if ( iWcLineZero <= 0 )
    {
        return 0;
    }

    //
    LPCSTR  pDefaultChar        = "?";
    BOOL    UsedDefaultChar     = FALSE;
    LPBOOL  pUsedDefaultChar    = &UsedDefaultChar;

    DWORD   dwWCFlag = SetWCFlag ( CodePageTo, "?", pDefaultChar, UsedDefaultChar, pUsedDefaultChar );

    //
    //      Convert UFT-16 to CodePage (for example CP_OEM)
    int retMbLineFinal = WideCharToMultiByte (
        CodePageTo,                             // code page
        dwWCFlag,                               // performance and mapping flags
        wcLineZero,                             // wide-character string
        iWcLineZero,                            // number of chars in string
        mbLineFinal,                            // buffer for new string
        iMbLineFinal,                           // size of buffer
        pDefaultChar,                           // default for unmappable chars
        pUsedDefaultChar                        // set when default char used
    );

    //
    bErrors = UsedDefaultChar;

    //  Default CHar used is an error but if ignore return
    if ( UsedDefaultChar )
    {
        return retMbLineFinal;
    }

    //  Error or default char has been used and not ignored
    if ( retMbLineFinal <= 0 )
    {
        bErrors = TRUE;
        retMbLineFinal = 0;
    }

    return retMbLineFinal;

}

//
//====================================================================================
//  Return Size in BYtes
//====================================================================================
SizeInBytes ConvertMBCSToWC (   const char *mbLineZero, SizeInBytes iMbLineZero,
                                WCHAR *wcLineFinal, SizeInBytes iWcLineFinal,
                                INT CodePageTo, BOOL &bErrors )
{
    bErrors = FALSE;
    SetLastError ( NULL );

    if ( iWcLineFinal > 0 )
    {
        ZeroMemory ( wcLineFinal, iWcLineFinal );
    }

    if ( iMbLineZero <= 0 )
    {
        return 0;
    }

    //
    DWORD dwMBFlag = SetMBFlag ( CodePageTo );

    int     iLineFinal = iWcLineFinal / sizeof(WCHAR);

    //
    //  Convert From Code Page to UTF-16
    int retWCLineFinal = MultiByteToWideChar (
        CodePageTo,                             // code page
        dwMBFlag,                               // performance and mapping flags
        mbLineZero,                             // wide-character string
        iMbLineZero,                            // number of chars in string
        wcLineFinal,                            // buffer for new string
        iLineFinal                              // size of buffer in chars
    );

    //  Size is Zero : an error occurs
    if ( retWCLineFinal <= 0 )
    {
        bErrors = TRUE;
        retWCLineFinal = 0;
    }

    return retWCLineFinal * sizeof(WCHAR);
}

//
//====================================================================================
//  Generally CodePageTo is CP_ACP and CodePageFrom is other 1047...
//  To Show EBCDIC
//====================================================================================
SizeInBytes ConvertWCToWC ( WCHAR *wcLine, SizeInChars iWcLine, SizeInChars iWcLineLength,
                            INT CodePageTo, INT CodePageFrom, BOOL &bErrors )
{
    if ( iWcLine <= 0 || iWcLineLength <= 0 )
    {
        return 0;
    }

    SizeInBytes iMbLine = ( iWcLine + 1 ) * sizeof(WCHAR);
    char *mbLine = (char* ) malloc ( iMbLine );
    ZeroMemory ( mbLine, iMbLine );

    BOOL bError1 = FALSE;
    iMbLine = ConvertWCToMBCS ( wcLine, iWcLineLength, mbLine, iMbLine, CodePageTo, bError1 );
    if ( iMbLine <= 0 )
    {
        bErrors = bError1;
        if ( mbLine != NULL )
        {
            free ( mbLine );
        }
        return 0;
    }

    BOOL bError2 = FALSE;
    iWcLine = ConvertMBCSToWC ( mbLine, iMbLine, wcLine, iWcLine, CodePageFrom, bError2 );

    bErrors = bError1 | bError2;
    if ( mbLine != NULL )
    {
        free ( mbLine );
    }
    return iWcLine;
}

//
//====================================================================================
//
//====================================================================================
extern SizeInBytes ConvertMBToMB (  char *mbLine, SizeInBytes iMbLine, SizeInBytes iMbLineLength,
                                    INT CodePageFrom, INT CodePageTo, BOOL &bErrors )
{
    if ( iMbLine <= 0 || iMbLineLength <= 0 )
    {
        return 0;
    }

    SizeInBytes iWcLine = ( iMbLine + 1 )  * sizeof(WCHAR);
    WCHAR *wcLine = (WCHAR* ) malloc ( iWcLine );
    ZeroMemory ( wcLine, iWcLine );

    BOOL bError1 = FALSE;
    BOOL bError2 = FALSE;
    iWcLine = ConvertMBCSToWC ( mbLine, iMbLineLength, wcLine, iWcLine, CodePageFrom, bError1 );
    if ( iWcLine <= 0 )
    {
        bErrors = bError1;
        if ( wcLine != NULL )
        {
            free ( wcLine );
        }
        return 0;
    }

    iMbLine = ConvertWCToMBCS ( wcLine, iWcLine, mbLine, iMbLine, CodePageTo, bError2 );

    bErrors = bError1 | bError2;
    if ( wcLine != NULL )
    {
        free ( wcLine );
    }

    return iMbLine;
}

//
//====================================================================================
//      Convert From UTF8 to ANSI (ACP)
//====================================================================================
bool Try5ConvertUtf8ToAnsiW ( WCHAR *pSource, int iSizeByte, bool bDisplay )
{
    //
    UINT    CodePageFrom    = CP_UTF8;

    //  ANSI
    UINT    CodePageTo      = CP_ACP;

    //
    //      Test Special characters
    int     iRes = 0;

    //
    //      The returned result
    bool    uft8Done = false;

    //
    //  If Line Empty return false
    //  We convert a text line
    if ( wcslen(pSource) <= 0 )
    {
        return uft8Done;
    }

    //
    //  First Convert UTF-16 to ANSI : The String will be an UTF-8 MBCS String
    int iLineOne    = ( int ) ( wcslen ( pSource ) + 1 ) * sizeof(WCHAR) * 2;
    char *mbsLineOne    = ( char * ) malloc ( iLineOne );
    memset ( mbsLineOne, 0, iLineOne );

    LPCSTR  pDefaultChar        = "?";
    BOOL    UsedDefaultChar     = FALSE;
    LPBOOL  pUsedDefaultChar    = &UsedDefaultChar;

    DWORD   dwWCFlag = SetWCFlag ( CodePageTo, "?", pDefaultChar, UsedDefaultChar, pUsedDefaultChar );

    //  UTF-8. With this value set, lpDefaultChar and lpUsedDefaultChar must be set to NULL.
    //  WC_ERR_INVALID_CHARS only applies when CodePage is specified as CP_UTF8
    iRes = WideCharToMultiByte (
        CodePageTo,                             // code page
        dwWCFlag,                               // NULL,    // performance and mapping flags
        pSource,                                // wide-character string
        -1,                                     // number of chars in string
        mbsLineOne,                             // buffer for new string
        iLineOne,                               // size of buffer
        pDefaultChar,                           // default for unmappable chars
        pUsedDefaultChar                        // set when default char used
    );

    //
    //  Error : Leave Unchanged
    if ( iRes <= 0 || UsedDefaultChar )
    {
        free ( mbsLineOne );
        return uft8Done;
    }

    //
    //  Second Convert
    int iLineTwo        =  iLineOne;
    WCHAR   *wcsLineTwo = (WCHAR * ) malloc ( iLineTwo );
    memset ( wcsLineTwo, 0, iLineTwo );

    //
    //      Then Convert MBCS UTF-8 to UTF-16 (unicode)
    DWORD dwMBFlag = SetMBFlag ( CodePageFrom );
    iRes = MultiByteToWideChar (
        CodePageFrom,                       //  code page
        dwMBFlag,                           //  MB_ERR_INVALID_CHARS,   // performance and mapping flags
        mbsLineOne,                         //  wide-character string
        -1,                                 //  number of chars in string
        wcsLineTwo,                         //  buffer for new string
        iLineTwo / sizeof ( WCHAR )         //  size of buffer
    );

    //
    //  Error : Leave Unchanged
    if ( iRes <= 0 )
    {
        free ( wcsLineTwo );
        free ( mbsLineOne );
        return uft8Done;
    }

    ////////////////////
    //  Convert BACK
    //  Normally a simpler method would be to Use Back CodePageFrom and CodePageTo
    //  But We Do a CodePageTo CodePageTo CodePageFrom
    ////////////////////

    //
    //  Convert UTF-16 (unicode) to ANSI
    int iLineThree      = iLineOne;
    char *mbsLineThree  = ( char * ) malloc ( iLineThree );
    memset ( mbsLineThree, 0, iLineThree );

    pDefaultChar        = "?";
    UsedDefaultChar     = FALSE;
    pUsedDefaultChar    = &UsedDefaultChar;
    dwWCFlag = SetWCFlag ( CodePageTo, "?", pDefaultChar, UsedDefaultChar, pUsedDefaultChar );

    //  UTF-8. With this value set, lpDefaultChar and lpUsedDefaultChar must be set to NULL.
    //  WC_ERR_INVALID_CHARS only applies when CodePage is specified as CP_UTF8
    iRes = WideCharToMultiByte(
        CodePageTo,                             //  code page
        dwWCFlag,                               //  NULL,   // performance and mapping flags
        wcsLineTwo,                             //  wide-character string
        -1,                                     //  number of chars in string
        mbsLineThree,                           //  buffer for new string
        iLineThree,                             //  size of buffer
        pDefaultChar,                           //  default for unmappable chars
        pUsedDefaultChar                        //  set when default char used
    );

    if ( iRes <= 0 || UsedDefaultChar )
    {
        free ( mbsLineThree );
        free ( wcsLineTwo );
        free ( mbsLineOne );
        return uft8Done;
    }

    //
    //  Then Convert ANSI to UTF-16
    int iLineFour           =  iLineOne;
    WCHAR   *wcsLineFour    = ( WCHAR * ) malloc ( iLineFour );
    memset ( wcsLineFour, 0, iLineFour );

    //
    //      Convert ANSI to UTF-16
    dwMBFlag = SetMBFlag ( CodePageTo );
    iRes = MultiByteToWideChar (
        CodePageTo,                         //  code page
        dwMBFlag,                           //  MB_ERR_INVALID_CHARS,   // performance and mapping flags
        mbsLineThree,                       //  wide-character string
        -1,                                 //  number of chars in string
        wcsLineFour,                        //  buffer for new string
        iLineFour / sizeof ( WCHAR )        //  size of buffer
    );

    if ( iRes <= 0 )
    {
        free ( wcsLineFour );
        free ( mbsLineThree );
        free ( wcsLineTwo );
        free ( mbsLineOne );
        return uft8Done;
    }

    //
    //  Finally Convert UTF-16 (ANSI) back to UTF-8
    int iLineFive           = iLineOne;
    char *mbsLineFive       = ( char * ) malloc ( iLineFive );
    memset ( mbsLineFive, 0, iLineFive );

    pDefaultChar        = "?";
    UsedDefaultChar     = FALSE;
    pUsedDefaultChar    = &UsedDefaultChar;
    dwWCFlag = SetWCFlag ( CodePageFrom, "?", pDefaultChar, UsedDefaultChar, pUsedDefaultChar );

    //  UTF-8. With this value set, lpDefaultChar and lpUsedDefaultChar must be set to NULL.
    //  WC_ERR_INVALID_CHARS only applies when CodePage is specified as CP_UTF8
    iRes = WideCharToMultiByte(
        CodePageFrom,                           //  code page
        dwWCFlag,                               //  NULL,   // performance and mapping flags
        wcsLineFour,                            //  wide-character string
        -1,                                     //  number of chars in string
        mbsLineFive,                            //  buffer for new string
        iLineFive,                              //  size of buffer
        pDefaultChar,                           //  NULL    // default for unmappable chars
        pUsedDefaultChar                        //  NULL    // set when default char used
    );

    if ( iRes <= 0 )
    {
        free ( mbsLineFive );
        free ( wcsLineFour );
        free ( mbsLineThree );
        free ( wcsLineTwo );
        free ( mbsLineOne );
        return uft8Done;
    }

    //
    //  Result Match with original line
    if ( strcmp ( mbsLineOne, mbsLineFive ) == 0 )
    {
        //  Final Line are Differents
        if ( wcscmp ( pSource, wcsLineTwo ) != 0 )
        {
            if ( bDisplay )
            {
                PrintDirectW ( L"Replacing Utf8 Line '%s' By '%s'\n", pSource, wcsLineTwo ) ;
            }
            wcscpy_s ( pSource, iSizeByte / sizeof ( WCHAR ), wcsLineTwo );
            uft8Done = true;
        }
    }

    //
    free ( mbsLineFive );
    free ( wcsLineFour );
    free ( mbsLineThree );
    free ( wcsLineTwo );
    free ( mbsLineOne );

    //
    return uft8Done;

}

//
//====================================================================================
//      Convert From UTF8 to ANSI (ACP)
//====================================================================================
bool Try4ConvertUtf8ToAnsiW ( WCHAR *pSource, int iSizeByte, bool bDisplay )
{
    //
    UINT    CodePageFrom    = CP_UTF8;

    //  ANSI
    UINT    CodePageTo      = CP_ACP;

    //
    //      Test Special characters
    int     iRes = 0;

    //
    //      The returned result
    bool    uft8Done = false;

    //
    //  If Line Empty return false
    //  We convert a text line
    if ( wcslen(pSource) <= 0 )
    {
        return uft8Done;
    }

    //
    //  First Convert UTF-16 to ANSI : The String will be an UTF-8 MBCS String
    int iLineOne    = ( int ) ( wcslen ( pSource ) + 1 ) * sizeof(WCHAR) * 2;
    char *mbsLineOne    = ( char * ) malloc ( iLineOne );
    memset ( mbsLineOne, 0, iLineOne );

    LPCSTR  pDefaultChar        = "?";
    BOOL    UsedDefaultChar     = FALSE;
    LPBOOL  pUsedDefaultChar    = &UsedDefaultChar;

    DWORD dwWCFlag = SetWCFlag ( CodePageTo, "?", pDefaultChar, UsedDefaultChar, pUsedDefaultChar );

    //  UTF-8. With this value set, lpDefaultChar and lpUsedDefaultChar must be set to NULL.
    //  WC_ERR_INVALID_CHARS only applies when CodePage is specified as CP_UTF8
    iRes = WideCharToMultiByte (
        CodePageTo,                             //  code page
        dwWCFlag,                               //  NULL,   // performance and mapping flags
        pSource,                                //  wide-character string
        -1,                                     //  number of chars in string
        mbsLineOne,                             //  buffer for new string
        iLineOne,                               //  size of buffer
        pDefaultChar,                           //  NULL,   // default for unmappable chars
        pUsedDefaultChar                        //  set when default char used
    );

    //
    //  Error : Leave Unchanged
    if ( iRes <= 0 || UsedDefaultChar )
    {
        free ( mbsLineOne );
        return uft8Done;
    }

    //
    //  Second Convert
    int iLineTwo        =  iLineOne;
    WCHAR   *wcsLineTwo = (WCHAR * ) malloc ( iLineTwo );
    memset ( wcsLineTwo, 0, iLineTwo );

    //
    //      Then Convert MBCS UTF-8 to UTF-16 (unicode)
    DWORD dwMBFlag = SetMBFlag ( CodePageFrom );
    iRes = MultiByteToWideChar (
        CodePageFrom,                       //  code page
        dwMBFlag,                           //  MB_ERR_INVALID_CHARS,   // performance and mapping flags
        mbsLineOne,                         //  wide-character string
        -1,                                 //  number of chars in string
        wcsLineTwo,                         //  buffer for new string
        iLineTwo / sizeof ( WCHAR )         //  size of buffer
    );

    //
    //  Error : Leave Unchanged
    if ( iRes <= 0 )
    {
        free ( wcsLineTwo );
        free ( mbsLineOne );
        return uft8Done;
    }

    ////////////////////
    //  Convert BACK
    //  Normally a simpler method would be to Use Back CodePageFrom and CodePageTo
    //  But We Do a CodePageTo CodePageTo CodePageFrom
    ////////////////////

    //
    //  Convert UTF-16 (unicode) to ANSI
    int iLineThree      = iLineOne;
    char *mbsLineThree  = ( char * ) malloc ( iLineThree );
    memset ( mbsLineThree, 0, iLineThree );

    pDefaultChar        = "?";
    UsedDefaultChar     = FALSE;
    pUsedDefaultChar    = &UsedDefaultChar;
    dwWCFlag = SetWCFlag ( CodePageFrom, "?", pDefaultChar, UsedDefaultChar, pUsedDefaultChar );

    //  UTF-8. With this value set, lpDefaultChar and lpUsedDefaultChar must be set to NULL.
    //  WC_ERR_INVALID_CHARS only applies when CodePage is specified as CP_UTF8
    iRes = WideCharToMultiByte(
        CodePageFrom,                           // code page
        dwWCFlag,                               // WC_ERR_INVALID_CHARS,    // performance and mapping flags
        wcsLineTwo,                             // wide-character string
        -1,                                     // number of chars in string
        mbsLineThree,                           // buffer for new string
        iLineThree,                             // size of buffer
        pDefaultChar,                           // NULL,    // default for unmappable chars
        pUsedDefaultChar                        // NULL     // set when default char used
    );

    if ( iRes <= 0 || UsedDefaultChar )
    {
        free ( mbsLineThree );
        free ( wcsLineTwo );
        free ( mbsLineOne );
        return uft8Done;
    }

    //
    //  Then Convert ANSI to UTF-16
    int iLineFour           =  iLineOne;
    WCHAR   *wcsLineFour    = ( WCHAR * ) malloc ( iLineFour );
    memset ( wcsLineFour, 0, iLineFour );

    //
    //      Convert ANSI to UTF-16
    dwMBFlag = SetMBFlag ( CodePageTo );
    iRes = MultiByteToWideChar (
        CodePageTo,                         //  code page
        dwMBFlag,                           //  MB_ERR_INVALID_CHARS,   // performance and mapping flags
        mbsLineThree,                       //  wide-character string
        -1,                                 //  number of chars in string
        wcsLineFour,                        //  buffer for new string
        iLineFour / sizeof ( WCHAR )        //  size of buffer
    );

    if ( iRes <= 0 )
    {
        free ( wcsLineFour );
        free ( mbsLineThree );
        free ( wcsLineTwo );
        free ( mbsLineOne );
        return uft8Done;
    }

    //
    //  Result Match with original line
    if ( wcscmp ( pSource, wcsLineFour ) == 0 )
    {
        //  Final Line are Differents
        if ( wcscmp ( pSource, wcsLineTwo ) != 0 )
        {
            if ( bDisplay )
            {
                PrintDirectW ( L"Replacing Utf8 Line '%s' By '%s'\n", pSource, wcsLineTwo ) ;
            }
            wcscpy_s ( pSource, iSizeByte / sizeof ( WCHAR ), wcsLineTwo );
            uft8Done = true;
        }
    }

    //
    free ( wcsLineFour );
    free ( mbsLineThree );
    free ( wcsLineTwo );
    free ( mbsLineOne );

    //
    return uft8Done;

}

//
//====================================================================================
//  Return the ccs : UTF-8 UTF-16LE UNICODE UTF-32LE UTF-32BE or ERROR or empty
//====================================================================================
const WCHAR *CheckBOMValueW ( const WCHAR *pFilename, bool display )
{
    //
    const WCHAR *BOM = L"";

    //
    if ( ! CheckPathExistW ( pFilename ) )
    {
        if ( display )
        {
            PrintStderrW ( L"File %s does not exist\n", pFilename );
        }
        return BOM;
    }

    //
    //  Check BOM when no encoding specified
    FILE *hInputFile = NULL;

    errno_t iError = _wfopen_s ( &hInputFile, pFilename, L"rb" );
    if ( hInputFile != NULL )
    {
        char szLine [ 8 ];
        ZeroMemory ( szLine, sizeof(szLine) );
        size_t iRead = fread ( szLine, sizeof(char), 4, hInputFile );
        if ( iRead >= 2  )
        {
            if (        (BYTE)szLine [ 0 ] == 0xFF && (BYTE)szLine [ 1 ] == 0xFE    &&
                        (BYTE)szLine [ 2 ] == 0x00 && (BYTE)szLine [ 3 ] == 0x00    &&
                        iRead >= 4 )
            {
                BOM = CCS_UTF_32LE_W;
            }
            else if (   (BYTE)szLine [ 0 ] == 0x00 && (BYTE)szLine [ 1 ] == 0x00    &&
                        (BYTE)szLine [ 2 ] == 0xFE && (BYTE)szLine [ 3 ] == 0xFF    &&
                        iRead >= 4 )
            {
                BOM = CCS_UTF_32BE_W;
            }
            else if (   (BYTE) szLine [ 0 ] == 0xEF && (BYTE) szLine [ 1 ] == 0xBB  &&
                        (BYTE) szLine [ 2 ] == 0xBF && iRead >= 3 )
            {
                BOM = CCS_UTF_8_W;
            }
            else if (   (BYTE)szLine [ 0 ] == 0xFF && (BYTE)szLine [ 1 ] == 0xFE )
            {
                BOM = CCS_UTF_16LE_W;
            }
            else if (   (BYTE)szLine [ 0 ] == 0xFE && (BYTE)szLine [ 1 ] == 0xFF )
            {
                BOM = CCS_UNICODE_W;
            }
            else
            {
                BOM = L"";
            }
        }

        fclose ( hInputFile );

        return BOM;
    }
    else if ( display )
    {
        _wcserror_s ( ErrorTextW, _wsizeof ( ErrorTextW ), iError );
        PrintStderrW ( L"Error Opening File %s for %s : %s\n", pFilename, L"rb", ErrorTextW );
    }

    return BOM;
}

//
//====================================================================================
//  Return the ccs : UTF-8 UTF-16LE UNICODE UTF-32LE UTF-32BE or ERROR or empty
//====================================================================================
const char *CheckBOMValueA ( const char *pFilename, bool display )
{
    //
    const char *BOM = "";

    //
    if ( ! CheckPathExistA ( pFilename ) )
    {
        if ( display )
        {
            PrintStderrA ( "File %s does not exist\n", pFilename );
        }
        return BOM;
    }

    //
    //  Check BOM when no encoding specified
    FILE *hInputFile = NULL;

    errno_t iError = fopen_s ( &hInputFile, pFilename, "rb" );
    if ( hInputFile != NULL )
    {
        char szLine [ 8 ];
        ZeroMemory ( szLine, sizeof(szLine) );
        size_t iRead = fread ( szLine, sizeof(char), 4, hInputFile );
        if ( iRead >= 2  )
        {
            if (        (BYTE)szLine [ 0 ] == 0xFF && (BYTE)szLine [ 1 ] == 0xFE    &&
                        (BYTE)szLine [ 2 ] == 0x00 && (BYTE)szLine [ 3 ] == 0x00    &&
                        iRead >= 4 )
            {
                BOM = CCS_UTF_32LE_A;
            }
            else if (   (BYTE)szLine [ 0 ] == 0x00 && (BYTE)szLine [ 1 ] == 0x00    &&
                        (BYTE)szLine [ 2 ] == 0xFE && (BYTE)szLine [ 3 ] == 0xFF    &&
                        iRead >= 4 )
            {
                BOM = CCS_UTF_32BE_A;
            }
            else if (   (BYTE) szLine [ 0 ] == 0xEF && (BYTE) szLine [ 1 ] == 0xBB  &&
                        (BYTE) szLine [ 2 ] == 0xBF && iRead >= 3 )
            {
                BOM = CCS_UTF_8_A;
            }
            else if (   (BYTE)szLine [ 0 ] == 0xFF && (BYTE)szLine [ 1 ] == 0xFE )
            {
                BOM = CCS_UTF_16LE_A;
            }
            else if (   (BYTE)szLine [ 0 ] == 0xFE && (BYTE)szLine [ 1 ] == 0xFF )
            {
                BOM = CCS_UNICODE_A;
            }
            else
            {
                BOM = "";
            }
        }

        fclose ( hInputFile );

        return BOM;
    }
    else if ( display )
    {
        strerror_s ( ErrorTextA, sizeof ( ErrorTextA ), iError );
        PrintStderrA ( "Error Opening File %s for %s : %s\n", pFilename, L"rb", ErrorTextA );
    }

    return BOM;
}

//
//====================================================================================
//  Get BOM Directive
//====================================================================================
bool AddBOMDirectiveW ( WCHAR *pText, size_t iSizeInCHars, const WCHAR *pBOMValue )
{
    //
    if (    ( _wcsicmp ( pBOMValue, CCS_UTF_8_W ) == 0 )        ||
            ( _wcsicmp ( pBOMValue, CCS_UTF_16LE_W ) == 0 )     ||
            ( _wcsicmp ( pBOMValue, CCS_UNICODE_W ) == 0 )          )
    {
        swprintf_s ( pText + wcslen(pText), iSizeInCHars - wcslen(pText), L", ccs=%s", pBOMValue );
        return true;
    }

    return false;
}

//
//====================================================================================
//  Get BOM Directive
//====================================================================================
bool AddBOMDirectiveA ( char *pText, size_t iSizeInCHars, const char *pBOMValue )
{
    //
    if (    ( _stricmp ( pBOMValue, CCS_UTF_8_A ) == 0 )        ||
            ( _stricmp ( pBOMValue, CCS_UTF_16LE_A ) == 0 )     ||
            ( _stricmp ( pBOMValue, CCS_UNICODE_A ) == 0 )          )
    {
        sprintf_s ( pText + strlen(pText), iSizeInCHars - strlen(pText), ", ccs=%s", pBOMValue );
        return true;
    }

    return false;
}

//
//====================================================================================
//  Open File
//====================================================================================
FILE *OpenFileCcsW ( const WCHAR *filename, const WCHAR *pMode, bool binary, bool display, const WCHAR *bomFile )
{
    //
    WCHAR szMode [ 32 ];
    wcscpy_s ( szMode, _wsizeof(szMode), pMode );

    //
    const WCHAR *pBOM = L"";

    //  Already a ccs
    if ( wcsstr ( szMode, L"ccs" ) == NULL )
    {
        //  We Check BOM even for write for existing file to get the same BOM
        //  Display Only Message on read
        if ( wcslen(bomFile) == 0 )
        {
            pBOM = CheckBOMValueW ( filename, display && ( *szMode == L'r' ) );
        }
        else
        {
            pBOM = CheckBOMValueW ( bomFile, display );
        }
    }

    //  Write
    if ( __wcsnicmp1W ( szMode, L"w8" ) == 0 )
    {
        __strrepxW ( szMode, _wsizeof(szMode), L"w8", L"w" );
        pBOM = CCS_UTF_8_W;
    }
    else if ( __wcsnicmp1W ( szMode, L"w16" ) == 0 )
    {
        __strrepxW ( szMode, _wsizeof(szMode), L"w16", L"w" );
        pBOM = CCS_UTF_16LE_W;
    }
    else if ( __wcsnicmp1W ( szMode, L"wu" ) == 0 )
    {
        __strrepxW ( szMode, _wsizeof(szMode), L"wu", L"w" );
        pBOM = CCS_UNICODE_W;
    }
    //  Append
    else if ( __wcsnicmp1W ( szMode, L"a8" ) == 0 )
    {
        __strrepxW ( szMode, _wsizeof(szMode), L"a8", L"a" );
        pBOM = CCS_UTF_8_W;
    }
    else if ( __wcsnicmp1W ( szMode, L"a16" ) == 0 )
    {
        __strrepxW ( szMode, _wsizeof(szMode), L"a16", L"a" );
        pBOM = CCS_UTF_16LE_W;
    }
    else if ( __wcsnicmp1W ( szMode, L"au" ) == 0 )
    {
        __strrepxW ( szMode, _wsizeof(szMode), L"au", L"a" );
        pBOM = CCS_UNICODE_W;
    }
    //  Read
    else if ( __wcsnicmp1W ( szMode, L"r8" ) == 0 )
    {
        __strrepxW ( szMode, _wsizeof(szMode), L"r8", L"r" );
        pBOM = CCS_UTF_8_W;
    }
    else if ( __wcsnicmp1W ( szMode, L"r16" ) == 0 )
    {
        __strrepxW ( szMode, _wsizeof(szMode), L"r16", L"r" );
        pBOM = CCS_UTF_16LE_W;
    }
    else if ( __wcsnicmp1W ( szMode, L"ru" ) == 0 )
    {
        __strrepxW ( szMode, _wsizeof(szMode), L"ru", L"r" );
        pBOM = CCS_UNICODE_W;
    }

    //  If Binary add b
    if ( binary && wcschr ( szMode, L'b' ) == NULL )
    {
        wcscat_s ( szMode, _wsizeof(szMode), L"b" );
    }

    //
    WCHAR   OpenOption [ 32 ];

    wcscpy_s ( OpenOption, _wsizeof(OpenOption), szMode );
    if ( wcslen(pBOM) > 0 )
    {
        AddBOMDirectiveW ( OpenOption, _wsizeof(OpenOption), pBOM );
    }

    //
    FILE *hFile = NULL;
    errno_t iError = 0;
    if ( ! ForceShareMode && wcsstr ( OpenOption, L"ccs" ) != NULL )
    {
        //
        if ( display )
        {
            PrintVerboseW ( L"Opening Normal Mode for '%s' is '%s'\n", filename, OpenOption );
        }

        iError = _wfopen_s ( &hFile, filename, OpenOption );
    }
    else
    {
        //
        if ( display )
        {
            PrintVerboseW ( L"Opening Share Mode for '%s' is '%s'\n", filename, OpenOption );
        }

        hFile = _wfsopen ( filename, OpenOption, _SH_DENYWR );
        _get_errno( &iError );
    }
    
    if ( display && hFile == NULL )
    {
        _wcserror_s ( ErrorTextW, _wsizeof ( ErrorTextW ), iError );
        PrintStderrW ( L"Error Opening File %s for %s : %s\n", filename, OpenOption, ErrorTextW );
    }

    return hFile;
}

//
//====================================================================================
//  Open File
//====================================================================================
errno_t OpenFileCcsW ( FILE **hFile, const WCHAR *filename, const WCHAR *pMode, bool display, const WCHAR *bomFile )
{
    //
    WCHAR szMode [ 32 ];
    wcscpy_s ( szMode, _wsizeof(szMode), pMode );

    //
    const WCHAR *pBOM = L"";

    //  Already a ccs
    if ( wcsstr ( szMode, L"ccs" ) == NULL )
    {
        //  We Check BOM even for write for existing file to get the same BOM
        //  Display Only Message on read
        if ( wcslen(bomFile) == 0 )
        {
            pBOM = CheckBOMValueW ( filename, display && ( *szMode == L'r' ) );
        }
        else
        {
            pBOM = CheckBOMValueW ( bomFile, display );
        }
    }

    //  Write
    if ( __wcsnicmp1W ( szMode, L"w8" ) == 0 )
    {
        __strrepxW ( szMode, _wsizeof(szMode), L"w8", L"w" );
        pBOM = CCS_UTF_8_W;
    }
    else if ( __wcsnicmp1W ( szMode, L"w16" ) == 0 )
    {
        __strrepxW ( szMode, _wsizeof(szMode), L"w16", L"w" );
        pBOM = CCS_UTF_16LE_W;
    }
    else if ( __wcsnicmp1W ( szMode, L"wu" ) == 0 )
    {
        __strrepxW ( szMode, _wsizeof(szMode), L"wu", L"w" );
        pBOM = CCS_UNICODE_W;
    }
    //  Append
    else if ( __wcsnicmp1W ( szMode, L"a8" ) == 0 )
    {
        __strrepxW ( szMode, _wsizeof(szMode), L"a8", L"a" );
        pBOM = CCS_UTF_8_W;
    }
    else if ( __wcsnicmp1W ( szMode, L"a16" ) == 0 )
    {
        __strrepxW ( szMode, _wsizeof(szMode), L"a16", L"a" );
        pBOM = CCS_UTF_16LE_W;
    }
    else if ( __wcsnicmp1W ( szMode, L"au" ) == 0 )
    {
        __strrepxW ( szMode, _wsizeof(szMode), L"au", L"a" );
        pBOM = CCS_UNICODE_W;
    }
    //  Read
    else if ( __wcsnicmp1W ( szMode, L"r8" ) == 0 )
    {
        __strrepxW ( szMode, _wsizeof(szMode), L"r8", L"r" );
        pBOM = CCS_UTF_8_W;
    }
    else if ( __wcsnicmp1W ( szMode, L"r16" ) == 0 )
    {
        __strrepxW ( szMode, _wsizeof(szMode), L"r16", L"r" );
        pBOM = CCS_UTF_16LE_W;
    }
    else if ( __wcsnicmp1W ( szMode, L"ru" ) == 0 )
    {
        __strrepxW ( szMode, _wsizeof(szMode), L"ru", L"r" );
        pBOM = CCS_UNICODE_W;
    }

    //
    WCHAR   OpenOption [ 32 ];

    wcscpy_s ( OpenOption, _wsizeof(OpenOption), szMode );
    if ( wcslen(pBOM) > 0 )
    {
        AddBOMDirectiveW ( OpenOption, _wsizeof(OpenOption), pBOM );
    }


    //
    errno_t iError = 0;
    if ( ! ForceShareMode && wcsstr ( OpenOption, L"ccs" ) != NULL )
    {
        //
        if ( display )
        {
            PrintVerboseW ( L"Opening Normal Mode for '%s' is '%s'\n", filename, OpenOption );
        }

        iError = _wfopen_s ( hFile, filename, OpenOption );
    }
    else
    {
        //
        if ( display )
        {
            PrintVerboseW ( L"Opening Share Mode for '%s' is '%s'\n", filename, OpenOption );
        }

        *hFile = _wfsopen ( filename, OpenOption, _SH_DENYWR );
        _get_errno( &iError );
    }

    //
    if ( display && *hFile == NULL )
    {
        _wcserror_s ( ErrorTextW, _wsizeof ( ErrorTextW ), iError );
        PrintStderrW ( L"Error Opening File %s for %s : %s\n", filename, OpenOption, ErrorTextW );
    }

    return iError;
}

//
//====================================================================================
//  Open File
//====================================================================================
FILE *OpenFileCcsA ( const char *filename, const char *pMode, bool binary, bool display, const char *bomFile )
{
    //
    char szMode [ 32 ];
    strcpy_s ( szMode, sizeof(szMode), pMode );

    //
    const char *pBOM = "";

    //  Already a ccs
    if ( strstr ( szMode, "ccs" ) == NULL )
    {
        //  We Check BOM even for write for existing file to get the same BOM
        //  Display Only Message on read
        if ( strlen(bomFile) == 0 )
        {
            pBOM = CheckBOMValueA ( filename, display && ( *szMode == L'r' ) );
        }
        else
        {
            pBOM = CheckBOMValueA ( bomFile, display );
        }
    }

    //  Write
    if ( __strnicmp1A ( szMode, "w8" ) == 0 )
    {
        __strrepxA ( szMode, sizeof(szMode), "w8", "w" );
        pBOM = CCS_UTF_8_A;
    }
    else if ( __strnicmp1A ( szMode, "w16" ) == 0 )
    {
        __strrepxA ( szMode, sizeof(szMode), "w16", "w" );
        pBOM = CCS_UTF_16LE_A;
    }
    else if ( __strnicmp1A ( szMode, "wu" ) == 0 )
    {
        __strrepxA ( szMode, sizeof(szMode), "wu", "w" );
        pBOM = CCS_UNICODE_A;
    }
    //  Append
    else if ( __strnicmp1A ( szMode, "a8" ) == 0 )
    {
        __strrepxA ( szMode, sizeof(szMode), "a8", "a" );
        pBOM = CCS_UTF_8_A;
    }
    else if ( __strnicmp1A ( szMode, "a16" ) == 0 )
    {
        __strrepxA ( szMode, sizeof(szMode), "a16", "a" );
        pBOM = CCS_UTF_16LE_A;
    }
    else if ( __strnicmp1A ( szMode, "au" ) == 0 )
    {
        __strrepxA ( szMode, sizeof(szMode), "au", "a" );
        pBOM = CCS_UNICODE_A;
    }
    //  Read
    else if ( __strnicmp1A ( szMode, "r8" ) == 0 )
    {
        __strrepxA ( szMode, sizeof(szMode), "r8", "r" );
        pBOM = CCS_UTF_8_A;
    }
    else if ( __strnicmp1A ( szMode, "r16" ) == 0 )
    {
        __strrepxA ( szMode, sizeof(szMode), "r16", "r" );
        pBOM = CCS_UTF_16LE_A;
    }
    else if ( __strnicmp1A ( szMode, "ru" ) == 0 )
    {
        __strrepxA ( szMode, sizeof(szMode), "ru", "r" );
        pBOM = CCS_UNICODE_A;
    }

    if ( binary && strchr ( szMode, 'b' ) == NULL )
    {
        strcat_s ( szMode, _wsizeof(szMode), "b" );
    }

    //
    char    OpenOption [ 32 ];

    strcpy_s ( OpenOption, sizeof(OpenOption), szMode );
    if ( strlen(pBOM) > 0 )
    {
        AddBOMDirectiveA ( OpenOption, sizeof(OpenOption), pBOM );
    }

    //
    FILE *hFile = NULL;
    errno_t iError = 0;
    if ( ! ForceShareMode && strstr ( OpenOption, "ccs" ) != NULL )
    {
        //
        if ( display )
        {
            PrintVerboseA ( "Opening Normal Mode for '%s' is '%s'\n", filename, OpenOption );
        }

        iError = fopen_s ( &hFile, filename, OpenOption );
    }
    else
    {
        //
        if ( display )
        {
            PrintVerboseA ( "Opening Share Mode for '%s' is '%s'\n", filename, OpenOption );
        }

        hFile = _fsopen ( filename, OpenOption, _SH_DENYWR );
        _get_errno( &iError );
    }

    if ( display && hFile == NULL )
    {
        strerror_s ( ErrorTextA, sizeof ( ErrorTextA ), iError );
        PrintStderrA ( "Error Opening File %s for %s : %s\n", filename, OpenOption, ErrorTextA );
    }

    return hFile;
}

//
//====================================================================================
//  Open File
//====================================================================================
errno_t OpenFileCcsA ( FILE **hFile, const char *filename, const char *pMode, bool display, const char *bomFile )
{
    //
    char szMode [ 32 ];
    strcpy_s ( szMode, sizeof(szMode), pMode );

    //
    const char *pBOM = "";

    //  Already a ccs
    if ( strstr ( szMode, "ccs" ) == NULL )
    {
        //  We Check BOM even for write for existing file to get the same BOM
        //  Display Only Message on read
        if ( strlen(bomFile) == 0 )
        {
            pBOM = CheckBOMValueA ( filename, display && ( *szMode == L'r' ) );
        }
        else
        {
            pBOM = CheckBOMValueA ( bomFile, display );
        }
    }

    //  Write
    if ( __strnicmp1A ( szMode, "w8" ) == 0 )
    {
        __strrepxA ( szMode, sizeof(szMode), "w8", "w" );
        pBOM = CCS_UTF_8_A;
    }
    else if ( __strnicmp1A ( szMode, "w16" ) == 0 )
    {
        __strrepxA ( szMode, sizeof(szMode), "w16", "w" );
        pBOM = CCS_UTF_16LE_A;
    }
    else if ( __strnicmp1A ( szMode, "wu" ) == 0 )
    {
        __strrepxA ( szMode, sizeof(szMode), "wu", "w" );
        pBOM = CCS_UNICODE_A;
    }
    //  Append
    else if ( __strnicmp1A ( szMode, "a8" ) == 0 )
    {
        __strrepxA ( szMode, sizeof(szMode), "a8", "a" );
        pBOM = CCS_UTF_8_A;
    }
    else if ( __strnicmp1A ( szMode, "a16" ) == 0 )
    {
        __strrepxA ( szMode, sizeof(szMode), "a16", "a" );
        pBOM = CCS_UTF_16LE_A;
    }
    else if ( __strnicmp1A ( szMode, "au" ) == 0 )
    {
        __strrepxA ( szMode, sizeof(szMode), "au", "a" );
        pBOM = CCS_UNICODE_A;
    }
    //  Read
    else if ( __strnicmp1A ( szMode, "r8" ) == 0 )
    {
        __strrepxA ( szMode, sizeof(szMode), "r8", "r" );
        pBOM = CCS_UTF_8_A;
    }
    else if ( __strnicmp1A ( szMode, "r16" ) == 0 )
    {
        __strrepxA ( szMode, sizeof(szMode), "r16", "r" );
        pBOM = CCS_UTF_16LE_A;
    }
    else if ( __strnicmp1A ( szMode, "ru" ) == 0 )
    {
        __strrepxA ( szMode, sizeof(szMode), "ru", "r" );
        pBOM = CCS_UNICODE_A;
    }

    //
    char    OpenOption [ 32 ];

    strcpy_s ( OpenOption, sizeof(OpenOption), szMode );
    if ( strlen(pBOM) > 0 )
    {
        AddBOMDirectiveA ( OpenOption, sizeof(OpenOption), pBOM );
    }

    //
    errno_t iError = 0;
    if ( ! ForceShareMode && strstr ( OpenOption, "ccs" ) != NULL )
    {
        //
        if ( display )
        {
            PrintVerboseA ( "Opening Normal Mode for '%s' is '%s'\n", filename, OpenOption );
        }

        iError = fopen_s ( hFile, filename, OpenOption );
    }
    else
    {
        //
        if ( display )
        {
            PrintVerboseA ( "Opening Share Mode for '%s' is '%s'\n", filename, OpenOption );
        }

        *hFile = _fsopen ( filename, OpenOption, _SH_DENYWR );
        _get_errno( &iError );
    }

    //
    if ( display && *hFile == NULL )
    {
        strerror_s ( ErrorTextA, sizeof ( ErrorTextA ), iError );
        PrintStderrA ( "Error Opening File %s for %s : %s\n", filename, OpenOption, ErrorTextA );
    }

    return iError;
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
bool CheckFileExistW ( const WCHAR *pathname )
{
    FILE *hFile = NULL;
    _wfopen_s ( &hFile, pathname, L"rb" );
    if ( hFile == NULL )
    {
        return false;
    }
    fclose ( hFile );
    return true;
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
bool CheckFileExistA ( const char *pathname )
{
    FILE *hFile = NULL;
    fopen_s ( &hFile, pathname, "rb" );
    if ( hFile == NULL )
    {
        return false;
    }
    fclose ( hFile );
    return true;
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
bool CheckPathExistW ( const WCHAR *pathname )
{
    BOOL bResult = PathFileExistsW ( pathname );
    if (  bResult )
    {
        return true;
    }

    return false;
}

//
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
bool CheckPathExistA ( const char *pathname )
{
    BOOL bResult = PathFileExistsA ( pathname );
    if (  bResult )
    {
        return true;
    }

    return false;
}

//
/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
BOOL WINAPI ConsoleEventHandler ( DWORD signal )
{
    ConsoleEventSignal = signal;

    switch ( signal )
    {
        case CTRL_C_EVENT :
        {
            if ( IsAbortedPointer != NULL )
            {
                *IsAbortedPointer   = true;
            }

            PrintStderrW ( L"CTRL-C\n" );

            if ( ConsoleEventTerminate )
            {
                _fcloseall();
                exit(1);
            }
            return TRUE;
        }
    }

    return FALSE;
}

//
/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void SetConsoleEventHandler ( bool *pIsAborted, bool bTerminate )
{
    ConsoleEventTerminate   = bTerminate;
    if ( pIsAborted != NULL )
    {
        IsAbortedPointer        = pIsAborted;
    }
    SetConsoleCtrlHandler(ConsoleEventHandler, TRUE);
}

//
/////////////////////////////////////////////////////////////////////////////
//
//  Routine Description: This routine returns TRUE if the caller's
//  process is a member of the Administrators local group. Caller is NOT
//  expected to be impersonating anyone and is expected to be able to
//  open its own process and process token.
//  Arguments: None.
//  Return Value:
//     TRUE - Caller has Administrators local group.
//     FALSE - Caller does not have Administrators local group. --
/////////////////////////////////////////////////////////////////////////////
BOOL IsThisUserAdmin()
{
    BOOL bResult = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    bResult = AllocateAndInitializeSid (
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &AdministratorsGroup);
    if ( bResult )
    {
        if ( ! CheckTokenMembership( NULL, AdministratorsGroup, &bResult ) )
        {
             bResult = FALSE;
        }
        FreeSid(AdministratorsGroup);
    }

    return(bResult);
}

//
/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
BOOL RunAsAdminA (int iArgCount, char * pArgValues[], bool bWait )
{
    const   size_t  LenString = 1024;
    static char     ModuleFileNameA [ LenString ]       = "";
    static char     ParameterA [ LenString ]            = "";
    static char     ParameterListA [ LenString * 10 ]   = "";

    if ( IsThisUserAdmin() )
    {
        return FALSE;
    }

    ZeroMemory ( ModuleFileNameA, sizeof(ModuleFileNameA) );
    DWORD dwResult = GetModuleFileNameA ( NULL, ModuleFileNameA, sizeof(ModuleFileNameA) );

    ZeroMemory ( ParameterListA, sizeof(ParameterListA) );
    for ( int i = 1; i < iArgCount; i++ )
    {
        ZeroMemory ( ParameterA, sizeof(ParameterA) );
        strcpy_s ( ParameterA, sizeof(ParameterA), pArgValues [ i ] );
        __strrepA ( ParameterA, sizeof(ParameterA), "\"", "\\\"", true );

        //
        strcat_s ( ParameterListA, sizeof(ParameterListA), "\"" );
        strcat_s ( ParameterListA, sizeof(ParameterListA), ParameterA );
        strcat_s ( ParameterListA, sizeof(ParameterListA), "\"" );
        if ( i != iArgCount - 1 )
        {
            strcat_s ( ParameterListA, sizeof(ParameterListA), " " );
        }
    }

    //
    SHELLEXECUTEINFOA shellExecuteInfos = {0};
    ZeroMemory ( &shellExecuteInfos, sizeof(shellExecuteInfos) );
    shellExecuteInfos.cbSize        = sizeof(shellExecuteInfos);
    if ( bWait )
    {
        shellExecuteInfos.fMask         = SEE_MASK_NOCLOSEPROCESS;
    }
    shellExecuteInfos.hwnd          = NULL;
    shellExecuteInfos.lpVerb        = "runas";
    shellExecuteInfos.lpFile        = ModuleFileNameA;
    shellExecuteInfos.lpParameters  = ParameterListA;
    shellExecuteInfos.lpDirectory   = NULL;
    shellExecuteInfos.nShow         = SW_SHOW;
    shellExecuteInfos.hInstApp      = NULL;

#if 1
    BOOL bLaunched = ShellExecuteExA ( &shellExecuteInfos );
    if ( ! bLaunched )
    {
        PrintStderrA ( "Unable to launch %s\n", ModuleFileNameA );
        PrintStderrA ( "Parameters: %s\n", ParameterListA );
        return FALSE;
    }
    else if ( bWait )
    {
        WaitForSingleObject(shellExecuteInfos.hProcess, INFINITE);
        CloseHandle(shellExecuteInfos.hProcess);
    }
#else
    HINSTANCE hInstance = ShellExecuteA ( NULL, "runas", ModuleFileNameA, ParameterListA, NULL, SW_SHOWNORMAL );
    if ( (unsigned) hInstance < 32 )
    {
        PrintStderrA ( "Unable to launch %s\n", ModuleFileNameA );
        PrintStderrA ( "Parameters: %s\n", ParameterListA );
        return FALSE;
    }
#endif

    return TRUE;
}

//
/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
BOOL RunAsAdminW(int iArgCount, WCHAR * pArgValues[], bool bWait )
{
    const   size_t  LenString = 1024;
    static WCHAR    ModuleFileNameW [ LenString ]       = L"";
    static WCHAR    ParameterW [ LenString ]            = L"";
    static WCHAR    ParameterListW [ LenString * 10 ]   = L"";

    if ( IsThisUserAdmin() )
    {
        return FALSE;
    }

    ZeroMemory ( ModuleFileNameW, sizeof(ModuleFileNameW) );
    DWORD dwResult = GetModuleFileNameW ( NULL, ModuleFileNameW, _wsizeof(ModuleFileNameW) );

    ZeroMemory ( ParameterListW, sizeof(ParameterListW) );
    for ( int i = 1; i < iArgCount; i++ )
    {
        ZeroMemory ( ParameterW, sizeof(ParameterW) );
        wcscpy_s ( ParameterW, _wsizeof(ParameterW), pArgValues [ i ] );
        __strrepW ( ParameterW, _wsizeof(ParameterW), L"\"", L"\\\"", true );

        wcscat_s ( ParameterListW, _wsizeof(ParameterListW), L"\"" );
        wcscat_s ( ParameterListW, _wsizeof(ParameterListW), ParameterW );
        wcscat_s ( ParameterListW, _wsizeof(ParameterListW), L"\"" );
        if ( i != iArgCount - 1 )
        {
            wcscat_s ( ParameterListW, _wsizeof(ParameterListW), L" " );
        }
    }

    SHELLEXECUTEINFOW shellExecuteInfos = {0};
    ZeroMemory ( &shellExecuteInfos, sizeof(shellExecuteInfos) );
    shellExecuteInfos.cbSize        = sizeof(shellExecuteInfos);
    if ( bWait )
    {
        shellExecuteInfos.fMask         = SEE_MASK_NOCLOSEPROCESS;
    }
    shellExecuteInfos.hwnd          = NULL;
    shellExecuteInfos.lpVerb        = L"runas";
    shellExecuteInfos.lpFile        = ModuleFileNameW;
    shellExecuteInfos.lpParameters  = ParameterListW;
    shellExecuteInfos.lpDirectory   = NULL;
    shellExecuteInfos.nShow         = SW_SHOW;
    shellExecuteInfos.hInstApp      = NULL;

#if 1
    BOOL bLaunched = ShellExecuteExW( &shellExecuteInfos );
    if ( ! bLaunched )
    {
        PrintStderrW ( L"Unable to launch : '%s'\n", ModuleFileNameW );
        PrintStderrW ( L"Parameters: %s\n", ParameterListW );
        return FALSE;
    }
    else if ( bWait )
    {
        WaitForSingleObject(shellExecuteInfos.hProcess, INFINITE);
        CloseHandle(shellExecuteInfos.hProcess);
    }
#else
    HINSTANCE hInstance = ShellExecuteW ( NULL, L"runas", ModuleFileNameW, ParameterListW, NULL, SW_SHOWNORMAL );
    if ( (unsigned) hInstance < 32 )
    {
        PrintStderrW ( L"Unable to launch : '%s'\n", ModuleFileNameW );
        PrintStderrW ( L"Parameters: %s\n", ParameterListW );
        return FALSE;
    }
#endif
    return TRUE;
}

//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
BOOL TreatLaunchOptionsA(int iArgCount, char* pArgValues[], const char* pOption, int &iX)
{
    if ( *pOption == L'-' )
    {
        pOption++;
    }

    if ( *pOption == L'-' )
    {
        pOption++;
    }

    if ( _stricmp ( pOption, "waitprocess" ) == 0 )
    {
        WaitProcessMode = true;
        return TRUE;
    }
    else if ( _stricmp ( pOption, "nowaitprocess" ) == 0 )
    {
        WaitProcessMode = false;
        return TRUE;
    }
    else if ( _stricmp ( pOption, "runasadmin" ) == 0 )
    {
        RunAsAdminMode  = true;
        return TRUE;
    }
    else if ( _stricmp ( pOption, "runasuser" ) == 0 )
    {
        RunAsAdminMode  = false;
        return TRUE;
    }

    return  FALSE;
}

//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
BOOL TreatLaunchOptionsW(int iArgCount, WCHAR* pArgValues[], const WCHAR* pOption, int &iX)
{
    if ( *pOption == L'-' )
    {
        pOption++;
    }

    if ( *pOption == L'-' )
    {
        pOption++;
    }

    if ( _wcsicmp ( pOption, L"waitprocess" ) == 0 )
    {
        WaitProcessMode = true;
        return TRUE;
    }
    else if ( _wcsicmp ( pOption, L"nowaitprocess" ) == 0 )
    {
        WaitProcessMode = false;
        return TRUE;
    }
    else if ( _wcsicmp ( pOption, L"runasadmin" ) == 0 )
    {
        RunAsAdminMode  = true;
        return TRUE;
    }
    else if ( _wcsicmp ( pOption, L"runasuser" ) == 0 )
    {
        RunAsAdminMode  = false;
        return TRUE;
    }

    return  FALSE;
}

//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
void PrintHelpLaunchA ( int iWidth, const char *programName, int iArgCount, char* pArgValues[], bool bLong )
{
    if ( bLong )
    {
        PrintHelpLineA ( );
        PrintHelpLineA ( iWidth, "-waitprocess", "Wait Process" );
        PrintHelpLineA ( iWidth, "-nowaitprocess", "No Wait Process" );
        PrintHelpLineA ( iWidth, "-runasadmin", "Run As Admin" );
        PrintHelpLineA ( iWidth, "-runasuser", "Run As User" );
    }
}

//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
void PrintHelpLaunchW ( int iWidth, const WCHAR *programName, int iArgCount, WCHAR* pArgValues[], bool bLong )
{
    if ( bLong )
    {
        PrintHelpLineW ( );
        PrintHelpLineW ( iWidth, L"-waitprocess", L"Wait Process" );
        PrintHelpLineW ( iWidth, L"-nowaitprocess", L"No Wait Process" );
        PrintHelpLineW ( iWidth, L"-runasadmin", L"Run As Admin" );
        PrintHelpLineW ( iWidth, L"-runasuser", L"Run As User" );
    }
}

//
//====================================================================================
//
//====================================================================================
char *GetReadMeResouce ( UINT iResource )
{
    HRSRC       hResource       = NULL;
    HGLOBAL     hResourceData   = NULL;
    char *      pReadMeText     = NULL;
    DWORD       dwResourceSize = 0;

    // Find Resource
    hResource = FindResource(NULL, MAKEINTRESOURCE(iResource), RT_RCDATA);
    if ( ! hResource )
    {
        return NULL;
    }

    //  Load Resource
    hResourceData = LoadResource (NULL, hResource );
    if ( ! hResourceData )
    {
        return NULL;
    }

    //
    pReadMeText     = ( char* ) LockResource(hResourceData);
    dwResourceSize  = SizeofResource ( NULL, hResource );

    //  Allocate Buffer to be freed by the caller
    char *pAlloc = ( char * ) malloc ( dwResourceSize  + 1 );
    if ( pAlloc )
    {
        ZeroMemory ( pAlloc, dwResourceSize  + 1 );
        memcpy ( pAlloc, pReadMeText, dwResourceSize );
    }

    return pAlloc;
}

//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
BOOL TreatReadMeOptionA(int iArgCount, char* pArgValues[], const char* pOption, int &iX)
{
    if ( *pOption == L'-' )
    {
        pOption++;
    }

    if ( *pOption == L'-' )
    {
        pOption++;
    }

    if ( _stricmp ( pOption, "readme" ) == 0 )
    {
        ReadMeTextMode      = true;
        return TRUE;
    }

    return FALSE;
}

//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
BOOL TreatReadMeOptionW(int iArgCount, WCHAR* pArgValues[], const WCHAR* pOption, int &iX)
{
    if ( *pOption == L'-' )
    {
        pOption++;
    }

    if ( *pOption == L'-' )
    {
        pOption++;
    }

    if ( _wcsicmp ( pOption, L"readme" ) == 0 )
    {
        ReadMeTextMode      = true;
        return TRUE;
    }

    return FALSE;
}

//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
void PrintHelpReadMeA ( int iWidth, const char *programName, int iArgCount, char* pArgValues[], bool bLong )
{
    if ( bLong )
    {
        PrintHelpLineA ( );
        PrintHelpLineA ( iWidth, "-readme", "Show Read Me Text" );
    }
}

//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
void PrintHelpReadMeW ( int iWidth, const WCHAR *programName, int iArgCount, WCHAR* pArgValues[], bool bLong )
{
    if ( bLong )
    {
        PrintHelpLineW ( );
        PrintHelpLineW ( iWidth, L"-readme", L"Show Read Me Text" );
    }
}

//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
void PrintRealVersionW (int iWidth)
{
    const   size_t  LenString                       = 1024;
    static  WCHAR   ModuleFileNameW [ LenString ]   = L"";
    static  WCHAR   ModuleKey [ LenString ]         = L"";
    static  WCHAR   ModuleText [ LenString ]        = L"";

    WCHAR           *pData                          = NULL;

    //      Get module name
    DWORD dwResult = GetModuleFileNameW ( NULL, ModuleFileNameW, _wsizeof ( ModuleFileNameW ) );

    //      Get Size to allocate for info
    DWORD           dwHandle;
    DWORD dwLength = GetFileVersionInfoSizeW ( ModuleFileNameW, &dwHandle );
    if ( dwLength > 0 )
    {
        pData = new WCHAR [ dwLength ];

        //  Get All Infos
        BOOL bResult = GetFileVersionInfoW( ModuleFileNameW, dwHandle, dwLength, pData );
        if ( bResult )
        {
            void            *pBufferInfo;
            UINT            iBufferLen;

            // Retrieve le language code: 40c or 409
            DWORD dwKey;

            bResult = VerQueryValueW ( pData, L"\\VarFileInfo\\Translation", &pBufferInfo, &iBufferLen );
            if ( bResult  && iBufferLen == 4 )
            {
                WORD    wKey [ 2 ];
                wKey [ 1 ] = * ( ( WORD * )( pBufferInfo ) );
                wKey [ 0 ] = * ( ( WORD * )( ( char * ) pBufferInfo + 2 ) );
                dwKey = * ( ( DWORD * )( wKey ) );
            }

            swprintf_s ( ModuleKey, _wsizeof ( ModuleKey ), L"\\StringFileInfo\\%08x\\ProductName", dwKey );
            bResult = VerQueryValueW ( pData, ModuleKey, &pBufferInfo, &iBufferLen );
            if ( bResult )
            {
                ZeroMemory ( ModuleText, sizeof(ModuleText) );
                memcpy ( ModuleText, pBufferInfo, iBufferLen * sizeof(WCHAR) );
                __strrepW ( ModuleText, _wsizeof(ModuleText), L"\r\n", L" - ", true );
#ifdef _WIN64
                __strrepW ( ModuleText, sizeof(ModuleText), L"x86", L"X64", true );
#else
                __strrepW ( ModuleText, sizeof(ModuleText), L"x64", L"X86", true );
#endif
                PrintDirectW ( L"%*s : %s\n", iWidth, L"Program", ModuleText );
            }

            swprintf_s ( ModuleKey, _wsizeof ( ModuleKey ), L"\\StringFileInfo\\%08x\\LegalCopyright", dwKey );
            bResult = VerQueryValueW ( pData, ModuleKey, &pBufferInfo, &iBufferLen );
            if ( bResult )
            {
                ZeroMemory ( ModuleText, sizeof(ModuleText) );
                memcpy ( ModuleText, pBufferInfo, iBufferLen * sizeof(WCHAR) );
                PrintDirectW ( L"%*s : %s\n", iWidth, L"Date", ModuleText );
            }

            swprintf_s ( ModuleKey, _wsizeof ( ModuleKey ), L"\\StringFileInfo\\%08x\\InternalName", dwKey );
            bResult = VerQueryValueW ( pData, ModuleKey, &pBufferInfo, &iBufferLen );
            if ( bResult )
            {
                ZeroMemory ( ModuleText, sizeof(ModuleText) );
                memcpy ( ModuleText, pBufferInfo, iBufferLen * sizeof(WCHAR) );
                PrintDirectW ( L"%*s : %s\n", iWidth, L"Web Site", ModuleText );
            }

            swprintf_s ( ModuleKey, _wsizeof ( ModuleKey ), L"\\StringFileInfo\\%08x\\Comments", dwKey );
            bResult = VerQueryValueW ( pData, ModuleKey, &pBufferInfo, &iBufferLen );
            if ( bResult )
            {
                ZeroMemory ( ModuleText, sizeof(ModuleText) );
                memcpy ( ModuleText, pBufferInfo, iBufferLen * sizeof(WCHAR) );
                PrintDirectW ( L"%*s : %s\n", iWidth, L"E/Mail", ModuleText );
            }

            swprintf_s ( ModuleKey, _wsizeof ( ModuleKey ), L"\\StringFileInfo\\%08x\\FileDescription", dwKey );
            bResult = VerQueryValueW ( pData, ModuleKey, &pBufferInfo, &iBufferLen );
            if ( bResult )
            {
                ZeroMemory ( ModuleText, sizeof(ModuleText) );
                memcpy ( ModuleText, pBufferInfo, iBufferLen * sizeof(WCHAR) );
                PrintDirectW ( L"%*s : %s\n", iWidth, L"Description", ModuleText );
            }
        }

        if ( pData != NULL )
        {
            delete pData;
        }

    }
}

//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
void PrintRealVersionA (int iWidth)
{
    const   size_t  LenString                       = 1024;
    static  char    ModuleFileNameA [ LenString ]   = "";
    static  char    ModuleKey [ LenString ]         = "";
    static  char    ModuleText [ LenString ]        = "";

    char            *pData                          = NULL;

    //      Get module name
    DWORD dwResult = GetModuleFileNameA ( NULL, ModuleFileNameA, sizeof ( ModuleFileNameA ) );

    //      Get Size to allocate for info
    DWORD           dwHandle;
    DWORD dwLength = GetFileVersionInfoSizeA ( ModuleFileNameA, &dwHandle );
    if ( dwLength > 0 )
    {
        pData = new char [ dwLength ];

        //  Get All Infos
        BOOL bResult = GetFileVersionInfoA(ModuleFileNameA, dwHandle, dwLength, pData );
        if ( bResult )
        {
            void            *pBufferInfo;
            UINT            iBufferLen;

            // Retrieve le language code: 40c or 409
            DWORD dwKey;

            bResult = VerQueryValueA ( pData, "\\VarFileInfo\\Translation", &pBufferInfo, &iBufferLen );
            if ( bResult  && iBufferLen == 4 )
            {
                WORD    wKey [ 2 ];
                wKey [ 1 ] = * ( ( WORD * )( pBufferInfo ) );
                wKey [ 0 ] = * ( ( WORD * )( ( char * ) pBufferInfo + 2 ) );
                dwKey = * ( ( DWORD * )( wKey ) );
            }

            sprintf_s ( ModuleKey, sizeof ( ModuleKey ), "\\StringFileInfo\\%08x\\ProductName", dwKey );
            bResult = VerQueryValueA ( pData, ModuleKey, &pBufferInfo, &iBufferLen );
            if ( bResult )
            {
                ZeroMemory ( ModuleText, sizeof(ModuleText) );
                memcpy ( ModuleText, pBufferInfo, iBufferLen * sizeof(WCHAR) );
                __strrepA ( ModuleText, sizeof(ModuleText), "\r\n", " - ", true );
#ifdef _WIN64
                __strrepA ( ModuleText, sizeof(ModuleText), "x86", "X64", true );
#else
                __strrepA ( ModuleText, sizeof(ModuleText), "x64", "X86", true );
#endif
                PrintDirectA ( "%*s : %s\n", iWidth, "Program", ModuleText );
            }

            sprintf_s ( ModuleKey, sizeof ( ModuleKey ), "\\StringFileInfo\\%08x\\LegalCopyright", dwKey );
            bResult = VerQueryValueA ( pData, ModuleKey, &pBufferInfo, &iBufferLen );
            if ( bResult )
            {
                ZeroMemory ( ModuleText, sizeof(ModuleText) );
                memcpy ( ModuleText, pBufferInfo, iBufferLen * sizeof(char) );
                PrintDirectA ( "%*s : %s\n", iWidth, "Date", ModuleText );
            }

            sprintf_s ( ModuleKey, sizeof ( ModuleKey ), "\\StringFileInfo\\%08x\\InternalName", dwKey );
            bResult = VerQueryValueA ( pData, ModuleKey, &pBufferInfo, &iBufferLen );
            if ( bResult )
            {
                ZeroMemory ( ModuleText, sizeof(ModuleText) );
                memcpy ( ModuleText, pBufferInfo, iBufferLen * sizeof(char) );
                PrintDirectA ( "%*s : %s\n", iWidth, "Web Site", ModuleText );
            }

            sprintf_s ( ModuleKey, _wsizeof ( ModuleKey ), "\\StringFileInfo\\%08x\\Comments", dwKey );
            bResult = VerQueryValueA ( pData, ModuleKey, &pBufferInfo, &iBufferLen );
            if ( bResult )
            {
                ZeroMemory ( ModuleText, sizeof(ModuleText) );
                memcpy ( ModuleText, pBufferInfo, iBufferLen * sizeof(char) );
                PrintDirectA ( "%*s : %s\n", iWidth, "E/Mail", ModuleText );
            }

            sprintf_s ( ModuleKey, sizeof ( ModuleKey ), "\\StringFileInfo\\%08x\\FileDescription", dwKey );
            bResult = VerQueryValueA ( pData, ModuleKey, &pBufferInfo, &iBufferLen );
            if ( bResult )
            {
                ZeroMemory ( ModuleText, sizeof(ModuleText) );
                memcpy ( ModuleText, pBufferInfo, iBufferLen * sizeof(char) );
                PrintDirectA ( "%*s : %s\n", iWidth, "Description", ModuleText );
            }
        }

        if ( pData != NULL )
        {
            delete pData;
        }

    }
}
