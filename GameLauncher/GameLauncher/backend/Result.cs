namespace GameLauncher.backend;

public struct Result
{
    public int code { get; }
    public string message { get; }
    
    public Result(int code, string message)
    {
        this.code = code;
        this.message = message;
    }
}