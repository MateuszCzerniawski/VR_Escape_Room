using System.Windows;
using System.Windows.Controls;

namespace GameLauncher.frontend;

public partial class MainButton : UserControl
{
    public enum ActionType
    {
        INSTALL,LAUNCH,TERMINATE
    }

    public static readonly DependencyProperty ActionProperty =
        DependencyProperty.Register(nameof(Action),typeof(ActionType),
            typeof(MainButton),new PropertyMetadata(ActionType.INSTALL, OnActionChanged));

    public ActionType Action
    {
        get => (ActionType)GetValue(ActionProperty);
        set => SetValue(ActionProperty, value);
    }

    private static void OnActionChanged(DependencyObject obj, DependencyPropertyChangedEventArgs args)
    {
        if (obj is MainButton control)
        {
           control.UpdateLabel();
        }
    }

    public MainButton()
    {
        InitializeComponent();
        UpdateLabel();
    }

    private void UpdateLabel()
    {
        Label.Text = Action.ToString().ToUpper();
    }
    public event RoutedEventHandler? Click;
    
    private void Button_Click(object sender, RoutedEventArgs args)
    {
        Click?.Invoke(this,args);
    }
}