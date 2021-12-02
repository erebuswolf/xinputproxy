/* Copyright 2021 Mircea-Dacian Munteanu
 *
 * The Source Code is this file is released under the terms of the New BSD License,
 * see LICENSE file, or the project homepage: https://github.com/mircead52/dinputproxy
 */

#include "pch.h"
#include <stdio.h>
#include <wchar.h>

static HANDLE log_file = NULL;

void LogInfo(const char* format, ...)
{
	int ercd;
	va_list vl;
	char buff[1000];

	va_start(vl, format);

	ercd = vsnprintf_s(buff, sizeof(buff), _TRUNCATE, format, vl);
	va_end(vl);

	if (ercd < 0) return;
	if (ercd > sizeof(buff)) ercd = sizeof(buff);


	if (log_file == NULL)
	{
		const char* pbuff = buff;
		size_t count = 0;
		wchar_t wbuff[1000];
		mbstate_t state;
		count = mbsrtowcs(wbuff, &pbuff, sizeof(buff), &state);
		if (count < 0) return;
		MessageBox(NULL, wbuff, _TEXT("Proxy Info"), MB_OK);
	}
	else
	{
		DWORD written;
		WriteFile(log_file, buff, ercd, &written, NULL);
	}
}

void LogInfoW(const wchar_t* format, ...)
{
	int ercd;
	size_t count = 0;
	va_list vl;
	wchar_t wbuff[1000];
	const wchar_t* pwbuff = wbuff;
	char buff[1000];

	wbuff[0] = 0; buff[0] = 0;
	va_start(vl, format);

	ercd = _vsnwprintf_s(wbuff, sizeof(wbuff) / sizeof(wchar_t), _TRUNCATE, format, vl);
	va_end(vl);

	mbstate_t state;
	count = wcsrtombs((char*)buff, (const wchar_t**)&pwbuff, (size_t)sizeof(buff), &state);

	if (count < 0) return;
	if (count > sizeof(buff)) count = sizeof(buff);

	if (log_file == NULL)
	{
		MessageBox(NULL, wbuff, _TEXT("Proxy Info"), MB_OK);
	}
	else
	{
		DWORD written;
		WriteFile(log_file, buff, (DWORD)count, &written, NULL);
	}
}

void CreateLogFile(PCTCH filename)
{
	if (log_file == NULL)
	{
		HANDLE h = CreateFile(filename,	// file name 
			GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_DELETE,
			NULL,                  // default security 
			CREATE_ALWAYS,         // existing file only 
			FILE_ATTRIBUTE_NORMAL, // normal file 
			NULL);                 // no template 
		if (h == INVALID_HANDLE_VALUE)
		{
			TCHAR newfn[4096] = { 0 };
			DWORD sz = GetTempPath(sizeof(newfn) / sizeof(newfn[0]), newfn);
			_tcsncat(newfn, filename, sizeof(newfn) / sizeof(newfn[0]) - sz -1);
			h = CreateFile(newfn,	// file name 
				GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_DELETE,
				NULL,                  // default security 
				CREATE_ALWAYS,         // existing file only 
				FILE_ATTRIBUTE_NORMAL, // normal file 
				NULL);                 // no template 
			if (h != INVALID_HANDLE_VALUE) {
				LOGINFO("Logfile %s failed to open; %s was used instead \r\n", filename, newfn);
			}
		}
		if (h != INVALID_HANDLE_VALUE) {
			log_file = h;
		}
	}
}

void CloseLogFile()
{
	if (log_file)
	{
		CloseHandle(log_file);
		log_file = NULL;
	}
}