#include<stdio.h>
#include<windows.h>
#include<tlhelp32.h>
#include<tchar.h>
#include<stdlib.h>


BOOL EjectDll(DWORD dwPID, LPCTSTR szDllName);

int main(int argc, char * argv[])
{

	DWORD dwPID;

	if (argc != 3)
		printf("usage %s : pid DLLName", argv[0]);

	//입력받은 프로세스 ID를 숫자로 변환
	dwPID = strtol(argv[1], NULL, 10);

	EjectDll(dwPID, (LPCTSTR)argv[2]);

	return 0;

}



BOOL EjectDll(DWORD dwPID, LPCTSTR szDllName)
{
	HANDLE hSnapshot, hProcess, hThread;
	MODULEENTRY32 me = { sizeof(me) };
	BOOL bMore = FALSE, bFound = FALSE;
	LPTHREAD_START_ROUTINE pThreadProc;

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);

	bMore = Module32First(hSnapshot, &me);
	for (; bMore; bMore = Module32Next(hSnapshot, &me))
	{

		if (!_tcsicmp((LPCTSTR)me.szModule, szDllName) || !_tcsicmp((LPCTSTR)me.szExePath, szDllName))
		{
			bFound = TRUE;
			break;

		}

	}


	if (!bFound)
	{
		OutputDebugStringA("No Dll here");
		CloseHandle(hSnapshot);
		return FALSE;
	}

	if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID)))
	{

		OutputDebugStringA("error OpenProcess");
		return FALSE;
	}

	pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "FreeLibrary");

	hThread = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, me.hModule, 0, NULL);

	WaitForSingleObject(hThread, INFINITE);

	CloseHandle(hThread);
	CloseHandle(hProcess);
	CloseHandle(hSnapshot);
	return TRUE;




}