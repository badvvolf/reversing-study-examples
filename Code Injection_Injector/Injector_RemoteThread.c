
#include<windows.h>

//������ ���ڷ� ���� ����ü
typedef struct _Thread_Param
{
	//�����忡�� ����� �Լ����� �����͸� ����
	FARPROC pFunc[2];

	//�����忡�� ����� ���ڿ����� ����
	char szBuf[4][128];
		
}THREAD_PARAM, *PTHREAD_PARAM;





//---�Լ� ������ ������---

//LoadLibraryA()
typedef HMODULE(WINAPI * PFLOADLIBRARYA)
(
	LPCSTR lpLibFileName

);



//GetProcAddress()
typedef FARPROC(WINAPI * PFGETPROCADDRESS)
(
	HMODULE hModule, LPCSTR lpProcName

);


//MessageBoxA()
typedef int(WINAPI * PFMESSAGEBOXA)
(
	HWND hwnd,
	LPCSTR lpText,
	LPCSTR lpCaption,
	UINT uType
);


//___�Լ� ������ ������___




DWORD WINAPI ThreadProc(LPVOID);

BOOL InjectCode(int);


int main(int argc, char * argv[])
{

	DWORD dwPID = 0;

	if (argc != 2)
	{
		printf("usage %s : pid\n", argv[0]);
		return 1;
	}


	//�ڵ� ������
	dwPID = (DWORD)atol(argv[1]);
	InjectCode(dwPID);

	return 0;

}



//��� ���α׷��� �������� �ڵ�
DWORD WINAPI ThreadProc(LPVOID lParam)
{

	//������ ������ ���ڷ� ���� �����͸� ĳ��ŷ
	PTHREAD_PARAM pParam = (PTHREAD_PARAM)lParam;

	HMODULE hMod = NULL;
	FARPROC pFunc = NULL;

	//��� ���μ��������� MessageBoxA �Լ� �ּҸ� �˾Ƴ��� ���� LoadLibraryA ȣ��
	//MessageBoxA�� �ִ� user32.dll�� �ε�
	hMod = ((PFLOADLIBRARYA)pParam->pFunc[0])(pParam->szBuf[0]);


	//���� user32.dll���� MessageBoxA�Լ��� �ּҸ� ����
	pFunc = (FARPROC)((PFGETPROCADDRESS)pParam->pFunc[1])(hMod, pParam->szBuf[1]);


	((PFMESSAGEBOXA)pFunc)(NULL, pParam->szBuf[2], pParam->szBuf[3], MB_OK);

	return 0;

}


BOOL InjectCode(int dwPID)
{
	HMODULE hMod = NULL;

	//�����忡 ���ڷ� ���� ����ü
	THREAD_PARAM param = { 0, };


	HANDLE hProcess = NULL;
	HANDLE hThread = NULL; 

	//��� ���μ����� ������ ������ ���� �������� �ּ�
	LPVOID pRemoteBuf[2] = { 0, };
	DWORD dwSize = 0;


	hMod = GetModuleHandleA("kernel32.dll");


	//������ �� ����� �Լ� �ּ� ���� ����
	param.pFunc[0] = GetProcAddress(hMod, "LoadLibraryA");
	param.pFunc[1] = GetProcAddress(hMod, "GetProcAddress");

	//������ �� ����� �Լ� ���ڸ� ����
	strcpy(param.szBuf[0], "user32.dll");
	strcpy(param.szBuf[1], "MessageBoxA");

	//������ �� ������ �޽��� �ڽ� �Լ��� �� ���ڸ� ����
	strcpy(param.szBuf[2], "Hello!");
	strcpy(param.szBuf[3], "Hi");




	//��� ���μ����� ����.
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);

	//��� ���μ����� �� ������ ũ�⸦ ���Ѵ�. 
	dwSize = sizeof(THREAD_PARAM);
	//��� ���μ����� �޸𸮿� ������ Ȯ���Ѵ�. 
	pRemoteBuf[0] = VirtualAllocEx(hProcess, NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
	//��� ���μ����� ������ ����.
	WriteProcessMemory(hProcess, pRemoteBuf[0], (LPVOID)&param, dwSize, NULL);


	//�ּ� ����� ���� �Լ��� ũ�⸦ ���Ѵ�. 
	//��� ���μ����� �޸𸮿� �� �ڵ��� ũ�⸦ ����
	dwSize = (DWORD)InjectCode - (DWORD)ThreadProc;

	//��� ���μ����� �޸� ������ Ȯ��
	pRemoteBuf[1] = VirtualAllocEx(hProcess, NULL, dwSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	//��� ���μ����� ������ ����
	WriteProcessMemory(hProcess, pRemoteBuf[1], (LPVOID)ThreadProc, dwSize, NULL);


	//������!
	hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pRemoteBuf[1], pRemoteBuf[0], 0, NULL);

	WaitForSingleObject(hThread, INFINITE);

	CloseHandle(hThread);
	CloseHandle(hProcess);

	return TRUE;
		
} //BOOL InjectCode(int dwPID)

