

#include <windows.h>
#include <stdio.h>
#include <wchar.h>

typedef BOOL(WINAPI * PFSETWINDOWTEXTW)(HWND hWnd, LPWSTR lpString);
FARPROC g_pOriginalFunc = NULL;

BOOL WINAPI MySetWindowTextW(HWND hWnd, LPWSTR lpString);
BOOL Hook_IAT(LPCSTR szDllName, PROC pfnOrg, PROC pfnNew);


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:

		g_pOriginalFunc = GetProcAddress(GetModuleHandleA("user32.dll"), "SetWindowTextW");

		Hook_IAT("user32.dll", g_pOriginalFunc, (PROC)MySetWindowTextW);

		break;

	case DLL_PROCESS_DETACH:
		Hook_IAT("user32.dll", (PROC)MySetWindowTextW, g_pOriginalFunc);
		break;

	}

	return TRUE;

}


BOOL WINAPI MySetWindowTextW(HWND hWnd, LPWSTR lpString)
{
	wchar_t * pNum = L"øµ¿œ¿ÃªÔªÁø¿¿∞ƒ•∆»±∏";
	wchar_t temp[2] = { 0, };
	int nLen = 0, nIndex = 0;

	nLen = wcslen(lpString);
	for (int i = 0; i < nLen; i++)
	{
		if (L'0' <= lpString[i] && lpString[i] <= L'9')
		{
			temp[0] = lpString[i];
			nIndex = _wtoi(temp);

			lpString[i] = pNum[nIndex];
		}

	}

	return ((PFSETWINDOWTEXTW)g_pOriginalFunc)(hWnd, lpString);

}


BOOL Hook_IAT(LPCSTR szDllName, PROC pfnOrg, PROC pfnNew)
{
	HMODULE hMod;
	LPCSTR szLibName;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
	PIMAGE_THUNK_DATA pThunk;
	DWORD dwOldProtect, dwRVA;
	PBYTE pAddr;


	hMod = GetModuleHandle(NULL);
	pAddr = (PBYTE)hMod;

	pAddr += *((DWORD *)&pAddr[0x3C]);
	dwRVA = *((DWORD *)&pAddr[0x80]);

	pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)hMod + dwRVA);

	for (; pImportDesc->Name; pImportDesc++)
	{
		szLibName = (LPCSTR)((DWORD)hMod + pImportDesc->Name);

		if (!+ stricmp(szLibName, szDllName))
		{

			pThunk = (PIMAGE_THUNK_DATA)((DWORD)hMod + pImportDesc->FirstThunk);

			for (; pThunk->u1.Function; pThunk++)
			{

				if (pThunk->u1.Function == (DWORD)pfnOrg)
				{
					VirtualProtect((LPVOID)&pThunk->u1.Function, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect);

					pThunk->u1.Function = (DWORD)pfnNew;

					VirtualProtect((LPVOID)&pThunk->u1.Function, 4, dwOldProtect, &dwOldProtect);

					return TRUE;

				} //if (pThunk->u1.Function == (DWORD)pfnOrg)

			} //for (; pThunk->u1.Function; pThunk++)

		} //if (!+ stricmp(szLibName, szDllName))


	} //for (; pImportDesc->Name; pImportDesc++)

	return FALSE;

}





