//
// ExplorerPage.xaml.cpp
// Implementation of the ExplorerPage class
//

#include "pch.h"
#include <experimental\resumable>
#include <ppl.h>
#include <pplawait.h>
#include <ppltasks.h>

#include "ExplorerPage.xaml.h"
#include "FileItem.h"
#include "FileOpen.h"

using namespace OneDriveAccess;

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
using namespace Windows::UI::Xaml::Media::Imaging;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

void ExplorerPage::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs ^ e)
{
	//Vector<FileItem^>^ items = (Vector<FileItem^>^)e->Parameter;
	UpdateFolderStatus();
	UpdateProgress();
}

void ExplorerPage::OnNavigatedFrom(Windows::UI::Xaml::Navigation::NavigationEventArgs ^ e)
{

}
void ExplorerPage::UpdateFolderStatus()
{
	App^ app = safe_cast<App^>(App::Current);
	if (!app)
		return;

	OneDriveService^ service = app->GetOneDriveService();
	String^ folderName = service->GetCurFolderName();
	if (!folderName)
	{
		folderName = L"Root";
	}
	pageTitle->Text = folderName;
	
	IVectorView<FileItem^>^ itemsView  = service->GetItemsView();
	itemGridView->Items->Clear();

	for each(auto item in itemsView)
	{
		itemGridView->Items->Append(item);
	}

	
}
void ExplorerPage::UpdateProgress()
{
	ProgressRing->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	ProgressRing->IsActive = false;

	App^ app = safe_cast<App^>(App::Current);
	OneDriveService^ service = app->GetOneDriveService();
	if (!service)
		return;

	if (service->IsBusy())
	{
		ProgressRing->Visibility = Windows::UI::Xaml::Visibility::Visible;
		ProgressRing->IsActive = true;
	}
}
ExplorerPage::ExplorerPage()
{
	InitializeComponent();
}


void ExplorerPage::itemGridView_ItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e)
{
	FileItem^ item = (FileItem^) e->ClickedItem;

	if (item->GetType() != SKY_FILE_TYPE_FOLDER)
		return;

	App^ app = safe_cast<App^>(App::Current);
	if (!app)
		return;
	
	OneDriveService^ service = app->GetOneDriveService();
	service->RequestFolder(item);
}


void ExplorerPage::itemGridView_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	
}


void ExplorerPage::backButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	App^ app = safe_cast<App^>(App::Current);
	if (!app)
		return;
	
	OneDriveService^ service = app->GetOneDriveService();
	service->RequestParentFolder();
}


void ExplorerPage::Grid_DragEnter(Platform::Object^ sender, Windows::UI::Xaml::DragEventArgs^ e)
{
	e->AcceptedOperation = Windows::ApplicationModel::DataTransfer::DataPackageOperation::Link;
	auto t = e->Handled;
	e->Handled = true;
	WriteDebugStringW(L"OnDragEnter\n");
}


void ExplorerPage::Grid_Drop(Platform::Object^ sender, Windows::UI::Xaml::DragEventArgs^ e)
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
			WriteDebugStringW(Msg->Data());

			if (item->IsOfType(Windows::Storage::StorageItemTypes::File))
			{
				Windows::Storage::StorageFile^ file = safe_cast<Windows::Storage::StorageFile^>(item);

				create_task(file->GetBasicPropertiesAsync()).then([this,item,file](Windows::Storage::FileProperties::BasicProperties^ fileProperty)
				{
					UINT64	ui64FileSize = fileProperty->Size;
					UploadFile(item->Name, file, ui64FileSize);
				},ui);
			}
			else
			{
				Windows::Storage::IStorageFolder^ folder = safe_cast<Windows::Storage::IStorageFolder^>(item);
				//GetFilesInFolder(folder);
			}
		}
		d->Complete();
	
	},ui);	
}

void ExplorerPage::UploadFile(String^ FileName, Windows::Storage::StorageFile^ file, UINT64 ui64FileSize)
{
	App^ app = safe_cast<App^>(App::Current);
	if (!app)
		return;

	OneDriveService^ service = app->GetOneDriveService();
	service->UploadFile(FileName, file, ui64FileSize);
}

void OneDriveAccess::ExplorerPage::UploadButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	
	App^ app = safe_cast<App^>(App::Current);
	if (!app)
		return;
	
	OneDriveService^ service = app->GetOneDriveService();

	FileOpenPicker^ openPicker = ref new FileOpenPicker();
    openPicker->ViewMode = PickerViewMode::List;
    openPicker->SuggestedStartLocation = PickerLocationId::ComputerFolder;
	
    openPicker->FileTypeFilter->Append("*");

	create_task(openPicker->PickMultipleFilesAsync()).then([this,service](IVectorView<Windows::Storage::StorageFile^>^ files)
    {
		for each (Windows::Storage::StorageFile^ file in files)
		{
			String^ FullPath = file->Path;
			const WCHAR*	wchFullPath = FullPath->Data();
			
			Platform::String^ Msg = FullPath + L"\n";
			WriteDebugStringW(Msg->Data());

			create_task(file->GetBasicPropertiesAsync()).then([this,service,file](Windows::Storage::FileProperties::BasicProperties^ fileProperty)
			{
				UINT64	ui64FileSize = fileProperty->Size;
				service->UploadFile(file->Name, file, ui64FileSize);
			});
		}
    });



	
//	service->UploadFile(FileName, file, ui64FileSize);
}
