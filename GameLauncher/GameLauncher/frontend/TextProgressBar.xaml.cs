using System.Windows;
using System.Windows.Controls;

namespace GameLauncher.frontend;

public partial class TextProgressBar : UserControl
{
    public static readonly DependencyProperty ContentProperty =
        DependencyProperty.Register(nameof(Content), typeof(string),
            typeof(TextProgressBar), new PropertyMetadata("Text"));
    
    public static readonly DependencyProperty CurrentStepProperty =
        DependencyProperty.Register(nameof(CurrentStep), typeof(int),
            typeof(TextProgressBar), new PropertyMetadata(0));
    
    public static readonly DependencyProperty TotalStepsProperty =
        DependencyProperty.Register(nameof(TotalSteps), typeof(int),
            typeof(TextProgressBar), new PropertyMetadata(5));

    public string Content
    {
        get => (string)GetValue(ContentProperty);
        set => SetValue(ContentProperty, value);
    }
    public int CurrentStep
    {
        get => (int)GetValue(CurrentStepProperty);
        set => SetValue(CurrentStepProperty, value);
    }
    public int TotalSteps
    {
        get => (int)GetValue(TotalStepsProperty);
        set => SetValue(TotalStepsProperty, value);
    }

    public TextProgressBar()
    {
        InitializeComponent();
        SwitchTextAndBarVisibility(false);
    }

    public void SwitchTextAndBarVisibility(bool barVisible)
    {
        InfoLabel.Visibility = barVisible ? Visibility.Collapsed : Visibility.Visible;
        LoadBar.Visibility = barVisible ? Visibility.Visible : Visibility.Collapsed;
    }

    public void StartProgress(int totalSteps)
    {
        TotalSteps = totalSteps;
        CurrentStep = 0;
        SwitchTextAndBarVisibility(true);
    }

    public void Progress()
    {
        CurrentStep++;
        if (CurrentStep >= TotalSteps)
        {
            SwitchTextAndBarVisibility(false);
        }
    }
}