using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

using System.IO.Compression;
using System.IO;

namespace PluginInstaller {
    static class Program {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args) {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            
            //Application.Run(new Form1());

            if (args.Length < 1)
            {
                
                Console.WriteLine("Nem adtál meg source filet te hülye! %0 :  sourceFile");
                return;
            }

            string sourceFile = args[0];
            try
            {
                CreateTsPluginInstallerFile(sourceFile);
            } catch(Exception e)
            {
                Console.WriteLine("Fatal error");
                Console.WriteLine(e.ToString());
            }
            
        }


        static void CreatePackageIni(ZipArchive zip) {

            var packageIniEntry = zip.CreateEntry("package.ini");
            var stream = packageIniEntry.Open();

            var writer = new StreamWriter(stream, System.Text.Encoding.UTF8);

            var packageIniContents = @"
Name = Battlechicken's soundplayer plugin
Type = Plugin
Author = Battlechicken
Version = 1.0
Platforms = win64
Description = ""Just another soundboard""
";

            writer.Write(packageIniContents);
            writer.Flush();

            stream.Flush();
            stream.Close();
        }

        static void CreateTsPluginInstallerFile(string sourceFilePath)
        {
            string directory = Path.GetDirectoryName(sourceFilePath);
            string sourceFileName = Path.GetFileName(sourceFilePath);

            string baseFileName = Path.GetFileNameWithoutExtension(sourceFileName);

            string outputFileName = Path.ChangeExtension(sourceFileName, "ts3_plugin");
            string outputFilePath = Path.Combine(directory, outputFileName);

            //string outputFilePath = sourceFilePath + ".ts3_plugin";

            if (File.Exists(outputFilePath)) {
                File.Delete(outputFilePath);
            }

            //FileStream outputStream = new FileStream(outputFileName, FileMode.CreateNew);

            ZipArchive zip = ZipFile.Open(outputFilePath, ZipArchiveMode.Create);
            //ZipArchive zip = new ZipArchive(outputStream);
            //ZipArchive zip = ZipFile.
            zip.CreateEntryFromFile(sourceFilePath, "plugins/" + sourceFileName);

            CreatePackageIni(zip);

            //zip.CreateEntryFromFile("package.ini", "package.ini");


            //ZipArchiveEntry package_ini = zip.CreateEntry("package.ini")
            //Stream package_ini_stream = package_ini.Open();
            //StreamWriter package_ini_streamWriter = new StreamWriter(package_ini_stream);
            //package_ini_streamWriter.WriteLine("Teszt 1");
            //package_ini_streamWriter.Flush();


            //outputStream.Flush();
            Console.WriteLine("Output: " + outputFilePath);
            zip.Dispose();

        }
    }
}
