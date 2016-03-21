using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace PluginInstaller {
    public partial class Form1 : Form {
        public Form1() {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e) {
            //FolderBrowserDialog dialog = new FolderBrowserDialog();
            SaveFileDialog dialog = new SaveFileDialog();
            var result = dialog.ShowDialog();
            if (result == DialogResult.OK) {
                //pathEditBox.Text = dialog.FileName;
                pathEditBox.Text = dialog.FileName;
            }

            //pathEditBox.
        }

        private void textBox1_TextChanged(object sender, EventArgs e) {

        }
    }
}
