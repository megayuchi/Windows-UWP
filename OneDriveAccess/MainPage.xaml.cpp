//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include <experimental\resumable>
#include <ppl.h>
#include <pplawait.h>
#include <ppltasks.h>

#include "MainPage.xaml.h"
#include "ExplorerPage.xaml.h"
#include "OneDriveService.h"
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
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::Web::Http;
using namespace Windows::Data::Json;
using namespace concurrency;


// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

using namespace Windows::Security;

using namespace Windows::Storage::Pickers;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::Web::Http;

MainPage::MainPage()
{
	InitializeComponent();
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
	UpdateSigninStatus();
	UpdateProgress();
	
}
void MainPage::UpdateSigninStatus()
{
	UserName->Text = L"";
	SignInOutButton->Content = L"Sign-in";

	OneDriveAccess::App^ app = safe_cast<OneDriveAccess::App^>(OneDriveAccess::App::Current);
	OneDriveAccess::OneDriveService^ service = app->GetOneDriveService();
	if (!service)
		return;

	UserName->Text = service->GetUserName();
	String^ token = service->GetAccessToken();
	if (token)
	{
		SignInOutButton->Content = L"Sign-out";
	}
	
}
void MainPage::UpdateProgress()
{
	ProgressRing->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	ProgressRing->IsActive = false;

	OneDriveAccess::App^ app = safe_cast<OneDriveAccess::App^>(OneDriveAccess::App::Current);
	OneDriveAccess::OneDriveService^ service = app->GetOneDriveService();
	if (!service)
		return;

	if (service->IsBusy())
	{
		ProgressRing->Visibility = Windows::UI::Xaml::Visibility::Visible;
		ProgressRing->IsActive = true;
	}
}
void MainPage::OnNavigatedFrom(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e)
{
	// Called when a page is no longer the active page in a frame.
	int a = 0;

}

void MainPage::LoginButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	String^ token = nullptr;
	OneDriveAccess::App^ app = safe_cast<OneDriveAccess::App^>(OneDriveAccess::App::Current);
	OneDriveAccess::OneDriveService^ service = app->GetOneDriveService();
	
	if (!service)
		return;
	
	token = service->GetAccessToken();
	
	if (token)
	{
		service->SignOut();
	}
	else
	{
		service->SignIn();
	}

	
}


void MainPage::ExploreFoldersButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
		
	OneDriveAccess::App^ app = safe_cast<OneDriveAccess::App^>(OneDriveAccess::App::Current);
	OneDriveAccess::OneDriveService^ service = app->GetOneDriveService();
	if (!service)
		return;

	String^ token = service->GetAccessToken();
	if (!token)
	{
		return;
	}
	service->RequestFolder(nullptr);
}

