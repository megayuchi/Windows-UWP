//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace FileOpenUWP;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

using namespace concurrency;
using namespace Windows::Storage::Pickers;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

MainPage::MainPage()
{
	InitializeComponent();
}


void FileOpenUWP::MainPage::PickButton_ClickSingle(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	FileOpenPicker^ openPicker = ref new FileOpenPicker();
	openPicker->ViewMode = PickerViewMode::Thumbnail;
	openPicker->SuggestedStartLocation = PickerLocationId::PicturesLibrary;
	openPicker->FileTypeFilter->Append(".txt");
	
	//auto ctx = task_continuation_context::use_arbitrary();
	auto ctx = task_continuation_context::use_default();
	create_task(openPicker->PickSingleFileAsync()).then([this](Windows::Storage::StorageFile^ file)
	{
		if (file)
		{
			ReadTextFromFile(file);
		}

	},ctx);
}

void MainPage::ReadTextFromFile(Windows::Storage::IStorageFile^ file)
{
	create_task(Windows::Storage::FileIO::ReadTextAsync(file)).then([this, file](Platform::String^ str)
	{
		Platform::String^ txt = str + "\n";
		OutputDebugString(txt->Data());
		m_Text->Text = txt;
	});
}

void FileOpenUWP::MainPage::PickButton_ClickMultiple(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

}


void FileOpenUWP::MainPage::PickButton_ClickFolder(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

}
