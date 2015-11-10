//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "SimpleObject.h"

using namespace RefClass;

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

using namespace Windows::UI::Popups;
// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

MainPage::MainPage()
{
	InitializeComponent();

	TestRefClass();

}
void MainPage::TestRefClass()
{
	CSimpleObject^	Obj0 = ref new CSimpleObject(L"Object 0",0);
	CSimpleObject^	Obj1 = ref new CSimpleObject(L"Object 1",1);

	
	CSimpleObject^	Obj2 = Obj0 + Obj1;
	CSimpleObject^	Obj3 = Obj2;	// ref count 1 증가
	

	Obj0 = nullptr;		// ref count가 0이 되어 해제
	Obj1 = nullptr;		// ref count가 0이 되어 해제
	Obj2 = nullptr;		// ref coutn가 1 남아있으므로 해제되지 않음.
	Obj3 = nullptr;		// ref count가 0이 되어 해제
}


void MainPage::MessageDialogOkCancel(Platform::String^ Message)
{
	MessageDialog^ msg = ref new MessageDialog(Message);

	// OK버튼을 눌렀을때의 이벤트 핸들러를 Lambda식으로 전달
	UICommand^ cmdOK = ref new UICommand("OK",ref new UICommandInvokedHandler([this](IUICommand^)	
	{
		// On Pressed OK

	}));
	
	// Cancel버튼을 눌렀을때의 이벤트 핸들러를 Lambda식으로 전달
	UICommand^ cmdCanccel = ref new UICommand("Cancel",ref new UICommandInvokedHandler([this](IUICommand^)	
	{
		// On Pressed Cancel

	}));

	msg->Commands->Append(cmdOK);
	msg->Commands->Append(cmdCanccel);

	msg->DefaultCommandIndex = 0;
	msg->CancelCommandIndex = 1;

	msg->ShowAsync();
}