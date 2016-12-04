namespace DotaAllstarsExErrorHandler
{
    partial class DotaErrorHandlerEx
    {
        /// <summary>
        /// Обязательная переменная конструктора.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Освободить все используемые ресурсы.
        /// </summary>
        /// <param name="disposing">истинно, если управляемый ресурс должен быть удален; иначе ложно.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Код, автоматически созданный конструктором форм Windows

        /// <summary>
        /// Требуемый метод для поддержки конструктора — не изменяйте 
        /// содержимое этого метода с помощью редактора кода.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.SaveButton = new System.Windows.Forms.Button();
            this.UpdateDotaLog = new System.Windows.Forms.Timer(this.components);
            this.SaveFileName = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // SaveButton
            // 
            this.SaveButton.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.SaveButton.Location = new System.Drawing.Point(0, 22);
            this.SaveButton.Name = "SaveButton";
            this.SaveButton.Size = new System.Drawing.Size(323, 23);
            this.SaveButton.TabIndex = 1;
            this.SaveButton.Text = "Save to file";
            this.SaveButton.UseVisualStyleBackColor = true;
            this.SaveButton.Click += new System.EventHandler(this.SaveButton_Click);
            // 
            // UpdateDotaLog
            // 
            this.UpdateDotaLog.Enabled = true;
            this.UpdateDotaLog.Interval = 200;
            this.UpdateDotaLog.Tick += new System.EventHandler(this.UpdateDotaLog_Tick);
            // 
            // SaveFileName
            // 
            this.SaveFileName.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.SaveFileName.Location = new System.Drawing.Point(0, 2);
            this.SaveFileName.Name = "SaveFileName";
            this.SaveFileName.Size = new System.Drawing.Size(323, 20);
            this.SaveFileName.TabIndex = 2;
            this.SaveFileName.Text = "DotaAllstarsLogDump.txt";
            // 
            // DotaErrorHandlerEx
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoSize = true;
            this.ClientSize = new System.Drawing.Size(323, 45);
            this.Controls.Add(this.SaveFileName);
            this.Controls.Add(this.SaveButton);
            this.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.MaximumSize = new System.Drawing.Size(339, 79);
            this.MinimumSize = new System.Drawing.Size(339, 79);
            this.Name = "DotaErrorHandlerEx";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Dota Allstars Extern Error Handler";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Button SaveButton;
        private System.Windows.Forms.Timer UpdateDotaLog;
        private System.Windows.Forms.TextBox SaveFileName;
    }
}

