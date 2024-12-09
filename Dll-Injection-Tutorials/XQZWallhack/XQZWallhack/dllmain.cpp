// dllmain.cpp : Define el punto de entrada de la aplicación DLL.
#include "pch.h"

static void (APIENTRY* pOrig_glBegin)(GLenum mode) = glBegin;

void APIENTRY Hook_glBegin(GLenum mode)
{
    if (mode == GL_TRIANGLE_STRIP)
    {
        glDisable(GL_DEPTH_TEST);
    }
    pOrig_glBegin(mode);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    LONG error;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)pOrig_glBegin, Hook_glBegin);
        error = DetourTransactionCommit();

        if (error == NO_ERROR) {
            MessageBoxA(0, "ogldet" DETOURS_STRINGIFY(DETOURS_BITS) ".dll:", " Detoured glBegin().\n", MB_ICONINFORMATION);
        }
        else {
            MessageBoxA(0, "ogldet" DETOURS_STRINGIFY(DETOURS_BITS) ".dll:", " Error detouring glBegin().\n", MB_ICONERROR);
        }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

