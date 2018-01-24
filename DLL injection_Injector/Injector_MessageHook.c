#include<stdio.h>
#include<windows.h>

typedef void(*PFN_HOOKSTART)();
typedef void(*PFN_HOOKSTOP)();


int main(int argc, char * argv[])
{
	HMODULE hDll = NULL;
	PFN_HOOKSTART HookStart = NULL;
	PFN_HOOKSTOP HookStop = NULL;
	char ch = 0;

	if (argc != 2)
	{
		printf("usage %s dllpath", argv[0]);
		return 1;
	}


	//DLL 로딩
	hDll = LoadLibraryA(argv[1]);
	
	//함수 주소 얻기	
	HookStart = (PFN_HOOKSTART)GetProcAddress(hDll, "HookStart");
	HookStop = (PFN_HOOKSTART)GetProcAddress(hDll, "HookStop");

	//후킹 시작
	HookStart(); 


	//q가 눌릴 때까지 대기
	printf("press 'q' to quit\n");
	while ( getch() != 'q');


	//후킹 종료
	HookStop();
	FreeLibrary(hDll);



}



