#pragma once

#ifdef _DEBUG
	#define _CRTDBG_MAP_ALLOC
#endif

#include "targetver.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#include <wrl.h>
#include <wrl/client.h>
#include <memory>
#include <agile.h>
#include <concrt.h>
#include <collection.h>
#include <ppltasks.h>

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <Windows.h>

#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <iphlpapi.h>

#include <Ole2.h>
#include <initguid.h>

#include "Decoder.h"
#include "DecoderThread.h"
#include "../Common/IDecoder.h"
#include "../Common/decoder_typedef.h"

