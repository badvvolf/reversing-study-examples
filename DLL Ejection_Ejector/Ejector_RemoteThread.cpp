#include<stdio.h>
#include<windows.h>
#include<tlhelp32.h>
#include<tchar.h>
#include<stdlib.h>


BOOL EjectDll(DWORD dwPID, LPCTSTR szDllName);

int _tmain(int argc, TCHAR * argv[])
{

	DWORD dwPID;

	if (argc != 3)
		printf("usage %s : pid DLLName", argv[0]);

	//입력받은 프로세스 ID를 숫자로 변환
	dwPID = _tstol(argv[1]);

	EjectDll(dwPID, (LPCTSTR)argv[2]);

	return 0;

}



BOOL EjectDll(DWORD dwPID, LPCTSTR szDllName)
{
	HANDLE hSnapshot, hProcess, hThread;
	MODULEENTRY32 me = { sizeof(me) };
	BOOL bMore = FALSE, bFound = FALSE;
	LPTHREAD_START_ROUTINE pThreadProc;

	//대상 프로세스의 정보를 얻음
	//대상 프로세스에 로딩된 DLL 이름을 얻을 수 있다
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);


	//대상 프로세스에 우리가 없애고자 하는 DLL이 있는지 살펴봄
	//bMore가 true면 (Module32First 함수가 true를 리턴하면) 아직 살펴보지 않은 DLL이 있다는 의미
	bMore = Module32First(hSnapshot, &me);
	for (; bMore; bMore = Module32Next(hSnapshot, &me))
	{

		//찾고자 하는 DLL이라면 변수 설정 후 루프 탈출
		if (!_tcsicmp((LPCTSTR)me.szModule, szDllName) || !_tcsicmp((LPCTSTR)me.szExePath, szDllName))
		{
			bFound = TRUE;
			break;
		}
	}


	//DLL을 전부 찾아봤지만 찾는 DLL이 없는 경우
	//함수 끝냄
	if (!bFound)
	{
		OutputDebugStringA("No Dll here");
		CloseHandle(hSnapshot);
		return FALSE;
	}


	//대상 프로세스 열기
	if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID)))
	{
		OutputDebugStringA("error OpenProcess");
		return FALSE;
	}

	//FreeLibrary 함수 주소 구함
	pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "FreeLibrary");


	//원격 쓰레드 생성 방식으로 DLL 내림
	//me.hModule는 대상 프로세스에서의 내리고자 하는 DLL 핸들
	//me.modBaseAddr로 해도 됨. DLL이 올라온 주소나 DLL핸들이나 같은 값..
	hThread = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, me.hModule, 0, NULL);


	WaitForSingleObject(hThread, INFINITE);

	CloseHandle(hThread);
	CloseHandle(hProcess);
	CloseHandle(hSnapshot);
	return TRUE;
}



