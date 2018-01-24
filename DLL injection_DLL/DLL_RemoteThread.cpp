
#include<windows.h>

HMODULE g_hMod = NULL;
HANDLE hThread = NULL;

extern "C" __declspec(dllexport) void dummy()
{
	return;
}
void WINAPI my_Function()
{
	MessageBoxA(NULL, "hello", "do something weird", 0);
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	g_hMod = (HMODULE)hinstDLL;

	switch (fdwReason)
	{

	case DLL_PROCESS_ATTACH:
		OutputDebugStringA("Dll excuted!!");

		my_Function(); 
		break;
		
	}

	return TRUE;

}