﻿#include "pch.h"
#include "ContactsTab.xaml.h"
#if __has_include("ContactsTab.g.cpp")
#include "ContactsTab.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Pine::implementation
{
    ContactsTab::ContactsTab()
    {
        InitializeComponent();
        NavigationCacheMode(Navigation::NavigationCacheMode::Enabled);
    }
}
