//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include <ppltasks.h>
#include <ppl.h>

namespace OneDriveAccess
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
		Windows::Security::Authentication::OnlineId::OnlineIdAuthenticator^ m_authenticator;

	public:
		void	OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
		void	OnNavigatedFrom(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
		void	UpdateSigninStatus();
		void	UpdateProgress();
		MainPage();
	internal:
		
	private:
		void LoginButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void ExploreFoldersButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
