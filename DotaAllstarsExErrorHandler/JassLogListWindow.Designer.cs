namespace DotaAllstarsExErrorHandler
{
    partial class JassLogListWindow
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
            this.JassLogListList = new System.Windows.Forms.ListBox();
            this.JassLogListTimer = new System.Windows.Forms.Timer(this.components);
            this.SuspendLayout();
            // 
            // JassLogListList
            // 
            this.JassLogListList.Dock = System.Windows.Forms.DockStyle.Fill;
            this.JassLogListList.FormattingEnabled = true;
            this.JassLogListList.Location = new System.Drawing.Point(0, 0);
            this.JassLogListList.Name = "JassLogListList";
            this.JassLogListList.Size = new System.Drawing.Size(284, 262);
            this.JassLogListList.TabIndex = 0;
            // 
            // JassLogListTimer
            // 
            this.JassLogListTimer.Enabled = true;
            this.JassLogListTimer.Interval = 200;
            this.JassLogListTimer.Tick += new System.EventHandler(this.JassLogListTimer_Tick);
            // 
            // JassLogListWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 262);
            this.Controls.Add(this.JassLogListList);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
            this.MaximumSize = new System.Drawing.Size(300, 2000);
            this.MinimumSize = new System.Drawing.Size(300, 296);
            this.Name = "JassLogListWindow";
            this.StartPosition = System.Windows.Forms.FormStartPosition.Manual;
            this.Text = "JassLogListWindow";
            this.Load += new System.EventHandler(this.JassLogListWindow_Load);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListBox JassLogListList;
        private System.Windows.Forms.Timer JassLogListTimer;
    }
}