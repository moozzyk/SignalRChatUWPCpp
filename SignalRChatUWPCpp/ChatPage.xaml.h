//
// ChatPage.xaml.h
// Declaration of the ChatPage class
//

#pragma once

#include "ChatPage.g.h"
#include "signalrclient\hub_connection.h"

namespace SignalRChatUWPCpp
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class ChatPage sealed
	{
	public:
		ChatPage();

    protected:
        virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

    private:
        std::unique_ptr<signalr::hub_connection> m_chat_connection;
        std::unique_ptr<signalr::hub_proxy> m_chat_hub;
        void SendMessageButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    };
}
