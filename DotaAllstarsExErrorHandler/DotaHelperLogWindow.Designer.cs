namespace DotaAllstarsExErrorHandler
{
    partial class DotaHelperLogWindow
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
            this.DotaHelperLogList = new System.Windows.Forms.ListBox();
            this.DotaHelperLogTimer = new System.Windows.Forms.Timer(this.components);
            this.SuspendLayout();
            // 
            // DotaHelperLogList
            // 
            this.DotaHelperLogList.Dock = System.Windows.Forms.DockStyle.Fill;
            this.DotaHelperLogList.FormattingEnabled = true;
            this.DotaHelperLogList.Location = new System.Drawing.Point(0, 0);
            this.DotaHelperLogList.Name = "DotaHelperLogList";
            this.DotaHelperLogList.Size = new System.Drawing.Size(284, 262);
            this.DotaHelperLogList.TabIndex = 0;
            // 
            // DotaHelperLogTimer
            // 
            this.DotaHelperLogTimer.Enabled = true;
            this.DotaHelperLogTimer.Interval = 200;
            this.DotaHelperLogTimer.Tick += new System.EventHandler(this.DotaHelperLogTimer_Tick);
            // 
            // DotaHelperLogWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 262);
            this.Controls.Add(this.DotaHelperLogList);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
            this.MaximumSize = new System.Drawing.Size(300, 2000);
            this.MinimumSize = new System.Drawing.Size(300, 296);
            this.Name = "DotaHelperLogWindow";
            this.StartPosition = System.Windows.Forms.FormStartPosition.Manual;
            this.Text = "DotaHelperLogWindow";
            this.Load += new System.EventHandler(this.DotaHelperLogWindow_Load);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListBox DotaHelperLogList;
        private System.Windows.Forms.Timer DotaHelperLogTimer;
    }
}