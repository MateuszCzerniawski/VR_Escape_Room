using System.Windows;
using System.Windows.Controls;
namespace GameLauncher.frontend;

public partial class MessageConsole : UserControl
{
    private List<Message> _messages = new List<Message>();
    
    public MessageConsole()
    {
        InitializeComponent();
    }

    public void SetVisibility(bool visible)
    {
        Border.Visibility = visible ? Visibility.Visible : Visibility.Collapsed;
    }

    public Message AddMessage(Message message)
    {
        _messages.Add(message);
        MessagesStackPanel.Children.Add(message);
        Dispatcher.InvokeAsync(()=>ScrollViewer.ScrollToEnd());
        return message;
    }

    public Message AddMessage(string msg)
    {
        return AddMessage(new Message{Content = msg});
    }

    public Message? GetMessage(int index)
    {
        if (index >= 0 && index < _messages.Count)
        {
            return _messages[index];
        }
        return null;
    }

    public Message? GetLastMessage()
    {
        return GetMessage(_messages.Count - 1);
    }
}