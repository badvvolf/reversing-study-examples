
#include<stdio.h>
#include<windows.h>
#include<tchar.h>

//멀티바이트로 설정
//유니코드 설정시 "" 앞에 L을 붙이고, LoadLibraryW를 써야 한다.

BOOL InjectDll(DWORD dwPID, LPCTSTR szDllPath)
{
	HANDLE hProcess = NULL, hThread = NULL;
	HMODULE hMod = NULL;
	LPVOID pRemoteBuf = NULL; 
	DWORD dwBufSize = (DWORD)(_tcslen(szDllPath) + 1) * sizeof(TCHAR);
	LPTHREAD_START_ROUTINE pThreadProc;


	//대상 프로세스를 통제하기 위해 연다.
	if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS/*XP일 경우 MAXIMUX_ALLOWED*/, FALSE, dwPID) ) )
	{ 
		_tprintf("OpenProcess fail!\n");
		return FALSE;
	}

	//대상 프로세스의 메모리에 데이터를 쓰기 위해 자리를 할당한다.
	pRemoteBuf = VirtualAllocEx(hProcess, NULL, dwBufSize, MEM_COMMIT, PAGE_READWRITE);


	//대상 프로세스의 메모리에 DLL path를 저장한다. 
	WriteProcessMemory(hProcess, pRemoteBuf, (LPVOID)szDllPath, dwBufSize, NULL);

	hMod = GetModuleHandle("kernel32.dll");
	pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(hMod, "LoadLibraryA");


	//원격 쓰레드 실행
	//대상 프로세스가 쓰레드를 실행하도록 함
	//pThreadProc은 LoadLibraryW, 인자는 Dll path를 저장한 주소
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