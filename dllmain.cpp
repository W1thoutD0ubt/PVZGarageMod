#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOMINMAX
#include <Windows.h>

import Index;
import BoardEvents;
import GarageResources;
import Drawing;
import ZombieEvents;

extern "C"
{
	__declspec(dllexport) void init()
	{
		PVZ::InitPVZDLL();
		PVZ::GetPVZApp().LoadProperties(PVZ::PVZString::Make("garage/settings.xml"));
		//PVZ::Memory::WriteMemory<int>(0x651200, 666);
		if (PVZ::GetPVZApp().GetBoolean(PVZ::PVZString::Make("Enabled"), true))
		{
			InitResource();
			InitBoardEvents();
			InitDrawing();
			InitZombieEvents();
		}
	}
}

bool APIENTRY DllMain( HMODULE hModule,
					   DWORD  ul_reason_for_call,
					   LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		PVZ::Plant::MemSize = 0x16C;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return true;
}

