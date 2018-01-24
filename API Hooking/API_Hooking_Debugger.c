#include <Windows.h>
#include <stdio.h>


void DebugLoop();
BOOL OnExceptionDebugEvent(LPDEBUG_EVENT pde);
BOOL OnCreateProcessDebugEvent(LPDEBUG_EVENT pde);


LPVOID g_pfWriteFile = NULL;
CREATE_PROCESS_DEBUG_INFO g_cpdi;
BYTE g_chINT3 = 0xCC;
BYTE g_chOriginalByte = 0;

int main(int argc, char * argv[])
{
	DWORD dwPID;

	if (argc != 2)
	{
		printf("usage %s PID", argv[0]);
		return 1;

	}

	dwPID = atoi(argv[1]);

	if (!DebugActiveProcess(dwPID))
	{
		printf("DebugActiveProcess(%d) failed!!\n" "Error code = %d\n", dwPID, GetLastError());
		return 1;
	}

	DebugLoop();

}



void DebugLoop()
{
	DEBUG_EVENT de;
	DWORD dwContinueStatus;

	while (WaitForDebugEvent(&de, INFINITE))
	{
		dwContinueStatus = DBG_CONTINUE;

		if (de.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT)
		{
			OnCreateProcessDebugEvent(&de);
		}
		

		else if (de.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
		{
			if (OnExceptionDebugEvent(&de))
				continue;
		}

		else if (de.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT)
		{
			break; 
		}

		ContinueDebugEvent(de.dwProcessId, de.dwThreadId, dwContinueStatus);

	}
}


BOOL OnCreateProcessDebugEvent(LPDEBUG_EVENT pde)
{
	g_pfWriteFile = GetProcAddress(GetModuleHandleA("kernel32.dll"), "WriteFile");

	memcpy(&g_cpdi, &pde->u.CreateProcessInfo, sizeof(CREATE_PROCESS_DEBUG_INFO));

	ReadProcessMemory(g_cpdi.hProcess, g_pfWriteFile, &g_chOriginalByte, sizeof(BYTE), NULL);

	WriteProcessMemory(g_cpdi.hProcess, g_pfWriteFile, &g_chINT3, sizeof(BYTE), NULL);
	
	return TRUE;

}


BOOL OnExceptionDebugEvent(LPDEBUG_EVENT pde)
{
	CONTEXT ctx;
	PBYTE lpBuffer = NULL;
	DWORD dwNumOfByteToWrite, dwAddrOfBuffer;
	PEXCEPTION_RECORD per = &pde->u.Exception.ExceptionRecord;

	if (per->ExceptionCode == EXCEPTION_BREAKPOINT)
	{
		if (per->ExceptionAddress == g_pfWriteFile)
		{

			WriteProcessMemory(g_cpdi.hProcess, g_pfWriteFile, &g_chOriginalByte, sizeof(BYTE), NULL);

			ctx.ContextFlags = CONTEXT_CONTROL;
			GetThreadContext(g_cpdi.hThread, &ctx);

		}


		ReadProcessMemory(g_cpdi.hProcess, (LPVOID)(ctx.Esp + 0x8), &dwAddrOfBuffer, sizeof(DWORD), NULL);
		ReadProcessMemory(g_cpdi.hProcess, (LPVOID)(ctx.Esp + 0xC), &dwNumOfByteToWrite, sizeof(DWORD), NULL);

		lpBuffer = (PBYTE)malloc(dwNumOfByteToWrite + 1);
		memset(lpBuffer, 0, dwNumOfByteToWrite + 1);

		ReadProcessMemory(g_cpdi.hProcess, (LPVOID)dwAddrOfBuffer, lpBuffer, dwNumOfByteToWrite, NULL);

		printf("\n## Original string : %s\n", lpBuffer);

		for (int i = 0; i < dwNumOfByteToWrite; i++)
		{
			if (0x61 <= lpBuffer[i] && lpBuffer[i] <= 0x7A)
				lpBuffer[i] -= 0x20;

		}

		printf("\n## modified string : %s\n", lpBuffer);


		WriteProcessMemory(g_cpdi.hProcess, (LPVOID)dwAddrOfBuffer, lpBuffer, dwNumOfByteToWrite, NULL);

		free(lpBuffer);

		ctx.Eip = (DWORD)g_pfWriteFile;
		SetThreadContext(g_cpdi.hThread, &ctx);

		ContinueDebugEvent(pde->dwProcessId, pde->dwThreadId, DBG_CONTINUE);


		Sleep(0);

		WriteProcessMemory(g_cpdi.hProcess, g_pfWriteFile, &g_chINT3, sizeof(BYTE), NULL);

		return TRUE;


	}


	return FALSE;


}



