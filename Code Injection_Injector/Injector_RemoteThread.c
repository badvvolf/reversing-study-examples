
#include<windows.h>

//쓰레드 인자로 넣을 구조체
typedef struct _Thread_Param
{
	//쓰레드에서 사용할 함수들의 포인터를 저장
	FARPROC pFunc[2];

	//쓰레드에서 사용할 문자열들을 저장
	char szBuf[4][128];
		
}THREAD_PARAM, *PTHREAD_PARAM;





//---함수 포인터 재정의---

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


//___함수 포인터 재정의___




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


	//코드 인젝션
	dwPID = (DWORD)atol(argv[1]);
	InjectCode(dwPID);

	return 0;

}



//대상 프로그램에 인젝션할 코드
DWORD WINAPI ThreadProc(LPVOID lParam)
{

	//쓰레드 생성시 인자로 받은 포인터를 캐스킹
	PTHREAD_PARAM pParam = (PTHREAD_PARAM)lParam;

	HMODULE hMod = NULL;
	FARPROC pFunc = NULL;

	//대상 프로세스에서의 MessageBoxA 함수 주소를 알아내기 위해 LoadLibraryA 호출
	//MessageBoxA가 있는 user32.dll을 로드
	hMod = ((PFLOADLIBRARYA)pParam->pFunc[0])(pParam->szBuf[0]);


	//얻어온 user32.dll에서 MessageBoxA함수의 주소를 얻음
	pFunc = (FARPROC)((PFGETPROCADDRESS)pParam->pFunc[1])(hMod, pParam->szBuf[1]);


	((PFMESSAGEBOXA)pFunc)(NULL, pParam->szBuf[2], pParam->szBuf[3], MB_OK);

	return 0;

}


BOOL InjectCode(int dwPID)
{
	HMODULE hMod = NULL;

	//쓰레드에 인자로 넣을 구조체
	THREAD_PARAM param = { 0, };


	HANDLE hProcess = NULL;
	HANDLE hThread = NULL; 

	//대상 프로세스의 데이터 영역에 넣은 데이터의 주소
	LPVOID pRemoteBuf[2] = { 0, };
	DWORD dwSize = 0;


	hMod = GetModuleHandleA("kernel32.dll");


	//인젝션 후 사용할 함수 주소 구해 저장
	param.pFunc[0] = GetProcAddress(hMod, "LoadLibraryA");
	param.pFunc[1] = GetProcAddress(hMod, "GetProcAddress");

	//인젝션 후 사용할 함수 인자를 저장
	strcpy(param.szBuf[0], "user32.dll");
	strcpy(param.szBuf[1], "MessageBoxA");

	//인젝션 후 실행할 메시지 박스 함수에 줄 인자를 저장
	strcpy(param.szBuf[2], "Hello!");
	strcpy(param.szBuf[3], "Hi");




	//대상 프로세스를 연다.
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);

	//대상 프로세스에 쓸 내용의 크기를 구한다. 
	dwSize = sizeof(THREAD_PARAM);
	//대상 프로세스의 메모리에 공간을 확보한다. 
	pRemoteBuf[0] = VirtualAllocEx(hProcess, NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
	//대상 프로세스에 내용을 쓴다.
	WriteProcessMemory(hProcess, pRemoteBuf[0], (LPVOID)&param, dwSize, NULL);


	//주소 계산을 위해 함수의 크기를 구한다. 
	//대상 프로세스의 메모리에 쓸 코드의 크기를 구함
	dwSize = (DWORD)InjectCode - (DWORD)ThreadProc;

	//대상 프로세스의 메모리 공간을 확보
	pRemoteBuf[1] = VirtualAllocEx(hProcess, NULL, dwSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	//대상 프로세스에 내용을 쓴다
	WriteProcessMemory(hProcess, pRemoteBuf[1], (LPVOID)ThreadProc, dwSize, NULL);


	//인젝션!
	hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pRemoteBuf[1], pRemoteBuf[0], 0, NULL);

	WaitForSingleObject(hThread, INFINITE);

	CloseHandle(hThread);
	CloseHandle(hProcess);

	return TRUE;
		
} //BOOL InjectCode(int dwPID)

