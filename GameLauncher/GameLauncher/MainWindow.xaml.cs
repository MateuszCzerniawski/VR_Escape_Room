using System.Diagnostics;
using System.IO;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using GameLauncher.backend;
using GameLauncher.frontend;
using Path = System.IO.Path;

namespace GameLauncher;


public partial class MainWindow : Window
{
    private Launcher? _launcher;
    public MainWindow()
    {
        InitializeComponent();
        TextLoadBar.Content =  PythonInstaller.IsPythonInstalled()? "Game is ready!":"Install dependencies to play";
        Btn.Action = PythonInstaller.IsPythonInstalled() ? MainButton.ActionType.LAUNCH : MainButton.ActionType.INSTALL;
    }

    public async void MainButton_Click(object sender, RoutedEventArgs args)
    {
        if (sender is MainButton btn)
        {
            switch (btn.Action)
            {
                case frontend.MainButton.ActionType.INSTALL:
                    await InstallPythonAsync();
                    break;
                case frontend.MainButton.ActionType.LAUNCH:
                    LaunchGameAndServerAsync();
                    break;
                case frontend.MainButton.ActionType.TERMINATE:
                    StopGameAndServerAsync();
                    break;
            }
        }
    }

    private async Task InstallPythonAsync()
    {
        MessageConsolePanel.SetVisibility(true);
        TextLoadBar.StartProgress(5);

        Message installPythonMessage = MessageConsolePanel.AddMessage("Installing Python");
        installPythonMessage.Update(await PythonInstaller.InstallPythonAsync());
        TextLoadBar.Progress();

        Message modifyPthMessage = MessageConsolePanel.AddMessage("Modifying pth file");
        modifyPthMessage.Update(PythonInstaller.ModifyPth());
        TextLoadBar.Progress();

        Message installPipMessage = MessageConsolePanel.AddMessage("Installing pip");
        installPipMessage.Update(await PythonInstaller.InstallPip());
        TextLoadBar.Progress();

        Message installLibrariesMessage = MessageConsolePanel.AddMessage("Installing libraries");
        installLibrariesMessage.Update(await PythonInstaller.InstallLibraries());
        TextLoadBar.Progress();

        Message cleanupMessage = MessageConsolePanel.AddMessage("Cleaning up");
        cleanupMessage.Update(PythonInstaller.CleanUp());
        TextLoadBar.Progress();

        TextLoadBar.Content = "Game is ready!";
        MessageConsolePanel.SetVisibility(false);
        Btn.Action = MainButton.ActionType.LAUNCH;
    }

    private void LaunchGameAndServerAsync()
    {
        _launcher = new Launcher();
        _launcher.Launch();
        Btn.Action = MainButton.ActionType.TERMINATE;
        TextLoadBar.Content = "Stop backend here, close game window manually";
    }

    private void StopGameAndServerAsync()
    {
        _launcher?.Terminate();
        Btn.Action =  PythonInstaller.IsPythonInstalled() ? MainButton.ActionType.LAUNCH : MainButton.ActionType.INSTALL;
        TextLoadBar.Content = "Game is ready!";
    }
}