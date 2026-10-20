using System.ComponentModel;
using System.Globalization;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Media.Imaging;
using GameLauncher.backend;

namespace GameLauncher.frontend;

public partial class Message : UserControl
{
    public enum MessageStatus
    {
        WIP,COMPLETED,ERROR
    }

    public static readonly DependencyProperty StatusProperty =
        DependencyProperty.Register(nameof(Status), typeof(MessageStatus),
            typeof(Message), new PropertyMetadata(MessageStatus.WIP));

    public static readonly DependencyProperty ContentProperty =
        DependencyProperty.Register(nameof(Content), typeof(string),
            typeof(Message), new PropertyMetadata("Content"));
        

    public MessageStatus Status
    {
        get => (MessageStatus)GetValue(StatusProperty);
        set => SetValue(StatusProperty, value);
    }

    public string Content
    {
        get => (string)GetValue(ContentProperty);
        set => SetValue(ContentProperty, value);
    }

    public Message()
    {
        InitializeComponent();
    }

    public void Update(Result result)
    {
        Content = result.message;
        Status = result.code == 0 ? MessageStatus.COMPLETED : MessageStatus.ERROR;
    }

}
public class StatusImageConverter : IValueConverter
{
    private static BitmapImage AsImage(string relativePath)
    {
        return new BitmapImage(new Uri($"pack://application:,,,/{relativePath}", UriKind.Absolute));
    }

    public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
    {
        if (value is Message.MessageStatus status)
        {
            return status switch
            {
                Message.MessageStatus.WIP => AsImage("Images/wip.png"),
                Message.MessageStatus.COMPLETED => AsImage("Images/ticked.png"),
                Message.MessageStatus.ERROR => AsImage("Images/crossed.png"),
                _ => null!
            };
        }

        return null!;
    }

    public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
    {
        throw new NotImplementedException();
    }
}