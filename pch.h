/* Copyright 2021 Mircea-Dacian Munteanu
 *
 * The Source Code is this file is released under the terms of the New BSD License,
 * see LICENSE file, or the project homepage: https://github.com/mircead52/xinputproxy
 */

// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"

#include <tchar.h>


 //#undef MY_NAME
#ifndef MY_NAME
#define MY_NAME "xinput"
#endif

#define PATH_SZ (4096)

void CreateLogFile(PCTCH filename);
void CloseLogFile();
void LogInfo(const char* format, ...);
void LogInfoW(const wchar_t* format, ...);

#ifdef UNICODE
#define LOGINFO(X, ...)  LogInfoW(__TEXT(X), __VA_ARGS__)
#else
#define LOGINFO  LogInfo
#endif // !UNICODE

#endif //PCH_H
