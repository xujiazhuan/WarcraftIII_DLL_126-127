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
    public partial class JassLogListWindow : Form
    {
        public JassLogListWindow()
        {
            InitializeComponent();
        }

        Label thistext = new Label();

        public JassLogListWindow(ref Label s, ref ListBox l)
        {
            InitializeComponent();
            s = thistext;
            l = JassLogListList;
        }

        private void JassLogListWindow_Load(object sender, EventArgs e)
        {

        }

        private void JassLogListTimer_Tick(object sender, EventArgs e)
        {
            if (thistext.Text != string.Empty)
            {
                this.Text = thistext.Text;
                this.Update();
            }

        }
    }
}
