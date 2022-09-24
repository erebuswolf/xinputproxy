/* Copyright 2021 Mircea-Dacian Munteanu
 *
 * The Source Code is this file is released under the terms of the New BSD License,
 * see LICENSE file, or the project homepage: https://github.com/mircead52/xinputproxy
 */

// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <stdlib.h>
#include <math.h>

#include <XInput.h>

#include "config.h"

//disable deprecated error
#pragma warning(disable : 4995)

/**
 * macros and defines
 */

#define TEXTIFYMACRO(X) _TEXT(X)
#define LOG_FILENAME TEXTIFYMACRO(MY_NAME".log")

#define MY_TOKEN_VAL 0x5aB63792u

/**
 * globals
 */
DWORD xinputoken = MY_TOKEN_VAL;
HINSTANCE dllHandle;

TCHAR original_dll[PATH_SZ] = TEXTIFYMACRO(".\\" MY_NAME "_rename.dll");
TCHAR myname[PATH_SZ] = TEXTIFYMACRO(MY_NAME);

/**
 * function macros and function declarations
 */
#define FREE(X) if(X) { free(X); X = NULL; }

#define LOADDLL() do { if(dllHandle == NULL) loadDll(); } while(0)

#define CHECK_RETCODE(X) do { if(ERROR_SUCCESS != (X)) LogInfo("Function %s failed with code: 0x%x\r\n", __func__, X); } while(0)

static void loadFunction(HINSTANCE dllHandle, LPCSTR name, FARPROC* found);

DWORD dummy_func() { return -1; }

FARPROC fp_XInputGetState = NULL;
FARPROC fp_XInputSetState = NULL;
FARPROC fp_XInputGetCapabilities = NULL;
FARPROC fp_XInputGetDSoundAudioDeviceGuids = NULL;
FARPROC fp_XInputEnable = NULL;
FARPROC fp_XInputGetAudioDeviceIds = NULL;
FARPROC fp_XInputGetBatteryInformation = NULL;
FARPROC fp_XInputGetKeystroke = NULL;

static void loadDll()
{
    if (dllHandle == NULL)
    {
        read_conf(myname);
        LOGINFO("Loading dll: %s\r\n", original_dll);
        //Load the dll and keep the handle to it
        dllHandle = LoadLibraryEx(original_dll, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (dllHandle == NULL) {
            return;
        }
        DWORD* token_adr = (DWORD*)GetProcAddress(dllHandle, "xinputoken");
        if (token_adr != NULL && *token_adr == MY_TOKEN_VAL)
        {
            LogInfo("Token detected, assuming recursive load\r\n");
            FreeLibrary(dllHandle);
            dllHandle = NULL;
            return;
        }

        //Get pointer to our functions using GetProcAddress:
        loadFunction(dllHandle, "XInputGetState", &fp_XInputGetState);
        loadFunction(dllHandle, "XInputSetState", &fp_XInputSetState);
        loadFunction(dllHandle, "XInputGetCapabilities", &fp_XInputGetCapabilities);
        loadFunction(dllHandle, "XInputGetDSoundAudioDeviceGuids", &fp_XInputGetDSoundAudioDeviceGuids);
        loadFunction(dllHandle, "XInputEnable", &fp_XInputEnable);
        loadFunction(dllHandle, "XInputGetAudioDeviceIds", &fp_XInputGetAudioDeviceIds);
        loadFunction(dllHandle, "XInputGetBatteryInformation", &fp_XInputGetBatteryInformation);
        loadFunction(dllHandle, "XInputGetKeystroke", &fp_XInputGetKeystroke);

        LogInfo("LoadDll finished\r\n");
    }
}

float get_radius(SHORT X, SHORT Y)
{
    float a = (float)X, b = (float)(Y);

    return sqrtf(a * a + b * b);
}

#define RADIUS_MAX (float)(0x7FFF)
#define INPUT_DEADZONE  ( 0.24f * RADIUS_MAX )
#define MAX_CONTROLLERS 4

typedef DWORD (*fpt_XInputGetState)(DWORD         dwUserIndex, XINPUT_STATE* pState);
DWORD WINAPI XInputGetState
(
    _In_  DWORD         dwUserIndex,  // Index of the gamer associated with the device
    _Out_ XINPUT_STATE* pState        // Receives the current state
)
{
    static bool override_in_effect[MAX_CONTROLLERS] = { false };
    static bool override_button_pressed[MAX_CONTROLLERS] = { false };
    LOADDLL();
    LogInfo("Called %s: ", __func__);
    LogInfo(" idx:%d state:%p", dwUserIndex, pState);
    LogInfo("\r\n");
    DWORD ercd = ((fpt_XInputGetState)fp_XInputGetState)(dwUserIndex, pState);
    if (ERROR_SUCCESS != ercd) return ercd;

    float r = get_radius(pState->Gamepad.sThumbLX, pState->Gamepad.sThumbLY);
    float newr = RADIUS_MAX * configx.crop_left_stick_value;

    if (configx.crop_left_stick_override & pState->Gamepad.wButtons)
    {
        override_button_pressed[dwUserIndex] = true;
    }
    if ((configx.crop_left_stick_override & pState->Gamepad.wButtons) == 0 && override_button_pressed[dwUserIndex])
    {
        override_button_pressed[dwUserIndex] = false;
        //button was pressed, flip override
        override_in_effect[dwUserIndex] = !override_in_effect[dwUserIndex];
    }
    if ( !override_in_effect[dwUserIndex] && (r < INPUT_DEADZONE))
    {
        //stick at middle position, enable override
        override_in_effect[dwUserIndex] = true;
    }
    if (override_in_effect[dwUserIndex] && (r > newr))
    {
        float newx = (float)(pState->Gamepad.sThumbLX) * newr / r;
        float newy = (float)(pState->Gamepad.sThumbLY) * newr / r;
        pState->Gamepad.sThumbLX = (SHORT)newx;
        pState->Gamepad.sThumbLY = (SHORT)newy;
    }
    //CHECK_RETCODE(ercd);
    return ercd;
}

typedef DWORD(*fpt_XInputSetState)(DWORD         dwUserIndex, XINPUT_VIBRATION* pVibration);
DWORD WINAPI XInputSetState
(
    _In_ DWORD             dwUserIndex,  // Index of the gamer associated with the device
    _In_ XINPUT_VIBRATION* pVibration    // The vibration information to send to the controller
)
{
    LOADDLL();
    LogInfo("Called %s: ", __func__);
    LogInfo(" idx:%d state:%p", dwUserIndex, pVibration);
    LogInfo("\r\n");
    DWORD ercd = ((fpt_XInputSetState)fp_XInputSetState)(dwUserIndex, pVibration);
    //CHECK_RETCODE(ercd);
    return ercd;
}

typedef DWORD(*fpt_XInputGetCapabilities)(DWORD         dwUserIndex, DWORD                dwFlags, XINPUT_CAPABILITIES* pCapabilities);
DWORD WINAPI XInputGetCapabilities
(
    _In_  DWORD                dwUserIndex,   // Index of the gamer associated with the device
    _In_  DWORD                dwFlags,       // Input flags that identify the device type
    _Out_ XINPUT_CAPABILITIES* pCapabilities  // Receives the capabilities
)
{
    LOADDLL();
    LogInfo("Called %s: ", __func__);
    LogInfo(" idx:%d flags:0x%x", dwUserIndex, dwFlags);
    LogInfo("\r\n");
    DWORD ercd = ((fpt_XInputGetCapabilities)fp_XInputGetCapabilities)(dwUserIndex, dwFlags, pCapabilities);
    //CHECK_RETCODE(ercd);
    return ercd;
}

typedef DWORD(*fpt_XInputGetDSoundAudioDeviceGuids)(DWORD         dwUserIndex, GUID* pDSoundRenderGuid, GUID* pDSoundCaptureGuid);
DWORD XInputGetDSoundAudioDeviceGuids(
    DWORD dwUserIndex,
    GUID* pDSoundRenderGuid,
    GUID* pDSoundCaptureGuid
)
{
    LOADDLL();
    LogInfo("Called %s: ", __func__);
    LogInfo(" idx:%d", dwUserIndex);
    LogInfo("\r\n");
    DWORD ercd = ((fpt_XInputGetDSoundAudioDeviceGuids)fp_XInputGetDSoundAudioDeviceGuids)(dwUserIndex, pDSoundRenderGuid, pDSoundCaptureGuid);
    //CHECK_RETCODE(ercd);
    return ercd;
}

typedef void(*fpt_XInputEnable)(BOOL enable);
void  WINAPI XInputEnable(
    _In_ BOOL enable
)
{
    LOADDLL();
    LogInfo("Called %s: ", __func__);
    LogInfo(" enable:%d", enable);
    LogInfo("\r\n");
    ((fpt_XInputEnable)fp_XInputEnable)(enable);
    return;
}

typedef DWORD(*fpt_XInputGetAudioDeviceIds)(DWORD dwUserIndex, LPWSTR  pRenderDeviceId, UINT* pRenderCount, LPWSTR pCaptureDeviceId, UINT* pCaptureCount);
DWORD WINAPI XInputGetAudioDeviceIds
(
    _In_  DWORD                             dwUserIndex,        // Index of the gamer associated with the device
    _Out_writes_opt_(*pRenderCount) LPWSTR  pRenderDeviceId,    // Windows Core Audio device ID string for render (speakers)
    _Inout_opt_ UINT* pRenderCount,       // Size of render device ID string buffer (in wide-chars)
    _Out_writes_opt_(*pCaptureCount) LPWSTR pCaptureDeviceId,   // Windows Core Audio device ID string for capture (microphone)
    _Inout_opt_ UINT* pCaptureCount       // Size of capture device ID string buffer (in wide-chars)
)
{
    LOADDLL();
    LogInfo("Called %s: ", __func__);
    LogInfo(" idx:%d", dwUserIndex);
    LogInfo("\r\n");
    DWORD ercd = ((fpt_XInputGetAudioDeviceIds)fp_XInputGetAudioDeviceIds)(dwUserIndex, pRenderDeviceId, pRenderCount, pCaptureDeviceId, pCaptureCount);
    //CHECK_RETCODE(ercd);
    return ercd;
}

typedef DWORD(*fpt_XInputGetBatteryInformation)(DWORD dwUserIndex, BYTE devType, XINPUT_BATTERY_INFORMATION* pBatteryInformation);
DWORD WINAPI XInputGetBatteryInformation
(
    _In_  DWORD                       dwUserIndex,        // Index of the gamer associated with the device
    _In_  BYTE                        devType,            // Which device on this user index
    _Out_ XINPUT_BATTERY_INFORMATION* pBatteryInformation // Contains the level and types of batteries
)
{
    LOADDLL();
    LogInfo("Called %s: ", __func__);
    LogInfo(" idx:%d", dwUserIndex);
    LogInfo("\r\n");
    DWORD ercd = ((fpt_XInputGetBatteryInformation)fp_XInputGetBatteryInformation)(dwUserIndex, devType, pBatteryInformation);
    //CHECK_RETCODE(ercd);
    return ercd;
}

typedef DWORD(*fpt_XInputGetKeystroke)(DWORD dwUserIndex, DWORD dwReserved, PXINPUT_KEYSTROKE pKeystroke);
DWORD WINAPI XInputGetKeystroke
(
    _In_       DWORD dwUserIndex,              // Index of the gamer associated with the device
    _Reserved_ DWORD dwReserved,               // Reserved for future use
    _Out_      PXINPUT_KEYSTROKE pKeystroke    // Pointer to an XINPUT_KEYSTROKE structure that receives an input event.
)
{
    LOADDLL();
    LogInfo("Called %s: ", __func__);
    LogInfo(" idx:%d", dwUserIndex);
    LogInfo("\r\n");
    DWORD ercd = ((fpt_XInputGetKeystroke)fp_XInputGetKeystroke)(dwUserIndex, dwReserved, pKeystroke);
    //CHECK_RETCODE(ercd);
    return ercd;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    DWORD ercd = 0;
    TCHAR* name = 0;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        TCHAR mfname[PATH_SZ];
        TCHAR logfname[PATH_SZ];
        DisableThreadLibraryCalls(hModule);
        ercd = GetModuleFileName(hModule, mfname, PATH_SZ);
        if (ercd > 0)
        {
            name = _tcsrchr(mfname, _T('\\'));
            if (name) {
                name++;
                TCHAR* tmp = _tcsrchr(mfname, _T('.'));
                if (tmp) {
                    tmp[0] = 0; //terminate string before .dll
                }
            }
        }
        if (name)
        {
            _stprintf_s(logfname, _T("%s.log"), name);
            CreateLogFile(logfname);
            _stprintf_s(original_dll, _T(".\\%s.dll"), name);
            _stprintf_s(myname, _T("%s"), name);
        }
        else
        {
            CreateLogFile(LOG_FILENAME);
            LOGINFO("Dll filename extraction failed for: %s\r\n", mfname);
        }
        LogInfo("DllMain finished!!!!\r\n");
        LogInfo("DllMain finished\r\n");
        break;
    case DLL_THREAD_ATTACH: break;
    case DLL_THREAD_DETACH: break;
    case DLL_PROCESS_DETACH:
        CloseLogFile();
        break;
    }
    return TRUE;
}

static void loadFunction(HINSTANCE dllHandle, LPCSTR name, FARPROC* found)
{
    if (found)
    {
        *found = GetProcAddress(dllHandle, name);

        if (*found == NULL)
        {
            LogInfo("Failed to find function %s\r\n", name);
            *found = (FARPROC)dummy_func;
        }

        LogInfo("Found function %s\r\n", name);
    }
}
