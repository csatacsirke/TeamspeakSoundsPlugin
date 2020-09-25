using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

using System.IO.Compression;
using System.IO;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace PluginInstaller {
    static class PluginPackager {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args) {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            //Application.Run(new Form1());

            if (args.Length < 2) {

                Console.WriteLine("Nem adtál meg source filet te hülye! %0 :  sourceFile");
                return;
            }

            string sourceFile = args[0];
            string changelogPath = args[1];
            try {
                CreateTsPluginInstallerFile(sourceFile, changelogPath);
            } catch (Exception e) {
                Console.WriteLine("Fatal error");
                Console.WriteLine(e.ToString());
            }

        }

        static string RunCommand(string path, string arguments) {
            Process process = new Process() {
                StartInfo = new ProcessStartInfo {
                    FileName = path,
                    Arguments = arguments,
                    UseShellExecute = false,
                    RedirectStandardOutput = true,
                }
            };
               
    

            process.Start();
            //* Read the output (or the error)
            string output = process.StandardOutput.ReadToEnd();
            process.WaitForExit();

            return output;
        }

        static void CreatePackageIni(ZipArchive zip, string version) {

            var packageIniEntry = zip.CreateEntry("package.ini");
            var stream = packageIniEntry.Open();

            var writer = new StreamWriter(stream, System.Text.Encoding.UTF8);

            //var version = DateTime.Now.ToString("yy.MM.dd");


           var packageIniContents = $@"
Name = Battlechicken's soundplayer plugin
Type = Plugin
Author = Battlechicken
Version = {version}
Platforms = win64
Description = ""Just another soundboard""
";

            writer.Write(packageIniContents);
            writer.Flush();

            stream.Flush();
            stream.Close();
        }


        static void CreateTsPluginInstallerFile(string sourceFilePath, string changelogPath) {

            string directory = Path.GetDirectoryName(sourceFilePath);
            string sourceFileName = Path.GetFileName(sourceFilePath);

            string baseFileName = Path.GetFileNameWithoutExtension(sourceFileName);

            string outputFileName = Path.ChangeExtension(sourceFileName, "ts3_plugin");
            string outputFilePath = Path.Combine(directory, outputFileName);


            var version = RunCommand("TsVersionFinder.exe", sourceFilePath);

            
            if (File.Exists(outputFilePath)) {
                File.Delete(outputFilePath);
            }

            ZipArchive zip = ZipFile.Open(outputFilePath, ZipArchiveMode.Create);

            zip.CreateEntryFromFile(sourceFilePath, "plugins/" + sourceFileName);
            zip.CreateEntryFromFile(changelogPath, "changelog.json");

            CreatePackageIni(zip, version);


            Console.WriteLine("Output: " + outputFilePath);
            zip.Dispose();

        }
    }
}

