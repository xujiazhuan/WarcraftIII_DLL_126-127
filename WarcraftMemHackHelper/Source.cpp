#include <Windows.h>
#include <vector>


struct offsetdata
{
	int offaddr;
	int offdata;
};

std::vector<offsetdata> offsetslist;
int __stdcall AddNewOffset(int address, int data)
{
	for (unsigned int i = 0; i < offsetslist.size(); i++)
	{
		if (offsetslist[i].offaddr == address)
		{
			return 0;
		}
	}

	offsetdata temp;
	temp.offaddr = address;
	temp.offdata = data;
	offsetslist.push_back(temp);

	return 1;
}

int SecondToGameEnd = 0;
int __stdcall UpdateGameTimer(int EndGameAfterNsec)
{
	SecondToGameEnd = EndGameAfterNsec;
	return 1;
}


unsigned int __stdcall InitDotaHelper(int nothing)
{
	return 1 + nothing;
}

void RestoreAllOffsets()
{

	for (offsetdata oData : offsetslist)
	{
		DWORD oldprotect, oldprotect2;
		if (VirtualProtect((void*)oData.offaddr, 4, PAGE_EXECUTE_READWRITE, &oldprotect))
		{
			*(int*)oData.offaddr = oData.offdata;
			VirtualProtect((void*)oData.offaddr, 4, oldprotect, &oldprotect2);
		}
	}
	offsetslist.clear();

}

HANDLE GameWatcherID;
DWORD WINAPI GameWatcher(LPVOID)
{
	while (true)
	{
		while (SecondToGameEnd > 0)
		{
			SecondToGameEnd -= 1;
			Sleep(1000);
		}
		RestoreAllOffsets();

		while (SecondToGameEnd >= 0)
		{
			Sleep(1000);
		}
	}

	return 0;
}

BOOL __stdcall DllMain(HINSTANCE Module, UINT reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		GameWatcherID = CreateThread(0, 0, GameWatcher, 0, 0, 0);
	}
	else if (reason == DLL_PROCESS_DETACH)
	{
		TerminateThread(GameWatcherID, 0);
		RestoreAllOffsets();
	}
	return TRUE;
}








































#pragma region Developer
//Karaulov 
#pragma endregion