using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.IO;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace DotaAllstarsExErrorHandler
{

    public partial class DotaErrorHandlerEx : Form
    {
        public DotaErrorHandlerEx()
        {
            InitializeComponent();
        }


        enum LogType : uint
        {
            DotaChatLog,
            DotaHelperLog,
            JassNativesFuncLog,
            JassLogList
        };



        [StructLayout(LayoutKind.Sequential)]
        struct COPYDATASTRUCT
        {
            public uint dwData;    // Any value the sender chooses.  Perhaps its main window handle?
            public int cbData;       // The count of bytes in the message.
            public IntPtr lpData;    // The address of the message.
        }

        const int WM_COPYDATA = 0x4A;

        List<string> lDotaChatLog = new List<string>();
        List<string> lDotaHelperLog = new List<string>();
        List<string> lJassNativesFuncLog = new List<string>();
        List<string> lJassLogList = new List<string>();

        uint cDotaChatLog = 0;
        uint cDotaHelperLog = 0;
        uint cJassNativesFuncLog = 0;
        uint cJassLogList = 0;

        bool bDotaChatLog = false;
        bool bDotaHelperLog = false;
        bool bJassNativesFuncLog = false;
        bool bJassLogList = false;


        protected override void WndProc(ref Message m)
        {

            switch (m.Msg)
            {
                // 
                case WM_COPYDATA:
                    {
                        COPYDATASTRUCT mystr = new COPYDATASTRUCT();

                        mystr = (COPYDATASTRUCT)Marshal.PtrToStructure(m.LParam, typeof(COPYDATASTRUCT));

                        if (mystr.cbData > 0)
                        {
                            byte[] data = new byte[mystr.cbData];
                            Marshal.Copy(mystr.lpData, data, 0, mystr.cbData);
                            string myString = Encoding.UTF8.GetString(data);
                            if (myString.Length > 0)
                            {
                                switch ((LogType)mystr.dwData)
                                {
                                    case LogType.DotaChatLog:
                                        if (lDotaChatLog.Count > 1000)
                                        {
                                            lDotaChatLog.RemoveAt(0);
                                        }
                                        lDotaChatLog.Add(myString);
                                        if (cDotaChatLog + 1 < uint.MaxValue)
                                            cDotaChatLog++;
                                        bDotaChatLog = true;
                                        break;
                                    case LogType.DotaHelperLog:
                                        if (lDotaHelperLog.Count > 1000)
                                        {
                                            lDotaHelperLog.RemoveAt(0);
                                        }
                                        lDotaHelperLog.Add(myString);
                                        if (cDotaHelperLog + 1 < uint.MaxValue)
                                            cDotaHelperLog++;
                                        bDotaHelperLog = true;
                                        break;
                                    case LogType.JassLogList:
                                        if (lJassLogList.Count > 1000)
                                        {
                                            lJassLogList.RemoveAt(0);
                                        }
                                        lJassLogList.Add(myString);
                                        if (cJassLogList + 1 < uint.MaxValue)
                                            cJassLogList++;
                                        bJassLogList = true;
                                        break;
                                    case LogType.JassNativesFuncLog:
                                        if (lJassNativesFuncLog.Count > 1000)
                                        {
                                            lJassNativesFuncLog.RemoveAt(0);
                                        }
                                        lJassNativesFuncLog.Add(myString);
                                        if (cJassNativesFuncLog + 1 < uint.MaxValue)
                                            cJassNativesFuncLog++;
                                        bJassNativesFuncLog = true;
                                        break;
                                }


                            }



                        }
                        break;
                    }
            }
            base.WndProc(ref m);


        }
      

        ListBox DotaChatLog = null;
        ListBox DotaHelperLog = null;
        ListBox JassNativesFuncLog = null;
        ListBox JassLogList = null;

        Label DotaChatLogVal = null;
        Label DotaHelperLogVal = null;
        Label JassNativesFuncLogVal = null;
        Label JassLogListVal = null;

        private void UpdateDotaLog_Tick(object sender, EventArgs e)
        {
            if (bDotaChatLog)
            {
                bDotaChatLog = false;
                DotaChatLog.BeginUpdate();
                DotaChatLog.Items.Clear();
                DotaChatLog.Items.AddRange(lDotaChatLog.ToArray());
                DotaChatLog.EndUpdate();
            }
            if (bDotaHelperLog)
            {
                bDotaHelperLog = false;
                DotaHelperLog.BeginUpdate();
                DotaHelperLog.Items.Clear();
                DotaHelperLog.Items.AddRange(lDotaHelperLog.ToArray());
                DotaHelperLog.EndUpdate();
            }
            if (bJassNativesFuncLog)
            {
                bJassNativesFuncLog = false;
                JassNativesFuncLog.BeginUpdate();
                JassNativesFuncLog.Items.Clear();
                JassNativesFuncLog.Items.AddRange(lJassNativesFuncLog.ToArray());
                JassNativesFuncLog.EndUpdate();
            }
            if (bJassLogList)
            {
                bJassLogList = false;
                JassLogList.BeginUpdate();
                JassLogList.Items.Clear();
                JassLogList.Items.AddRange(lJassLogList.ToArray());
                JassLogList.EndUpdate();
            }

            if (DotaChatLogVal != null)
                DotaChatLogVal.Text = "DotaChatLog:" + cDotaChatLog.ToString();
            if (DotaHelperLogVal != null)
                DotaHelperLogVal.Text = "DotaHelperLog:" + cDotaHelperLog.ToString();
            if (JassNativesFuncLogVal != null)
                JassNativesFuncLogVal.Text = "JassNativesFuncLog:" + cJassNativesFuncLog.ToString();
            if (JassLogListVal != null)
                JassLogListVal.Text = "JassLogList:" + cJassLogList.ToString();

        }

        private void SaveButton_Click(object sender, EventArgs e)
        {
            File.Open(SaveFileName.Text, FileMode.Create).Close();
            File.AppendAllText(SaveFileName.Text, "DotaChatLog:\n");
            File.AppendAllLines(SaveFileName.Text, DotaChatLog.Items.Cast<String>().ToList());
            File.AppendAllText(SaveFileName.Text, "DotaHelperLog:\n");
            File.AppendAllLines(SaveFileName.Text, DotaHelperLog.Items.Cast<String>().ToList());
            File.AppendAllText(SaveFileName.Text, "JassNativesFuncLog:\n");
            File.AppendAllLines(SaveFileName.Text, JassNativesFuncLog.Items.Cast<String>().ToList());
            File.AppendAllText(SaveFileName.Text, "JassLogList:\n");
            File.AppendAllLines(SaveFileName.Text, JassLogList.Items.Cast<String>().ToList());

        }
        private void Form1_Load(object sender, EventArgs e)
        {
            DotaChatLogWindow pDotaChatLogWindow = new DotaChatLogWindow(ref DotaChatLogVal, ref DotaChatLog);
            DotaHelperLogWindow pDotaHelperLogWindow = new DotaHelperLogWindow(ref DotaHelperLogVal, ref DotaHelperLog);
            JassLogListWindow pJassLogListWindow = new JassLogListWindow(ref JassLogListVal, ref JassLogList);
            JassNativesFuncLogWindow pJassNativesFuncLogWindow = new JassNativesFuncLogWindow(ref JassNativesFuncLogVal, ref JassNativesFuncLog);


            pDotaChatLogWindow.Show();
            pDotaHelperLogWindow.Show();
            pJassLogListWindow.Show();
            pJassNativesFuncLogWindow.Show();

            this.Top /= 2;

            pDotaChatLogWindow.Left = Left - pDotaChatLogWindow.Width;
            pDotaChatLogWindow.Top = Bottom - Height;

            pDotaHelperLogWindow.Left = Right;
            pDotaHelperLogWindow.Top = Bottom - Height;

            pJassLogListWindow.Left = Left - pJassLogListWindow.Width;
            pJassLogListWindow.Top = Bottom + pJassLogListWindow.Height - Height;

            pJassNativesFuncLogWindow.Left = Right;
            pJassNativesFuncLogWindow.Top = Bottom + pJassNativesFuncLogWindow.Height - Height;

          

        }
    }
}
