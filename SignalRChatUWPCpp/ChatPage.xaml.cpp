//
// ChatPage.xaml.cpp
// Implementation of the ChatPage class
//

#include "pch.h"
#include "ChatPage.xaml.h"

using namespace SignalRChatUWPCpp;

using namespace Platform;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

ChatPage::ChatPage()
{
	InitializeComponent();
}

void ChatPage::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e)
{
    if (e->Parameter)
    {
        m_name = (static_cast<String ^>(e->Parameter))->Data();
    }

    this->ChatTextBox->Text = "Connecting...";
    this->SendMessageButton->IsEnabled = false;
    this->MessageTextBox->IsEnabled = false;

    m_chat_connection = std::make_unique<signalr::hub_connection>(L"http://testsignalrchat.azurewebsites.net");
    m_chat_hub = std::make_unique<signalr::hub_proxy>(m_chat_connection->create_hub_proxy(L"ChatHub"));

    m_chat_hub->on(L"broadcastMessage", [this](const web::json::value& m)
    {
        auto sender = m.at(0).as_string();
        auto message = m.at(1).as_string();

        CoreApplication::MainView->CoreWindow->Dispatcher->RunAsync(
            CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, sender, message]()
        {
            auto line = ref new String((sender + L": " + message + L"\r\n").c_str());
            this->ChatTextBox->Text =
                String::Concat(this->ChatTextBox->Text, line);
        }));
    });

    auto ui_ctx = pplx::task_continuation_context::use_current();
    m_chat_connection->start()
        .then([this](pplx::task<void> start_task)
        {
            Platform::String^ error_message;
            try
            {
                start_task.get();
                this->ChatTextBox->Text = "Connected\r\n";
                this->SendMessageButton->IsEnabled = true;
                this->MessageTextBox->IsEnabled = true;

            }
            catch (const std::exception& e)
            {
                this->ChatTextBox->Text =
                    ref new Platform::String((L"Connecting to chat failed" + utility::conversions::to_string_t(e.what())).c_str());
            }
        }, ui_ctx);
}

void SignalRChatUWPCpp::ChatPage::SendMessageButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    web::json::value args{};
    args[0] = web::json::value::string(m_name);
    args[1] = web::json::value(this->MessageTextBox->Text->Data());
    this->MessageTextBox->Text = L"";

    auto ui_ctx = pplx::task_continuation_context::use_current();
    m_chat_hub->invoke<void>(L"send", args)
        .then([this](pplx::task<void> invoke_task)
        {
            try
            {
                invoke_task.get();
            }
            catch (const std::exception& e)
            {
                auto error_line = ref new String(
                    (L"Invoking 'send' function failed:" + utility::conversions::to_string_t(e.what())).c_str());
                this->ChatTextBox->Text =
                    String::Concat(this->ChatTextBox->Text, error_line);
            }
        }, ui_ctx);
}
