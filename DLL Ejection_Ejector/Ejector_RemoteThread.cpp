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

	//�Է¹��� ���μ��� ID�� ���ڷ� ��ȯ
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

	//��� ���μ����� ������ ����
	//��� ���μ����� �ε��� DLL �̸��� ���� �� �ִ�
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);


	//��� ���μ����� �츮�� ���ְ��� �ϴ� DLL�� �ִ��� ���캽
	//bMore�� true�� (Module32First �Լ��� true�� �����ϸ�) ���� ���캸�� ���� DLL�� �ִٴ� �ǹ�
	bMore = Module32First(hSnapshot, &me);
	for (; bMore; bMore = Module32Next(hSnapshot, &me))
	{

		//ã���� �ϴ� DLL�̶�� ���� ���� �� ���� Ż��
		if (!_tcsicmp((LPCTSTR)me.szModule, szDllName) || !_tcsicmp((LPCTSTR)me.szExePath, szDllName))
		{
			bFound = TRUE;
			break;
		}
	}


	//DLL�� ���� ã�ƺ����� ã�� DLL�� ���� ���
	//�Լ� ����
	if (!bFound)
	{
		OutputDebugStringA("No Dll here");
		CloseHandle(hSnapshot);
		return FALSE;
	}


	//��� ���μ��� ����
	if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID)))
	{
		OutputDebugStringA("error OpenProcess");
		return FALSE;
	}

	//FreeLibrary �Լ� �ּ� ����
	pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "FreeLibrary");


	//���� ������ ���� ������� DLL ����
	//me.hModule�� ��� ���μ��������� �������� �ϴ� DLL �ڵ�
	//me.modBaseAddr�� �ص� ��. DLL�� �ö�� �ּҳ� DLL�ڵ��̳� ���� ��..
	hThread = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, me.hModule, 0, NULL);


	WaitForSingleObject(hThread, INFINITE);

	CloseHandle(hThread);
	CloseHandle(hProcess);
	CloseHandle(hSnapshot);
	return TRUE;
}



