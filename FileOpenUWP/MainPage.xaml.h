//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

namespace FileOpenUWP
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

	private:
		void PickButton_ClickSingle(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void PickButton_ClickMultiple(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void PickButton_ClickFolder(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void ReadTextFromFile(Windows::Storage::IStorageFile^ file);
	};
}
