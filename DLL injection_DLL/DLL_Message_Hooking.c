#include<windows.h>
#include<stdio.h>
#include<string.h>

#define DEF_PROCESS_NAME "notepad.exe"

HINSTANCE g_hInstance = NULL;
HHOOK g_hHook = NULL;
HWND g_hWnd = NULL;

//Ű���� �̺�Ʈ�� �߻��ϸ� �� �Լ��� ȣ��� ��
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{

	char szPath[MAX_PATH] = { 0, };
	char * p = NULL;


	//�� ���� ������ �� ���ν����� �� �޽����� ó������ ���� ���� �� ���ν������� �Ѱܾ� ��
	if (nCode >= 0)
	{

		//31���� ��Ʈ��	0�� ��� Ű�� ���� ��
		//				1�� ��� Ű�� �� ��
		//Ű�� ���� ��쿡�� �Ʒ� �۾��� �ϵ��� ��
		if (!(lParam & 0x80000000))
		{
			//�� �Լ��� ȣ���� ���μ����� �̸��� ��´�. 
			GetModuleFileNameA(NULL, szPath, MAX_PATH);

			p = strrchr(szPath, '\\');

			//���� notepad.exe��� �޽����� ���� �������α׷�(Ȥ�� ��)���� ���޵��� ����
			//notepad�� ���� �޽����� �����Ѵ�. 
			if (!_stricmp(p + 1, DEF_PROCESS_NAME))
				return 1;

		}
		
	}

	// �ش� ���μ����� notepad�� �ƴϰų� nCode�� ������ ���
	// ���� �������α׷��̳� ������ ����
	return CallNextHookEx(g_hHook, nCode, wParam, lParam);
	
}



BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
	char szPath[MAX_PATH] = { 0, };

	char * szProcess = NULL;

	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		//���������� �ڵ��� �ڽ�(DLL)���� ����
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