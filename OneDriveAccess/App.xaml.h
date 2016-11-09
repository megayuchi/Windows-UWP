//
// App.xaml.h
// Declaration of the App class.
//

#pragma once

#include "App.g.h"
#include "OneDriveService.h"

namespace OneDriveAccess
{
	/// <summary>
	/// Provides application-specific behavior to supplement the default Application class.
	/// </summary>
	ref class App sealed
	{
		const WCHAR* CLIENT_ID = L"00000000441AC9BC";
		OneDriveService^ m_OneDriveService;
		Windows::UI::Core::CoreDispatcher^	m_CoreDispatcher = nullptr;
	protected:
		virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override;

	internal:
		Windows::UI::Core::CoreDispatcher^	GetCoreDispatcher()	{return m_CoreDispatcher;}
		void UpdateFolderStatus();
		void UpdateSigninStatus();
		void UpdateProgress();
		String^	GetClientID()	{return ref new String(CLIENT_ID);}
		App();

	private:
		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);
		void OnNavigationFailed(Platform::Object ^sender, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs ^e);
	public:
		OneDriveService^ GetOneDriveService() 
		{
			return m_OneDriveService;
		}
	};
}
