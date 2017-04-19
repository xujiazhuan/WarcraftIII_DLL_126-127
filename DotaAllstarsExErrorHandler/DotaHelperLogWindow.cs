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
    public partial class DotaHelperLogWindow : Form
    {
        public DotaHelperLogWindow()
        {
            InitializeComponent();
        }

        Label thistext = new Label( );

        public DotaHelperLogWindow(ref Label s, ref ListBox l)
        {
            InitializeComponent();
            s = thistext;
            l = DotaHelperLogList;
        }


        private void DotaHelperLogWindow_Load(object sender, EventArgs e)
        {

        }

        private void DotaHelperLogTimer_Tick(object sender, EventArgs e)
        {
            if (thistext.Text != string.Empty)
            {
                this.Text = thistext.Text;
                this.Update();
            }

        }
    }
}
