#pragma once

#ifdef _WIN64
#ifdef UNICODE
#define ORIGINAL_FILENAME   L"SmartInfo (x64 Unicode) (MSVC)\0"
#define PRODUCT_NAME        L"SmartInfo - Version 1.0.04.016\r\n(Build 69) - (x64 Unicode) (MSVC)\0"
#else
#define ORIGINAL_FILENAME   "SmartInfo (x64 MBCS) (MSVC)\0"
#define PRODUCT_NAME        "SmartInfo - Version 1.0.04.016\r\n(Build 69) - (x64 MBCS) (MSVC)\0"
#endif
#elif _WIN32
#ifdef UNICODE
#define ORIGINAL_FILENAME   L"SmartInfo (x86 Unicode) (MSVC)\0"
#define PRODUCT_NAME        L"SmartInfo - Version 1.0.04.016\r\n(Build 69) - (x86 Unicode) (MSVC)\0"
#else
#define ORIGINAL_FILENAME   "SmartInfo (x86 MBCS) (MSVC)\0"
#define PRODUCT_NAME        "SmartInfo - Version 1.0.04.016\r\n(Build 69) - (x86 MBCS) (MSVC)\0"
#endif
#else
#define ORIGINAL_FILENAME   "SmartInfo (MSVC)\0"
#define PRODUCT_NAME        "SmartInfo - Version 1.0.04.016\r\n(Build 69) - (MSVC)\0"
#endif
