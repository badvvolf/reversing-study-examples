
#include<windows.h>


typedef struct _Thread_Param
{

	FARPROC pFunc[2];


}THREAD_PARAM, *PTHREAD_PARAM;


unsigned char g_InjectionCode[] = {

	0x55, 0x8B, 0xEC, 0x8B, 0x75, 0x08, 0x68, 0x6C, 0x6C, 0x00, 0x00, 0x68, 0x33, 0x32, 
	0x2E, 0x64, 0x68, 0x75, 0x73, 0x65, 0x72, 0x54, 0xFF, 0x16, 0x68, 0x6F, 0x78, 0x41, 
	0x00, 0x68, 0x61, 0x67, 0x65, 0x42, 0x68, 0x4D, 0x65, 0x73, 0x73, 0x54, 0x50, 0xFF, 
	0x56, 0x04, 0x6A, 0x00, 0xE8, 0x0B, 0x00, 0x00, 0x00, 0x4D, 0x79, 0x4D, 0x65, 0x73, 
	0x73, 0x61, 0x67, 0x65, 0x00, 0x00, 0xE8, 0x1E, 0x00, 0x00, 0x00, 0x49, 0x20, 0x57,
	0x61, 0x6E, 0x74, 0x20, 0x74, 0x6F, 0x20, 0x70, 0x72, 0x69, 0x6E, 0x74, 0x20, 0x74, 
	0x68, 0x69, 0x73, 0x20, 0x6D, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x00, 0x00, 0x6A,
	0x00, 0xFF, 0xD0, 0x33, 0xC0, 0x8B, 0xE5, 0x5D, 0xC3

};


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


BOOL InjectCode(int dwPID)
{

	HMODULE hMod = NULL;

	//������ ���ڷ� ���� ����ü
	THREAD_PARAM param = { 0, };

	HANDLE hProcess = NULL;
	HANDLE hThread = NULL;

	//��� ���μ����� ������ ������ ���� �������� �ּ�
	LPVOID pRemoteBuf[2] = { 0, };

	DWORD dwSize = 0;


	hMod = GetModuleHandleA("kernel32.dll");

	param.pFunc[0] = GetProcAddress(hMod, "LoadLibraryA");
	param.pFunc[1] = GetProcAddress(hMod, "GetProcAddress");


	//��� ���μ����� ����. 
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE , dwPID);

	//��� ���μ����� �� ������ ũ�⸦ ����
	dwSize = sizeof(THREAD_PARAM);

	//��� ���μ����� �޸� ������ Ȯ��
	pRemoteBuf[0] = VirtualAllocEx(hProcess, NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);

	//��� ���μ����� �����ǵ� �ڵ尡 �̿��� �Լ� ���ڸ� ����
	WriteProcessMemory(hProcess, pRemoteBuf[0], (LPVOID)&param, dwSize, NULL);


	//�������� ���� �ڵ��� ũ�⸦ ����
	dwSize = sizeof(g_InjectionCode);
	//��� ���μ����� �޸� ������ Ȯ��
	pRemoteBuf[1] = VirtualAllocEx(hProcess, NULL, dwSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	//��� ���μ����� ������ ��
	WriteProcessMemory(hProcess, pRemoteBuf[1], (LPVOID)g_InjectionCode, dwSize, NULL);

	//������!
	hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pRemoteBuf[1], pRemoteBuf[0], 0, NULL);
	
	WaitForSingleObject(hThread, INFINITE);

	CloseHandle(hThread);
	CloseHandle(hProcess);

	return TRUE;

} //BOOL InjectCode(int dwPID)