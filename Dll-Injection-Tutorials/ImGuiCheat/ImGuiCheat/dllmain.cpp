// dllmain.cpp : Define el punto de entrada de la aplicación DLL.
#include "pch.h"

typedef struct CheatOptions_s
{
    bool bWallhack = false;
}CheatOptions_t;

CheatOptions_t g_CheatOptions;

static BOOL (APIENTRY* pOrig_wglSwapBuffers)(HDC unnamedParam1) = 0;
static void (APIENTRY* pOrig_glBegin)(GLenum mode) = glBegin;
WNDPROC pOrig_WndProc = 0;
bool bImGuiInitialized = false;
bool bCheatMenuActive = true;
HGLRC hRealContext = 0, hNewContext = 0;
HWND hdHalfLife = 0;

LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK HOOK_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (bImGuiInitialized)
    {
		if (uMsg == WM_KEYUP && wParam == VK_INSERT)
		{
			bCheatMenuActive = !bCheatMenuActive;
		}
        if (bCheatMenuActive && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
            return TRUE;
    }
	return CallWindowProc(pOrig_WndProc, hWnd, uMsg, wParam, lParam);
}

BOOL WINAPI HOOK_wglSwapBuffers(HDC unnamedParam1)
{
    if (!pOrig_wglSwapBuffers)
        return TRUE;

	if (!bImGuiInitialized)
	{
        HWND hdHalfLife = ::WindowFromDC(unnamedParam1);
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::StyleColorsDark();
		ImGui_ImplWin32_Init(hdHalfLife);
		ImGui_ImplOpenGL2_Init();
        pOrig_WndProc = (WNDPROC)::SetWindowLong(hdHalfLife, GWL_WNDPROC, (LONG)HOOK_WndProc);
        hRealContext = ::wglGetCurrentContext();
        hNewContext = ::wglCreateContext(unnamedParam1);

		bImGuiInitialized = true;
	}

    wglMakeCurrent(unnamedParam1, hNewContext);
	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

    if (bCheatMenuActive)
    {
        ImGui::ShowDemoWindow();

		ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_Once);
		if (ImGui::Begin("Cheat Menu", &bCheatMenuActive))
		{
			ImGui::Checkbox("Wallhack", &g_CheatOptions.bWallhack);
		}
		ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    wglMakeCurrent(unnamedParam1, hRealContext);

    return pOrig_wglSwapBuffers(unnamedParam1);
}

void APIENTRY Hook_glBegin(GLenum mode)
{
    if (g_CheatOptions.bWallhack)
    {
        if (mode == GL_TRIANGLE_STRIP)
        {
            glDisable(GL_DEPTH_TEST);
        }
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
		pOrig_wglSwapBuffers = (decltype(pOrig_wglSwapBuffers))DetourFindFunction("opengl32.dll", "wglSwapBuffers");
        DetourAttach(&(PVOID&)pOrig_glBegin, Hook_glBegin);
		DetourAttach(&(PVOID&)pOrig_wglSwapBuffers, HOOK_wglSwapBuffers);
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

