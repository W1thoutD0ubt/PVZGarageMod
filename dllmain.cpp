#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOMINMAX
#include <Windows.h>

import Index;
import BoardEvents;
import GarageResources;
import Drawing;

bool APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		PVZ::InitPVZDLL();
		PVZ::GetPVZApp().LoadProperties(PVZ::PVZString::Make("garage/settings.xml"));
		if (PVZ::GetPVZApp().GetBoolean(PVZ::PVZString::Make("Enabled"), false))
		{
			InitResource();
			InitBoardEvents();
			InitDrawing();
		}
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return true;
}

