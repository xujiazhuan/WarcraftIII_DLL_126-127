using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace DotaAllstarsExErrorHandler
{
    public partial class JassNativesFuncLogWindow : Form
    {
        public JassNativesFuncLogWindow()
        {
            InitializeComponent();
        }

        Label thistext = new Label();

        public JassNativesFuncLogWindow(ref Label s, ref ListBox l)
        {
            InitializeComponent();
            s = thistext;
            l = JassNativesFuncLogList;
        }

        private void JassNativesFuncLogWindow_Load(object sender, EventArgs e)
        {

        }

        private void JassNativesFuncLogTimer_Tick(object sender, EventArgs e)
        {
            if (thistext.Text != string.Empty)
            {
                this.Text = thistext.Text;
                this.Update();
            }

        }
    }
}
