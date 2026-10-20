using System.Diagnostics;
using System.IO;

namespace GameLauncher.backend;

public class Launcher
{
    public static readonly string Python = Path.Combine(AppContext.BaseDirectory, "Server","Embedded","python.exe");
    public static readonly string FlaskServer = Path.Combine(AppContext.BaseDirectory, "Server","main.py");
    public static readonly string Game = Path.Combine(AppContext.BaseDirectory,"Windows","TheForgottenPyramid.exe");

    private Process? _flask;
    private Process? _game;

    private Process? Start(string exe, string args)
    {
        var info = new ProcessStartInfo()
        {
            FileName = exe,
            Arguments = args,
            WorkingDirectory = AppContext.BaseDirectory,
            RedirectStandardOutput = false,
            RedirectStandardError = false,
            UseShellExecute = false,
            CreateNoWindow = true
        };
        return Process.Start(info);
    }

    private void Stop(Process? process)
    {
        if (process is null || process.HasExited)
        {
            return;
        }

        try
        {
            process.CloseMainWindow();
            if (!process.WaitForExit(3000))
            {
                process.Kill();
            }
        }
        catch (Exception e)
        {
            Console.Error.WriteLine($"Error stopping process: {e.Message}");
        }
    }

    public void Launch()
    {
        _flask = Start(Python, FlaskServer);
        _game = Start(Game,"");
    }

    public void Terminate()
    {
        Stop(_game);
        Stop(_flask);
    }
}