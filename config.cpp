/* Copyright 2021 Mircea-Dacian Munteanu
 *
 * The Source Code is this file is released under the terms of the New BSD License,
 * see LICENSE file, or the project homepage: https://github.com/mircead52/xinputproxy
 */

#include "pch.h"
#include "config.h"


#include "inih\ini.h"

//#define LogInfo(...)


#define GROUP_DLL_PATH "XInputPath"
#define GROUP_OPTIONS "Options"

struct proxyconfig configx = { 0 };

static int handler(void* user, const char* section, const char* name,
    const char* value)
{
#define MATCH(s, n)  ((strcmp(section, s) == 0) && (strcmp(name, n) == 0))
#define MATCH_2BEG(s, n) ((strcmp(section, s) == 0) && (strncmp(name, n, sizeof(n) -1) == 0))

    if (MATCH(GROUP_DLL_PATH, "Path"))
    {
        configx.dllpath = _strdup(value);
        LogInfo("config path %s\r\n", configx.dllpath);
    }
    else if (MATCH(GROUP_OPTIONS, "CropLeftStick"))
    {
        float tmp = 0.0;
        sscanf(value, "%f", &tmp);
        if (tmp > 0. && tmp < 1.)
        {
            configx.crop_left_stick_value = tmp;
        }
        LogInfo("config LS crop %f\r\n", tmp);
    }
    else if (MATCH(GROUP_OPTIONS, "CropLeftStickOverride"))
    {
        int tmp = 0;
        sscanf(value, "%x", &tmp);
        configx.crop_left_stick_override = tmp;
        LogInfo("config LS crop override %d 0x%x\r\n", tmp, tmp);
    }

    return 1;
}

#define __WT(x)      L ## x
#define _WT(x)      __WT(x)

#include <memory>
#include <Shlwapi.h>
#include <Xinput.h>

void read_conf(TCHAR* module_name)
{
    size_t bsz = 10 + _tcslen(module_name);

    char* fname = new char[bsz]();
    std::unique_ptr<char[]> auto_free_fname(fname);

    TCHAR* buf = new TCHAR[bsz]();
    std::unique_ptr<TCHAR[]> auto_free_buf(buf);

    //set default values
    configx.crop_left_stick_value = 0.9f;
    configx.crop_left_stick_override = XINPUT_GAMEPAD_LEFT_THUMB;

    _stprintf_p(buf, bsz, _T(".\\%s.ini"), module_name);

    if (!PathFileExists(buf))
    {
        _stprintf_p(buf, bsz, _T(".\\%s.ini"), _T(MY_NAME));
    }
    if (!PathFileExists(buf))
    {
        //no config file available
        LogInfoW(L"Config file %s.ini or %s.ini not found, using default values\r\n", module_name, _WT( MY_NAME ));
        return;
    }

    if (sizeof(TCHAR) == sizeof(char))
    {
        strncpy(fname, (const char *)buf, bsz);
    }
    else
    {
        mbstate_t state;
        wcsrtombs(fname, (const TCHAR **)&buf, bsz, &state);
    }

    int ercd;
    if ((ercd = ini_parse(fname, handler, NULL)) < 0)
    {
        LogInfo("Reading config file failed %d\r\n", ercd);
    }
    else
    {
        LogInfo("Config file read\r\n", ercd);
    }
}