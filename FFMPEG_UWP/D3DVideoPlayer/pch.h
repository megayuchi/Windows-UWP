//
// pch.h
// Header for standard system include files.
//

#pragma once

#ifdef _DEBUG
	#define _CRTDBG_MAP_ALLOC
#endif

#include <wrl.h>
#include <wrl/client.h>
#include <d3d11_2.h>
#include <d2d1_2.h>
#include <d2d1effects_1.h>
#include <dwrite_2.h>
#include <wincodec.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <windows.ui.xaml.media.dxinterop.h>
#include <memory>
#include <agile.h>
#include <concrt.h>
#include <collection.h>

#include "App.xaml.h"
#include "Util.h"
#include "../Common/IDecoder.h"
#include "d3d_typede.h"
#include "D3DHelper.h"
#include "D3DRenderer.h"

