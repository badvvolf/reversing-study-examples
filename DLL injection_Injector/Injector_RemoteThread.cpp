
#include<stdio.h>
#include<windows.h>
#include<tchar.h>

//��Ƽ����Ʈ�� ����
//�����ڵ� ������ "" �տ� L�� ���̰�, LoadLibraryW�� ��� �Ѵ�.

BOOL InjectDll(DWORD dwPID, LPCTSTR szDllPath)
{
	HANDLE hProcess = NULL, hThread = NULL;
	HMODULE hMod = NULL;
	LPVOID pRemoteBuf = NULL; 
	DWORD dwBufSize = (DWORD)(_tcslen(szDllPath) + 1) * sizeof(TCHAR);
	LPTHREAD_START_ROUTINE pThreadProc;


	//��� ���μ����� �����ϱ� ���� ����.
	if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS/*XP�� ��� MAXIMUX_ALLOWED*/, FALSE, dwPID) ) )
	{ 
		_tprintf("OpenProcess fail!\n");
		return FALSE;
	}

	//��� ���μ����� �޸𸮿� �����͸� ���� ���� �ڸ��� �Ҵ��Ѵ�.
	pRemoteBuf = VirtualAllocEx(hProcess, NULL, dwBufSize, MEM_COMMIT, PAGE_READWRITE);


	//��� ���μ����� �޸𸮿� DLL path�� �����Ѵ�. 
	WriteProcessMemory(hProcess, pRemoteBuf, (LPVOID)szDllPath, dwBufSize, NULL);

	hMod = GetModuleHandle("kernel32.dll");
	pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(hMod, "LoadLibraryA");


	//���� ������ ����
	//��� ���μ����� �����带 �����ϵ��� ��
	//pThreadProc�� LoadLibraryW, ���ڴ� Dll path�� ������ �ּ�
	hThread = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, pRemoteBuf, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);


	CloseHandle(hThread);
	CloseHandle(hProcess);
	
	return TRUE;

}


int _tmain(int argc, TCHAR * argv[])
{
	if (argc != 3)
	{
		_tprintf("Usage : %s pid dll_path\n", argv[0]);
		return 1;
	}

	if (InjectDll((DWORD)_tstol(argv[1]), argv[2]))
		_tprintf("Injection success!\n");
	else
		_tprintf("Injection fail\n");

	return 0;


}