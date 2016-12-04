namespace DotaAllstarsExErrorHandler
{
    partial class JassNativesFuncLogWindow
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.JassNativesFuncLogList = new System.Windows.Forms.ListBox();
            this.JassNativesFuncLogTimer = new System.Windows.Forms.Timer(this.components);
            this.SuspendLayout();
            // 
            // JassNativesFuncLogList
            // 
            this.JassNativesFuncLogList.Dock = System.Windows.Forms.DockStyle.Fill;
            this.JassNativesFuncLogList.FormattingEnabled = true;
            this.JassNativesFuncLogList.Location = new System.Drawing.Point(0, 0);
            this.JassNativesFuncLogList.Name = "JassNativesFuncLogList";
            this.JassNativesFuncLogList.Size = new System.Drawing.Size(284, 262);
            this.JassNativesFuncLogList.TabIndex = 0;
            // 
            // JassNativesFuncLogTimer
            // 
            this.JassNativesFuncLogTimer.Enabled = true;
            this.JassNativesFuncLogTimer.Interval = 200;
            this.JassNativesFuncLogTimer.Tick += new System.EventHandler(this.JassNativesFuncLogTimer_Tick);
            // 
            // JassNativesFuncLogWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 262);
            this.Controls.Add(this.JassNativesFuncLogList);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
            this.MaximumSize = new System.Drawing.Size(300, 2000);
            this.MinimumSize = new System.Drawing.Size(300, 296);
            this.Name = "JassNativesFuncLogWindow";
            this.StartPosition = System.Windows.Forms.FormStartPosition.Manual;
            this.Text = "JassNativesFuncLogWindow";
            this.Load += new System.EventHandler(this.JassNativesFuncLogWindow_Load);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListBox JassNativesFuncLogList;
        private System.Windows.Forms.Timer JassNativesFuncLogTimer;
    }
}