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

		//������ �� ���μ����� �̸�(���)�� ��� ��
		if (!GetModuleFileNameA(NULL, szPath, MAX_PATH))
			break;
	
		//�� ���� \�� ã��(EXE �̸��� �˾Ƴ��� ����)
		if (!(szProcess = strrchr(szPath, '\\')))
			break;

		//�����͸� 1 ���� -> \ ���� EXE �̸��� ����Ű�� ��
		szProcess++; 

		//������ ���α׷��� �������ϰ��� �ϴ� ��� ���α׷��� �ƴ� �� 
		//�ƹ��͵� �� ��
		//���� ���α׷��� ���Ƿ� PEiD.exe�� ��������
		if (stricmp(szProcess, "PEiD.exe"))
			 break;

		WriteFile();
	}

	return TRUE;

}


