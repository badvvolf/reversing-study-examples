#include<windows.h>
#include<stdio.h>


void WriteFile()
{
	FILE * fp = fopen("HI.txt", "wt");
	fwrite("hi", 3, 1, fp);
	fclose(fp);

}


BOOL WINAPI DllMain(HINSTANCE hinstance, DWORD fdwReason, LPVOID lpvReserved)
{
	char szPath[MAX_PATH] = { 0, }; 
	char * szProcess = NULL; 

	switch (fdwReason)
	{	
	case DLL_PROCESS_ATTACH:

		//인젝션 된 프로세스의 이름(경로)를 얻어 옴
		if (!GetModuleFileNameA(NULL, szPath, MAX_PATH))
			break;
	
		//맨 뒤의 \를 찾음(EXE 이름을 알아내기 위해)
		if (!(szProcess = strrchr(szPath, '\\')))
			break;

		//포인터를 1 증가 -> \ 뒤의 EXE 이름을 가리키게 됨
		szProcess++; 

		//실행한 프로그램이 인젝션하고자 하는 대상 프로그램이 아닐 시 
		//아무것도 안 함
		//목적 프로그램은 임의로 PEiD.exe로 설정했음
		if (stricmp(szProcess, "PEiD.exe"))
			 break;

		WriteFile();
	}

	return TRUE;

}


