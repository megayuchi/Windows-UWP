//
// BlankPage.xaml.h
// Declaration of the BlankPage class
//

#pragma once

#include "BlankPage.g.h"

namespace D3DVideoPlayer
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class BlankPage sealed
	{
	public:
		void	OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
		void	OnNavigatedFrom(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
		BlankPage();
	};
}
