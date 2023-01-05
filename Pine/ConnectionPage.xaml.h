﻿#pragma once

#include "ConnectionPage.g.h"


namespace winrt::Pine::implementation
{
	struct ConnectionPage : ConnectionPageT<ConnectionPage>
	{
		ConnectionPage();
		
		virtual void OnNavigatedTo(Microsoft::UI::Xaml::Navigation::NavigationEventArgs const&);

		Windows::Foundation::IAsyncAction TryConnect();
		void PasswordBox_KeyDown(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e);
		void Button_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void Id_KeyDown(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e);
	};
}

namespace winrt::Pine::factory_implementation
{
	struct ConnectionPage : ConnectionPageT<ConnectionPage, implementation::ConnectionPage>
	{
	};
}
