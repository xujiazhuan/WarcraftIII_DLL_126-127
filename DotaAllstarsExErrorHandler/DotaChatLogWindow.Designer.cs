namespace DotaAllstarsExErrorHandler
{
    partial class DotaChatLogWindow
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
            this.DotaChatLogList = new System.Windows.Forms.ListBox();
            this.DotaChatLogUpdateTimer = new System.Windows.Forms.Timer(this.components);
            this.SuspendLayout();
            // 
            // DotaChatLogList
            // 
            this.DotaChatLogList.Dock = System.Windows.Forms.DockStyle.Fill;
            this.DotaChatLogList.FormattingEnabled = true;
            this.DotaChatLogList.Location = new System.Drawing.Point(0, 0);
            this.DotaChatLogList.Name = "DotaChatLogList";
            this.DotaChatLogList.Size = new System.Drawing.Size(284, 262);
            this.DotaChatLogList.TabIndex = 0;
            // 
            // DotaChatLogUpdateTimer
            // 
            this.DotaChatLogUpdateTimer.Enabled = true;
            this.DotaChatLogUpdateTimer.Interval = 200;
            this.DotaChatLogUpdateTimer.Tick += new System.EventHandler(this.DotaChatLogUpdateTimer_Tick);
            // 
            // DotaChatLogWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 262);
            this.Controls.Add(this.DotaChatLogList);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
            this.MaximumSize = new System.Drawing.Size(300, 2000);
            this.MinimumSize = new System.Drawing.Size(300, 296);
            this.Name = "DotaChatLogWindow";
            this.StartPosition = System.Windows.Forms.FormStartPosition.Manual;
            this.Text = "DotaChatLogWindow";
            this.Load += new System.EventHandler(this.DotaChatLogWindow_Load);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListBox DotaChatLogList;
        private System.Windows.Forms.Timer DotaChatLogUpdateTimer;
    }
}