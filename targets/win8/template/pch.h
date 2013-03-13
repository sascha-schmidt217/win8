
#pragma once

#include <wrl/client.h>
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <agile.h>
#include <ppltasks.h>
#include <Xaudio2.h>

#include <initguid.h>	//Must come BEFORE the following mf #includes for some reason!
#include <mfmediaengine.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#include <wincodec.h>
#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/) && !defined(DXGI_1_2_FORMATS)
#define DXGI_1_2_FORMATS
#endif

#include <cmath>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <memory>
#include <vector>
#include <typeinfo>

#include <signal.h>

#define WINDOWS_8 1



/* 
--------------------------------------
modules.win8.networking.sockets
--------------------------------------
*/

#define _HIDE_GLOBAL_ASYNC_STATUS 100 // hides static const AsyncStatus Error = AsyncStatus::Error;
//..maybe monkey's 'Error' should be called bberror

#include <windows.networking.sockets.h>
#include <wrl.h>
#include <robuffer.h>
#include <vector>
/* 
--------------------------------------
*/
