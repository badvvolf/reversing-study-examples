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


	//DLL �ε�
	hDll = LoadLibraryA(argv[1]);
	
	//�Լ� �ּ� ���	
	HookStart = (PFN_HOOKSTART)GetProcAddress(hDll, "HookStart");
	HookStop = (PFN_HOOKSTART)GetProcAddress(hDll, "HookStop");

	//��ŷ ����
	HookStart(); 


	//q�� ���� ������ ���
	printf("press 'q' to quit\n");
	while ( getch() != 'q');


	//��ŷ ����
	HookStop();
	FreeLibrary(hDll);



}



