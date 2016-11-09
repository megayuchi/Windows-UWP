#include "pch.h"
#include "FileItem.h"
#include "DelegateCommand.h"
#include "OneDriveService.h"

using namespace OneDriveAccess;
using namespace concurrency;
using namespace Windows::UI::Popups;

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;

using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::ViewManagement;


using namespace Windows::Storage;
using namespace Windows::Storage::FileProperties;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage::Pickers;
using namespace Windows::Media::MediaProperties;

using namespace concurrency;

using namespace Windows::Graphics::Display;

using namespace Windows::UI::Xaml::Media;


FileItem::FileItem()
{
	m_playCommand = ref new DelegateCommand(
        ref new ExecuteDelegate(this, &OneDriveAccess::FileItem::OnCommandPlay),
        ref new CanExecuteDelegate(this, &OneDriveAccess::FileItem::OnCommandCanPlaying));

	m_downloadCommand = ref new DelegateCommand(
        ref new ExecuteDelegate(this, &OneDriveAccess::FileItem::OnCommandDownload),
        ref new CanExecuteDelegate(this, &OneDriveAccess::FileItem::OnCommandCanDownload));

	m_deleteCommand = ref new DelegateCommand(
        ref new ExecuteDelegate(this, &OneDriveAccess::FileItem::OnCommandDelete),
        ref new CanExecuteDelegate(this, &OneDriveAccess::FileItem::OnCommandCanDelete));
}

void FileItem::OnCommandPlay(Object^ parameter)
{
	if (GetType() == SKY_FILE_TYPE_FOLDER)
		return;

	OneDriveAccess::App^ app = safe_cast<OneDriveAccess::App^>(OneDriveAccess::App::Current);
	if (!app)
		return;
	
	OneDriveService^ service = app->GetOneDriveService();
	

}
bool FileItem::OnCommandCanPlaying(Object^ parameter)
{
	
	return true;
}

void FileItem::OnCommandDownload(Object^ parameter)
{
	if (GetType() == SKY_FILE_TYPE_FOLDER)
		return;

	OneDriveAccess::App^ app = safe_cast<OneDriveAccess::App^>(OneDriveAccess::App::Current);
	if (!app)
		return;
	
	OneDriveService^ service = app->GetOneDriveService();
	service->DownloadFile(this);

}
bool FileItem::OnCommandCanDownload(Object^ parameter)
{
	
	return true;
}

void FileItem::OnCommandDelete(Object^ parameter)
{
	
	
}
bool FileItem::OnCommandCanDelete(Object^ parameter)
{
	return true;
}
void FileItem::SetType(OneDriveAccess::SKY_FILE_TYPE type)
{
	m_Type = type;
	SetImage();
}
void FileItem::SetImage()
{
	String^ ImagePath = nullptr;
	Windows::Foundation::Uri^ _baseUri = nullptr;
		
	_baseUri = ref new Windows::Foundation::Uri("ms-appx:///");
	ImagePath = L"Images/Icon_Etc.png";
	
	switch (m_Type)
	{
	case SKY_FILE_TYPE_FOLDER:
		ImagePath = L"Images/Icon_Folder.png";
		break;
	case SKY_FILE_TYPE_AUDIO:
		ImagePath = L"Images/Icon_Audio.png";
		break;
	default:
		ImagePath = L"Images/Icon_Etc.png";
		break;

	}
	SetImage(_baseUri, ImagePath);
}