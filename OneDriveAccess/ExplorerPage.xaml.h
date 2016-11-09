//
// ExplorerPage.xaml.h
// Declaration of the ExplorerPage class
//

#pragma once

#include "ExplorerPage.g.h"

using namespace Platform;
using namespace Platform::Collections;

namespace OneDriveAccess
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class ExplorerPage sealed
	{
		
	public:
		void UpdateFolderStatus();
		void UpdateProgress();
		virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
		virtual void OnNavigatedFrom(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

		ExplorerPage();
	private:
		void itemGridView_ItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e);
		void itemGridView_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void backButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Grid_DragEnter(Platform::Object^ sender, Windows::UI::Xaml::DragEventArgs^ e);
		void Grid_Drop(Platform::Object^ sender, Windows::UI::Xaml::DragEventArgs^ e);
		void UploadFile(String^ FileName, Windows::Storage::StorageFile^ file, UINT64 ui64FileSize);
		void UploadButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
