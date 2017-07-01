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
                
                Console.WriteLine("%0 :  sourceFile");
                return;
            }

            string sourceFile = args[0];
            try
            {
                CreateTsPluginInstallerFile(sourceFile);
            } catch(Exception e)
            {
                Console.WriteLine(e.ToString());
                Console.ReadKey();
            }
            
        }

        static void CreateTsPluginInstallerFile(string sourceFile)
        {
            string outputFileName = sourceFile + ".ts3_plugin";
            if (File.Exists(outputFileName)) File.Delete(outputFileName);

            //FileStream outputStream = new FileStream(outputFileName, FileMode.CreateNew);

            ZipArchive zip = ZipFile.Open(outputFileName, ZipArchiveMode.Create);
            //ZipArchive zip = new ZipArchive(outputStream);
            //ZipArchive zip = ZipFile.
            zip.CreateEntryFromFile(sourceFile, "plugins/" + "Soundplayer.dll");

            zip.CreateEntryFromFile("package.ini", "package.ini");

            //ZipArchiveEntry package_ini = zip.CreateEntry("package.ini")
            //Stream package_ini_stream = package_ini.Open();
            //StreamWriter package_ini_streamWriter = new StreamWriter(package_ini_stream);
            //package_ini_streamWriter.WriteLine("Teszt 1");
            //package_ini_streamWriter.Flush();
            



            //outputStream.Flush();
            Console.WriteLine("Output: " + outputFileName);
            zip.Dispose();

        }
    }
}
