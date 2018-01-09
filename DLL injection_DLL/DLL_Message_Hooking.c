#include<windows.h>
#include<stdio.h>
#include<string.h>

#define DEF_PROCESS_NAME "notepad.exe"

HINSTANCE g_hInstance = NULL;
HHOOK g_hHook = NULL;
HWND g_hWnd = NULL;

//키보드 이벤트가 발생하면 이 함수가 호출될 것
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{

	char szPath[MAX_PATH] = { 0, };
	char * p = NULL;


	//이 값이 음수면 훅 프로시저는 이 메시지를 처리하지 말고 다음 훅 프로시저에게 넘겨야 함
	if (nCode >= 0)
	{

		//31번쨰 비트가	0일 경우 키를 누른 것
		//				1일 경우 키를 뗀 것
		//키를 누른 경우에만 아래 작업을 하도록 함
		if (!(lParam & 0x80000000))
		{
			//이 함수를 호출한 프로세스의 이름을 얻는다. 
			GetModuleFileNameA(NULL, szPath, MAX_PATH);

			p = strrchr(szPath, '\\');

			//만약 notepad.exe라면 메시지는 다음 응용프로그램(혹은 훅)으로 전달되지 않음
			//notepad에 들어온 메시지를 삭제한다. 
			if (!_stricmp(p + 1, DEF_PROCESS_NAME))
				return 1;

		}
		
	}

	// 해당 프로세스가 notepad가 아니거나 nCode가 음수일 경우
	// 다음 응용프로그램이나 훅으로 전달
	return CallNextHookEx(g_hHook, nCode, wParam, lParam);
	
}



BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
	char szPath[MAX_PATH] = { 0, };

	char * szProcess = NULL;

	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		//전역변수의 핸들을 자신(DLL)으로 설정
		g_hInstance = hinstDLL;
		break;
	case DLL_PROCESS_DETACH:
		g_hInstance = 0;
		break;

	}

	return TRUE;

}


__declspec(dllexport) void HookStart()
{
	g_hHook = SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, g_hInstance, 0);
}

__declspec(dllexport) void HookStop()
{

	if (g_hHook)
	{
		UnhookWindowsHookEx(g_hHook);
		g_hHook = NULL;
		
	}

}