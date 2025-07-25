using System.IO;
using System;

namespace UnrealBuildTool.Rules
{

  public class GBSNats : ModuleRules
  {
    public GBSNats(ReadOnlyTargetRules Target) : base(Target)
    {

      //PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
      //PrecompileForTargets = PrecompileTargetsType.Any;

      PrivateIncludePaths.AddRange(

          new string[] {
          "GBSNats/Private",
          "../ThirdParty/nats/Binaries/include",
          NatsIncludePath
            // ... add other private include paths required here ...
          });

      PublicDependencyModuleNames.AddRange(
        new string[]
        {
          "Core",
          "CoreUObject",
          "Engine",
          "Json",
          "Projects",
          "Networking",
          "Sockets" // Required by IPluginManager etc (used to get plugin information)
					// ... add other public dependencies that you statically link with here ...
				});

        if (IsUnix)
        {
          BuildNats(Target);
          PublicDefinitions.Add("USE_NATS");
          PublicIncludePaths.Add(NatsIncludePath);
          PublicAdditionalLibraries.Add(NatsLibraryPath + "/libnats.so");
          PublicAdditionalLibraries.Add("/usr/lib/x86_64-linux-gnu/libprotobuf-c.so");        
        }	  
      else
      {
          BuildNats(Target);
          PublicDefinitions.Add("USE_NATS");
          PublicIncludePaths.Add(NatsIncludePath);
          PublicAdditionalLibraries.Add(NatsLibraryPath + "/nats.lib");
          CopyToBinaries(ProtobufCDLLPath);
          CopyToBinaries(NatsDLLPath);
          //PublicAdditionalLibraries.Add(NatsLibraryPath + "/protobuf-c.lib");
      }
    }
    private void CopyToBinaries(string FilePath)
    {
      string binariesDir = Path.Combine(ProjectRoot, "Binaries", Target.Platform.ToString());
      string filename = Path.GetFileName(FilePath);

        if (!Directory.Exists(binariesDir))
            Directory.CreateDirectory(binariesDir);

        if (!File.Exists(Path.Combine(binariesDir, filename)))
            File.Copy(FilePath, Path.Combine(binariesDir, filename), true);    

    }

    private bool BuildNats(ReadOnlyTargetRules Target)
    {
      Console.WriteLine("Executing " + NatsBuildScript);

      var buildCode = ExecuteCommandSync(NatsBuildScript);

      if (buildCode != 0)
      {
        Console.WriteLine(
            "\nCannot configure CMake project. Exited with code: "
            + buildCode);
        return false;
      }

      return true;
    }

    public int ExecuteCommandSync(string command)
    {
      try
      {
		string args = GBSNatsPath + " " + BinaryOutputDir;
        System.Diagnostics.ProcessStartInfo procStartInfo;
		if (IsUnix) 
		{
			procStartInfo = new System.Diagnostics.ProcessStartInfo(command, args);
		}
		else
		{
            // For Windows, it should be the following
            procStartInfo = new System.Diagnostics.ProcessStartInfo("cmd", "/c " + command + " " + args);
		}
		
        // The following commands are needed to redirect the standard output.
        // This means that it will be redirected to the Process.StandardOutput StreamReader.
        procStartInfo.RedirectStandardOutput = true;
        procStartInfo.UseShellExecute = false;
        // Do not create the black window.
        procStartInfo.CreateNoWindow = true;
        // Now we create a process, assign its ProcessStartInfo and start it
        System.Diagnostics.Process proc = new System.Diagnostics.Process();
        proc.StartInfo = procStartInfo;
        proc.Start();
        // Get the output into a string
        string result = proc.StandardOutput.ReadToEnd();
        // Display the command output.
        Console.WriteLine(result);
        proc.WaitForExit();
        return proc.ExitCode;
      }
      catch (Exception objException)
      {

        // Log the exception
      }
      return 1;
    }


    private bool IsUnix
    { 
      get { return Target.IsInPlatformGroup(UnrealPlatformGroup.Unix); }
    }

    private string BinaryOutputDir
    {
      get
      {
        return System.IO.Path.GetFullPath(
          System.IO.Path.Combine(ModuleDirectory, "../../../../Binaries/Win64")
        );
      }
    }

    private string ProjectRoot
    {
      get
      {
        return System.IO.Path.GetFullPath(
          System.IO.Path.Combine(ModuleDirectory, "../../../../")
        );
      }
    }

    private string ModulePath
    {
      get { return ModuleDirectory; }
    }

    private string ThirdPartyPath
    {
      get
      {
        return Path.GetFullPath(Path.Combine(ModulePath,
                                            "../../ThirdParty/"));
      }
    }
	
	private string GBSNatsPath
	{
      get
      {
        return Path.GetFullPath(Path.Combine(ModulePath,
                                            "../.."));
      }
	}

    private string NatsPath
    {
      get
      {
        return Path.GetFullPath(Path.Combine(ThirdPartyPath, "nats"));
      }
    }
    
    private string NatsBuildScript
    {
      get
      {
		if (IsUnix)
		{
			return Path.GetFullPath(Path.Combine(NatsPath, "BUILD.GBS.sh"));
		}
		else
		{
			return Path.GetFullPath(Path.Combine(NatsPath, "BUILD.GBS.bat"));
		}
      }
    }

    private string NatsInstallPath
    {
      get
      {
        return Path.GetFullPath(Path.Combine(NatsPath, "Binaries"));
      }
    }

    private string NatsIncludePath
    {
      get
      {
        return Path.GetFullPath(Path.Combine(NatsInstallPath, "include"));
      }
    }

    private string NatsLibraryPath
    {
      get
      {
        return Path.GetFullPath(Path.Combine(NatsInstallPath, "lib"));
      }
    }

    private string ProtobufCDLLPath
    {
      get
      {
        return Path.GetFullPath(Path.Combine(NatsInstallPath, "bin", "protobuf-c.dll"));
      }
    }


    private string NatsDLLPath
    {
      get
      {
        return Path.GetFullPath(Path.Combine(NatsInstallPath, "lib", "nats.dll"));
      }
    }

  }
}
