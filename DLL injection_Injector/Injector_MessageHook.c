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
	}

	hDll = LoadLibraryA(argv[1]);
	
	HookStart = (PFN_HOOKSTART)GetProcAddress(hDll, "HookStart");
	HookStop = (PFN_HOOKSTART)GetProcAddress(hDll, "HookStop");

	HookStart(); 

	printf("press 'q' to quit\n");
	while ( getch() != 'q');

	HookStop();
	FreeLibrary(hDll);



}
//C로 할거냐 cpp로 할거냐