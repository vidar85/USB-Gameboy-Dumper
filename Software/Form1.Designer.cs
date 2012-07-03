namespace Dumper
{
    partial class Form1
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.Banks_l = new System.Windows.Forms.Label();
            this.MBC_l = new System.Windows.Forms.Label();
            this.RAM_l = new System.Windows.Forms.Label();
            this.Size_l = new System.Windows.Forms.Label();
            this.Title_l = new System.Windows.Forms.Label();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.WriteRAM_b = new System.Windows.Forms.Button();
            this.DumpRAM_b = new System.Windows.Forms.Button();
            this.DumpROM_b = new System.Windows.Forms.Button();
            this.Scan_b = new System.Windows.Forms.Button();
            this.progressBar1 = new System.Windows.Forms.ProgressBar();
            this.Device_l = new System.Windows.Forms.Label();
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            this.saveFileDialogROM = new System.Windows.Forms.SaveFileDialog();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.saveFileDialogRAM = new System.Windows.Forms.SaveFileDialog();
            this.backgroundWorker1 = new System.ComponentModel.BackgroundWorker();
            this.label1 = new System.Windows.Forms.Label();
            this.button1 = new System.Windows.Forms.Button();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.Banks_l);
            this.groupBox1.Controls.Add(this.MBC_l);
            this.groupBox1.Controls.Add(this.RAM_l);
            this.groupBox1.Controls.Add(this.Size_l);
            this.groupBox1.Controls.Add(this.Title_l);
            this.groupBox1.Location = new System.Drawing.Point(12, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(150, 144);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Game Info";
            // 
            // Banks_l
            // 
            this.Banks_l.AutoSize = true;
            this.Banks_l.Location = new System.Drawing.Point(7, 120);
            this.Banks_l.Name = "Banks_l";
            this.Banks_l.Size = new System.Drawing.Size(43, 13);
            this.Banks_l.TabIndex = 4;
            this.Banks_l.Text = "Banks: ";
            // 
            // MBC_l
            // 
            this.MBC_l.AutoSize = true;
            this.MBC_l.Location = new System.Drawing.Point(7, 95);
            this.MBC_l.Name = "MBC_l";
            this.MBC_l.Size = new System.Drawing.Size(36, 13);
            this.MBC_l.TabIndex = 3;
            this.MBC_l.Text = "MBC: ";
            // 
            // RAM_l
            // 
            this.RAM_l.AutoSize = true;
            this.RAM_l.Location = new System.Drawing.Point(7, 70);
            this.RAM_l.Name = "RAM_l";
            this.RAM_l.Size = new System.Drawing.Size(60, 13);
            this.RAM_l.TabIndex = 2;
            this.RAM_l.Text = "RAM Size: ";
            // 
            // Size_l
            // 
            this.Size_l.AutoSize = true;
            this.Size_l.Location = new System.Drawing.Point(7, 45);
            this.Size_l.Name = "Size_l";
            this.Size_l.Size = new System.Drawing.Size(30, 13);
            this.Size_l.TabIndex = 1;
            this.Size_l.Text = "Size:";
            // 
            // Title_l
            // 
            this.Title_l.AutoSize = true;
            this.Title_l.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Title_l.Location = new System.Drawing.Point(7, 20);
            this.Title_l.Name = "Title_l";
            this.Title_l.Size = new System.Drawing.Size(34, 13);
            this.Title_l.TabIndex = 0;
            this.Title_l.Text = "Title: ";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.WriteRAM_b);
            this.groupBox2.Controls.Add(this.DumpRAM_b);
            this.groupBox2.Controls.Add(this.DumpROM_b);
            this.groupBox2.Controls.Add(this.Scan_b);
            this.groupBox2.Location = new System.Drawing.Point(168, 13);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(120, 143);
            this.groupBox2.TabIndex = 1;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Tools";
            // 
            // WriteRAM_b
            // 
            this.WriteRAM_b.Enabled = false;
            this.WriteRAM_b.Location = new System.Drawing.Point(7, 110);
            this.WriteRAM_b.Name = "WriteRAM_b";
            this.WriteRAM_b.Size = new System.Drawing.Size(107, 23);
            this.WriteRAM_b.TabIndex = 3;
            this.WriteRAM_b.Text = "Write RAM";
            this.WriteRAM_b.UseVisualStyleBackColor = true;
            this.WriteRAM_b.Click += new System.EventHandler(this.WriteRAM_b_Click);
            // 
            // DumpRAM_b
            // 
            this.DumpRAM_b.Enabled = false;
            this.DumpRAM_b.Location = new System.Drawing.Point(7, 80);
            this.DumpRAM_b.Name = "DumpRAM_b";
            this.DumpRAM_b.Size = new System.Drawing.Size(107, 23);
            this.DumpRAM_b.TabIndex = 2;
            this.DumpRAM_b.Text = "Dump RAM";
            this.DumpRAM_b.UseVisualStyleBackColor = true;
            this.DumpRAM_b.Click += new System.EventHandler(this.DumpRAM_b_Click);
            // 
            // DumpROM_b
            // 
            this.DumpROM_b.Enabled = false;
            this.DumpROM_b.Location = new System.Drawing.Point(7, 50);
            this.DumpROM_b.Name = "DumpROM_b";
            this.DumpROM_b.Size = new System.Drawing.Size(107, 23);
            this.DumpROM_b.TabIndex = 1;
            this.DumpROM_b.Text = "Dump ROM";
            this.DumpROM_b.UseVisualStyleBackColor = true;
            this.DumpROM_b.Click += new System.EventHandler(this.DumpROM_b_Click);
            // 
            // Scan_b
            // 
            this.Scan_b.Enabled = false;
            this.Scan_b.Location = new System.Drawing.Point(7, 20);
            this.Scan_b.Name = "Scan_b";
            this.Scan_b.Size = new System.Drawing.Size(107, 23);
            this.Scan_b.TabIndex = 0;
            this.Scan_b.Text = "Scan";
            this.Scan_b.UseVisualStyleBackColor = true;
            this.Scan_b.Click += new System.EventHandler(this.Scan_b_Click);
            this.Scan_b.KeyDown += new System.Windows.Forms.KeyEventHandler(this.Scan_b_KeyDown);
            this.Scan_b.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.Scan_b_KeyPress);
            // 
            // progressBar1
            // 
            this.progressBar1.Location = new System.Drawing.Point(12, 162);
            this.progressBar1.Name = "progressBar1";
            this.progressBar1.Size = new System.Drawing.Size(276, 23);
            this.progressBar1.TabIndex = 2;
            // 
            // Device_l
            // 
            this.Device_l.AutoSize = true;
            this.Device_l.Location = new System.Drawing.Point(12, 188);
            this.Device_l.Name = "Device_l";
            this.Device_l.Size = new System.Drawing.Size(119, 13);
            this.Device_l.TabIndex = 3;
            this.Device_l.Text = "Device: Not Connected";
            // 
            // timer1
            // 
            this.timer1.Interval = 750;
            this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
            // 
            // saveFileDialogROM
            // 
            this.saveFileDialogROM.FileName = "dfgdg";
            this.saveFileDialogROM.Filter = "Gameboy|*.gb|All files|*.*";
            this.saveFileDialogROM.FileOk += new System.ComponentModel.CancelEventHandler(this.saveFileDialogROM_FileOk);
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.FileName = "openFileDialog1";
            this.openFileDialog1.Filter = "Gameboy save|*.sav|All files|*.*";
            this.openFileDialog1.FileOk += new System.ComponentModel.CancelEventHandler(this.openFileDialog1_FileOk);
            // 
            // saveFileDialogRAM
            // 
            this.saveFileDialogRAM.Filter = "Gameboy save|*.sav|All files|*.*";
            this.saveFileDialogRAM.FileOk += new System.ComponentModel.CancelEventHandler(this.saveFileDialogRAM_FileOk);
            // 
            // backgroundWorker1
            // 
            this.backgroundWorker1.WorkerReportsProgress = true;
            this.backgroundWorker1.DoWork += new System.ComponentModel.DoWorkEventHandler(this.backgroundWorker1_DoWork);
            this.backgroundWorker1.ProgressChanged += new System.ComponentModel.ProgressChangedEventHandler(this.backgroundWorker1_ProgressChanged);
            this.backgroundWorker1.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.backgroundWorker1_RunWorkerCompleted);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(209, 188);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(79, 13);
            this.label1.TabIndex = 5;
            this.label1.Text = "Rival-Corp.com";
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(12, 204);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(106, 23);
            this.button1.TabIndex = 6;
            this.button1.Text = "Boot Bootloader";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click_3);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(297, 232);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.Device_l);
            this.Controls.Add(this.progressBar1);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MinimumSize = new System.Drawing.Size(306, 245);
            this.Name = "Form1";
            this.Text = "USB Gameboy Dumper";
            this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.Form1_KeyDown_1);
            this.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.Form1_KeyPress);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label Banks_l;
        private System.Windows.Forms.Label MBC_l;
        private System.Windows.Forms.Label RAM_l;
        private System.Windows.Forms.Label Size_l;
        private System.Windows.Forms.Label Title_l;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Button WriteRAM_b;
        private System.Windows.Forms.Button DumpRAM_b;
        private System.Windows.Forms.Button DumpROM_b;
        private System.Windows.Forms.Button Scan_b;
        private System.Windows.Forms.Label Device_l;
        private System.Windows.Forms.Timer timer1;
        private System.Windows.Forms.SaveFileDialog saveFileDialogROM;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.SaveFileDialog saveFileDialogRAM;
        public System.Windows.Forms.ProgressBar progressBar1;
        private System.ComponentModel.BackgroundWorker backgroundWorker1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button button1;
    }
}

