
#pragma region Includes
// Все WINAPI и прочие функции
#include <Windows.h>
#include <intrin.h>
#include <vector>
#include <tchar.h>
#include <fstream> 
#include <iostream>
#pragma intrinsic(_ReturnAddress)
// Перехват функций
#include <MinHook.h>
#include <string>
using namespace std;
#pragma comment(lib,"libMinHook.x86.lib")
#pragma endregion


BOOL __stdcall IsEnemy( int UnitAddr );