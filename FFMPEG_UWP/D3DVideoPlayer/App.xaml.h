//
// App.xaml.h
// Declaration of the App class.
//

#pragma once

#include "App.g.h"

namespace D3DVideoPlayer
{
	/// <summary>
	/// Provides application-specific behavior to supplement the default Application class.
	/// </summary>
	ref class App sealed
	{
		
	protected:
		virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override;

	internal:
		App();

	private:
		void BeginHeapCheck();
		void EndHeapCheck();

		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);
		void OnNavigationFailed(Platform::Object ^sender, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs ^e);
		void OnClosed(Windows::UI::Core::CoreWindow ^ window, Windows::UI::Core::CoreWindowEventArgs^ args);
		
	};
}
