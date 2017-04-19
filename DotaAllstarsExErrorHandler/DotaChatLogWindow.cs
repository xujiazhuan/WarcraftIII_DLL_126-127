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
    public partial class DotaChatLogWindow : Form
    {
        public DotaChatLogWindow()
        {
            InitializeComponent();
        }

        Label thistext = new Label( );

        public DotaChatLogWindow(ref Label s, ref ListBox l)
        {
            InitializeComponent();
            s = thistext;
            l = DotaChatLogList;
        }

        private void DotaChatLogWindow_Load(object sender, EventArgs e)
        {

        }

        private void DotaChatLogUpdateTimer_Tick(object sender, EventArgs e)
        {
            if (thistext.Text != string.Empty)
            {
                this.Text = thistext.Text;
                this.Update();
            }

        }
    }
}
