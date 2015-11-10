//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

class CVideoPlayer;
namespace D3DVideoPlayer
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
		
		float	m_fCompositionScaleX;
		float	m_fCompositionScaleY;
		bool	m_windowVisible;
		Windows::Foundation::EventRegistrationToken         m_onRenderingEventToken;
	public:
		MainPage();
		void	OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
		void	OnNavigatedFrom(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
	internal:
	
		Windows::UI::Xaml::Controls::SwapChainPanel^ GetSwapChainPanel();
	private:
		~MainPage();
		void OnUpdate(_In_ Object^ /* sender */,_In_ Object^ /* args */);

		void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
		void OnDpiChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnOrientationChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnDisplayContentsInvalidated(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel^ sender, Object^ args);
		void OnSwapChainPanelSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);

		void LoadButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);


		void ExitButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void Grid_DragEnter(Platform::Object^ sender, Windows::UI::Xaml::DragEventArgs^ e);
		void Grid_Drop(Platform::Object^ sender, Windows::UI::Xaml::DragEventArgs^ e);

		void MediaFailed(Platform::Object^ sender, Windows::UI::Xaml::ExceptionRoutedEventArgs^ e);
		void DisplayErrorMessage(Platform::String^ message);
		void swapChainPanel_Tapped(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e);
		void ShaderList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
	};
}
