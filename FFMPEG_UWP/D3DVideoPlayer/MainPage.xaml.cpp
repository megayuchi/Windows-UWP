//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "BlankPage.xaml.h"
#include <shcore.h>
#include <ppltasks.h>
#include "VideoPlayer.h"
#include "D3DRenderer.h"

#include "../Common_UTIL/debug_new.h"

using namespace D3DVideoPlayer;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;



using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Devices::Input;
using namespace concurrency;

using namespace Windows::Storage::Pickers;
using namespace Windows::UI::Popups;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409


BOOL CreateVideoPlayer(Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanel)
{
#ifdef _DEBUG
	BeginHeapCheck();
#endif
	g_pVideoPlayer = new CVideoPlayer;
	g_pVideoPlayer->Initialize(swapChainPanel);
	return TRUE;
}

void CleanupVideoPlayer()
{

	if (g_pVideoPlayer)
	{
		delete g_pVideoPlayer;
		g_pVideoPlayer = NULL;
	}
#ifdef _DEBUG
	EndHeapCheck();
#endif

}
MainPage::MainPage()
{
	InitializeComponent();


	CoreWindow^ window = Window::Current->CoreWindow;

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &MainPage::OnVisibilityChanged);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &MainPage::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &MainPage::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &MainPage::OnDisplayContentsInvalidated);

	swapChainPanel->CompositionScaleChanged += 
		ref new TypedEventHandler<SwapChainPanel^, Object^>(this, &MainPage::OnCompositionScaleChanged);

	swapChainPanel->SizeChanged +=
		ref new SizeChangedEventHandler(this, &MainPage::OnSwapChainPanelSizeChanged);

	EventHandler<Object^>^ ev = ref new EventHandler<Object^>(this, &MainPage::OnUpdate);
	m_onRenderingEventToken = CompositionTarget::Rendering::add(ev);

	CreateVideoPlayer(swapChainPanel);

	ShaderList->Items->Append(L"Normal");	// 0
	ShaderList->Items->Append(L"Blur");		// 1
	ShaderList->Items->Append(L"Edge");		// 2
	ShaderList->Items->Append(L"Grey");		// 3

}



void MainPage::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
	if (m_windowVisible)
	{
		//m_main->StartRenderLoop();
	}
	else
	{
		//m_main->StopRenderLoop();
	}
}

// DisplayInformation event handlers.
void MainPage::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	int a = 0;
}
void MainPage::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	int a = 0;
}

void MainPage::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	int a = 0;
}

void MainPage::OnCompositionScaleChanged(SwapChainPanel^ sender, Object^ args)
{
	Windows::Foundation::Size logicalSize = Windows::Foundation::Size(static_cast<float>(swapChainPanel->ActualWidth), static_cast<float>(swapChainPanel->ActualHeight));

	m_fCompositionScaleX = swapChainPanel->CompositionScaleX;
	m_fCompositionScaleY = swapChainPanel->CompositionScaleY;

	//m_fCompositionScaleX = 1.0f;
	//m_fCompositionScaleY = 1.0f;

	g_pVideoPlayer->UpdateForWindowSizeChange(&logicalSize,m_fCompositionScaleX,m_fCompositionScaleY);

}


void MainPage::OnSwapChainPanelSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
	Windows::Foundation::Size logicalSize = Windows::Foundation::Size(static_cast<float>(swapChainPanel->ActualWidth), static_cast<float>(swapChainPanel->ActualHeight));
	
	m_fCompositionScaleX = swapChainPanel->CompositionScaleX;
	m_fCompositionScaleY = swapChainPanel->CompositionScaleY;
	
	//m_fCompositionScaleX = 1.0f;
	//m_fCompositionScaleY = 1.0f;

	g_pVideoPlayer->UpdateForWindowSizeChange(&logicalSize,m_fCompositionScaleX,m_fCompositionScaleY);

}

Windows::UI::Xaml::Controls::SwapChainPanel^ MainPage::GetSwapChainPanel()
{
	return swapChainPanel;
}
void MainPage::OnUpdate(
    _In_ Object^ /* sender */,
    _In_ Object^ /* args */
    )
{
	if (g_pVideoPlayer)
	{
		g_pVideoPlayer->Process();
	}
}
void MainPage::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e)
{
	(void) e;	// Unused parameter

	// TODO: Prepare page for display here.

	// TODO: If your application contains multiple pages, ensure that you are
	// handling the hardware Back button by registering for the
	// Windows::Phone::UI::Input::HardwareButtons.BackPressed event.
	// If you are using the NavigationHelper provided by some templates,
	// this event is handled for you.
	
	
}

void MainPage::OnNavigatedFrom(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e)
{
	// Called when a page is no longer the active page in a frame.
	int a = 0;
	//Cleanup();
	

}
void MainPage::LoadButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	FileOpenPicker^ openPicker = ref new FileOpenPicker();
    openPicker->ViewMode = PickerViewMode::Thumbnail;
    openPicker->SuggestedStartLocation = PickerLocationId::ComputerFolder;
	openPicker->FileTypeFilter->Append(".mp4");
	openPicker->FileTypeFilter->Append(".avi");
	openPicker->FileTypeFilter->Append(".mkv");

	auto ui = task_continuation_context::use_current();

    create_task(openPicker->PickSingleFileAsync()).then([this,ui](Windows::Storage::StorageFile^ file)
    {
		if (file)
		{
			
			create_task(file->OpenAsync(Windows::Storage::FileAccessMode::Read)).then([this,ui](Windows::Storage::Streams::IRandomAccessStream^ stream)
			{
				IStream* fileStreamData = nullptr;
				HRESULT hr = CreateStreamOverRandomAccessStream(reinterpret_cast<IUnknown*>(stream), IID_PPV_ARGS(&fileStreamData));
				if (S_OK != hr)
					__debugbreak();

				

				g_pVideoPlayer->Play(fileStreamData,mediaElement);
				//m_pDecoder->DecodeAsync(fileStreamData);
			},ui);
		}
		else
		{
			
		}

    });
}
void MainPage::ExitButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

	// AppBar가 닫히지 않아서 swapchain 카운터가 1 남아있을 수 있다
	// 따라서 무조건 앱바를 닫는다.
	bottomAppBar->IsOpen = false;

	auto dispatcher = Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher;
	auto frame = this->Frame;
	
	dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,ref new Windows::UI::Core::DispatchedHandler([frame]()
	{
		Sleep(100);
		frame->Navigate(TypeName(BlankPage::typeid), nullptr);
	}));

}

void MainPage::Grid_DragEnter(Platform::Object^ sender, Windows::UI::Xaml::DragEventArgs^ e)
{
	e->AcceptedOperation = Windows::ApplicationModel::DataTransfer::DataPackageOperation::Link;
	auto t = e->Handled;
	e->Handled = true;

}

void MainPage::Grid_Drop(Platform::Object^ sender, Windows::UI::Xaml::DragEventArgs^ e)
{
	Windows::UI::Xaml::DragOperationDeferral^ d = e->GetDeferral();

	auto ui = task_continuation_context::use_current();

	create_task(e->DataView->GetStorageItemsAsync()).then([this,e,d,ui](IVectorView<Windows::Storage::IStorageItem^>^ fileList)
	{
		for each (Windows::Storage::IStorageItem^ item in fileList)
		{	
			String^ FullPath = item->Path;
			const WCHAR*	wchFullPath = FullPath->Data();
			
			
			Platform::String^ Msg = FullPath + L"\n";
		

			if (item->IsOfType(Windows::Storage::StorageItemTypes::File))
			{
				Windows::Storage::StorageFile^ file = safe_cast<Windows::Storage::StorageFile^>(item);

				create_task(file->OpenAsync(Windows::Storage::FileAccessMode::Read)).then([this,ui](Windows::Storage::Streams::IRandomAccessStream^ stream)
				{
					IStream* fileStreamData = nullptr;
					HRESULT hr = CreateStreamOverRandomAccessStream(reinterpret_cast<IUnknown*>(stream), IID_PPV_ARGS(&fileStreamData));
					if (S_OK != hr)
						__debugbreak();

					g_pVideoPlayer->Play(fileStreamData,mediaElement);
					//m_pDecoder->DecodeAsync(fileStreamData);
				},ui);
				
			}
		}
		d->Complete();
	
	},ui);	

	
}
MainPage::~MainPage()
{
	//g_pVideoPlayer->CleanupSwapChainNaviePanel();
	swapChainPanel = nullptr;
	
	auto dispatcher = Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher;

	dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,ref new Windows::UI::Core::DispatchedHandler([]()
	{

		CleanupVideoPlayer();

	}));
	
}
void MainPage::MediaFailed(Platform::Object^ sender, Windows::UI::Xaml::ExceptionRoutedEventArgs^ e)
{
	DisplayErrorMessage(e->ErrorMessage);
}

void MainPage::DisplayErrorMessage(Platform::String^ message)
{
	// Display error message
	auto errorDialog = ref new MessageDialog(message);
	errorDialog->ShowAsync();
}


void D3DVideoPlayer::MainPage::swapChainPanel_Tapped(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e)
{
	if (PopupSelectShader->IsOpen) 
	{
		PopupSelectShader->IsOpen = false; 
	}
	else
	{
		PopupSelectShader->IsOpen = true;
	}
	

}


void D3DVideoPlayer::MainPage::ShaderList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	
	PopupSelectShader->IsOpen = false;

	int index = ShaderList->SelectedIndex;
	if (index < 0)
		return;

	g_pVideoPlayer->SetPixelShaderFilter((PIXEL_SHADER_FILTER)index);
}
