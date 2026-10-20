using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Net.Http;

namespace GameLauncher.backend;

public class PythonInstaller
{
    private static readonly string EmbedUrl = "https://www.python.org/ftp/python/3.12.0/python-3.12.0-embed-amd64.zip";
    private static readonly string PipUrl = "https://bootstrap.pypa.io/get-pip.py";
    private static readonly string InstallDir = Path.Combine(AppContext.BaseDirectory, "Server");
    private static readonly string EmbedDir = Path.Combine(InstallDir, "Embedded");
    private static readonly string PythonFile = Path.Combine(EmbedDir, "python.exe");
    private static readonly string EmbedZip = Path.Combine(InstallDir, "python-embed.zip");
    private static readonly string pthFile = Path.Combine(EmbedDir, "python312._pth");
    private static readonly string PipPath = Path.Combine(InstallDir, "get-pip.py");
    private static readonly string Ready = Path.Combine(InstallDir, "Ready");
    private static readonly string[] libs = { "pandas", "matplotlib", "opencv-python", "flask" , "scikit-learn", "tensorflow-cpu"};

    public static bool IsPythonInstalled()
    {
        return File.Exists(Ready);
    }
    public static async Task<Result> RunCommandAsync(string filename, string args,string workingDirectory)
    {
        ProcessStartInfo info = new ProcessStartInfo()
        {
            FileName = filename,
            Arguments = args,
            WorkingDirectory = workingDirectory,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false,
            CreateNoWindow = true
        };
        var process = Process.Start(info);
        process.WaitForExit();
        string message = await (process.ExitCode == 0 ? process.StandardOutput : process.StandardError).ReadToEndAsync();
        return new Result(process.ExitCode, message);
    }

    private static async Task<Result> DownloadAsync(string url, string path)
    {
        try
        {
            using HttpClient http = new HttpClient();
            var response = await http.GetAsync(url, HttpCompletionOption.ResponseHeadersRead);
            if (!response.IsSuccessStatusCode)
            {
                throw new Exception($"Http get failed, code {response.StatusCode}");
            }
            await using var stream = await response.Content.ReadAsStreamAsync();
            await using var fileStream = new FileStream(path, FileMode.Create, FileAccess.Write, FileShare.None);
            await stream.CopyToAsync(fileStream);
            return new Result(0, "Download Complete");
        }
        catch (Exception e)
        {
            return new Result(1, e.Message);
        }
    }

    private static async Task<Result> UsePythonAsync(string args)
    {
        return await RunCommandAsync(PythonFile,args,InstallDir);
    }

    private static Result Delete(string path)
    {
        try
        {
            if (File.Exists(path))
            {
                File.Delete(path);
            }

            if (Directory.Exists(path))
            {
                Directory.Delete(path,true);
            }

            return new Result(0, "Delete done");
        }
        catch (Exception e)
        {
            return new Result(1, e.Message);
        }
    }
    
    public static async Task<Result> InstallPythonAsync()
    {
        if (!Directory.Exists(InstallDir))
        {
            Directory.CreateDirectory(InstallDir);
        }
        Result download = await DownloadAsync(EmbedUrl, EmbedZip);
        if (download.code != 0)
        {
            return download;
        }
        Delete(EmbedDir);
        ZipFile.ExtractToDirectory(EmbedZip,EmbedDir);
        return new Result(0, "Installation Complete");
    }

    public static Result ModifyPth()
    {
        if (!File.Exists(pthFile))
        {
            return new Result(1, "pth file missing");
        }
        string[] lines = File.ReadAllLines(pthFile);
        for (int i = 0; i < lines.Length; i++)
            if (lines[i].StartsWith("#import site")) lines[i] = "import site";
        File.WriteAllLines(pthFile, lines);
        File.AppendAllLines(pthFile, new[] { "..", "Lib", "Lib/site-packages" });
        return new Result(0, "Pth modification Complete");
    }

    public static async Task<Result> InstallPip()
    {
        Result download = await DownloadAsync(PipUrl, PipPath);
        if (download.code != 0)
        {
            return download;
        }

        Result install = await UsePythonAsync(PipPath);
        return install.code == 0 ? new Result(0,"Installation Complete") : install;
    }

    public static async Task<Result> InstallLibraries()
    {
        foreach (string lib in libs)
        {
            Console.WriteLine($"Installing {lib}");
            Result install = await UsePythonAsync($"-m pip install {lib}");
            if (install.code != 0)
            {
                return install;
            }
            Console.WriteLine(install.message);
        }
        return new Result(0, "Libraries installed");
    }

    public static Result CleanUp()
    {
        Delete(EmbedZip);
        Delete(PipPath);
        if (!File.Exists(Ready))
        {
            File.Create(Ready);
        }
        return new Result(0, "Cleaned Up");
    }

}