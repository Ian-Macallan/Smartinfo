#pragma once

#ifndef AUTOMATICPRODUCTNAME_H
#define AUTOMATICPRODUCTNAME_H    1

#ifdef _WIN64
#ifdef UNICODE
#define ORIGINAL_FILENAME   L"SmartInfo (x64 Unicode) (MSVC)\0"
#define PRODUCT_NAME        L"SmartInfo - Version 1.0.05.002\r\n(Build 74) - (x64 Unicode) (MSVC)\0"
#else
#define ORIGINAL_FILENAME   "SmartInfo (x64 MBCS) (MSVC)\0"
#define PRODUCT_NAME        "SmartInfo - Version 1.0.05.002\r\n(Build 74) - (x64 MBCS) (MSVC)\0"
#endif
#elif _WIN32
#ifdef UNICODE
#define ORIGINAL_FILENAME   L"SmartInfo (x86 Unicode) (MSVC)\0"
#define PRODUCT_NAME        L"SmartInfo - Version 1.0.05.002\r\n(Build 74) - (x86 Unicode) (MSVC)\0"
#else
#define ORIGINAL_FILENAME   "SmartInfo (x86 MBCS) (MSVC)\0"
#define PRODUCT_NAME        "SmartInfo - Version 1.0.05.002\r\n(Build 74) - (x86 MBCS) (MSVC)\0"
#endif
#else
#define ORIGINAL_FILENAME   "SmartInfo (MSVC)\0"
#define PRODUCT_NAME        "SmartInfo - Version 1.0.05.002\r\n(Build 74) - (MSVC)\0"
#endif

#endif
