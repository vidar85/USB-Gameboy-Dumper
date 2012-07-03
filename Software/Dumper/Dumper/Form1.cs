using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using LibUsbDotNet;
using LibUsbDotNet.Main;
using LibUsbDotNet.DeviceNotify;
using System.Diagnostics;
using System.Threading;


namespace Dumper
{
    public partial class Form1 : Form
    {
        /* USB shit */
        public static UsbDevice MyUsbDevice;
        public static int actual_length = 0;
        public static UsbDeviceFinder MyUsbFinder = new UsbDeviceFinder(0x16C0, 0x05DD);
        public static IDeviceNotifier UsbDeviceNotifier = DeviceNotifier.OpenDeviceNotifier();
        public static ErrorCode ec = ErrorCode.None;
        public static IUsbDevice wholeUsbDevice;
        public static UsbEndpointReader reader;
        public static UsbEndpointWriter writer;
        public static byte[] usb_command, rom, RAM;
        public static int rom_size, ram_size;
        public static int debug = 0;
        public static int banks;
        public static string ROM_Title = "";
        public static int mode = 0;
        public static bool error = false;
        public static int help = 0;
        public Stopwatch stopwatch;
        public bool nonNumberEntered = false;
        


        /* Other crap */
        public static bool connected = false;

        public Form1()
        {
            InitializeComponent();
            // Hook the device notifier event
            UsbDeviceNotifier.OnDeviceNotify += OnDeviceNotifyEvent;
            
            usb_command = new byte[64];
            try
            {
                MyUsbDevice = UsbDevice.OpenUsbDevice(MyUsbFinder);
                if (MyUsbDevice == null)
                {
                    //throw new Exception("Device Not Found.");
                    connected = false;
                }
                else
                {

                    Device_l.Text = "Device: Connected";
                    connected = true;

                    Scan_b.Enabled = true;

                    wholeUsbDevice = MyUsbDevice as IUsbDevice;
                    if (!ReferenceEquals(wholeUsbDevice, null))
                    {
                        // This is a "whole" USB device. Before it can be used, 
                        // the desired configuration and interface must be selected.

                        // Select config #1
                        wholeUsbDevice.SetConfiguration(1);

                        // Claim interface #0.
                        wholeUsbDevice.ClaimInterface(0);
                    }
                    //MessageBox.Show(ReadEndpointID.Ep04.ToString());
                    reader = MyUsbDevice.OpenEndpointReader(ReadEndpointID.Ep01);
                    writer = MyUsbDevice.OpenEndpointWriter(WriteEndpointID.Ep01);
                    mode = 4;
                    backgroundWorker1.RunWorkerAsync();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show((ec != ErrorCode.None ? ec + ":" : String.Empty) + ex.Message);
            }
             
        }


        public void OnDeviceNotifyEvent(object sender, DeviceNotifyEventArgs e)
        {
            // A Device system-level event has occured

            //Console.SetCursorPosition(0, Console.CursorTop);
            //MessageBox.Show(e.Device.IdVendor.ToString());
            if (e.EventType == EventType.DeviceArrival && e.Device.IdVendor == 0x16C0 && e.Device.IdProduct == 0x05DD)
            {
                try
                {
                    MyUsbDevice = UsbDevice.OpenUsbDevice(MyUsbFinder);
                    if (MyUsbDevice == null)
                    {
                        //throw new Exception("Device Not Found.");
                        connected = false;
                    }
                    else
                    {
                        
                        Device_l.Text = "Device: Connected";
                        connected = true;

                        Scan_b.Enabled = true;

                        wholeUsbDevice = MyUsbDevice as IUsbDevice;
                        if (!ReferenceEquals(wholeUsbDevice, null))
                        {
                            // This is a "whole" USB device. Before it can be used, 
                            // the desired configuration and interface must be selected.

                            // Select config #1
                            wholeUsbDevice.SetConfiguration(1);

                            // Claim interface #0.
                            wholeUsbDevice.ClaimInterface(0);
                        }
                        //MessageBox.Show(ReadEndpointID.Ep04.ToString());
                        reader = MyUsbDevice.OpenEndpointReader(ReadEndpointID.Ep01);
                        writer = MyUsbDevice.OpenEndpointWriter(WriteEndpointID.Ep01);
                        mode = 4;
                        backgroundWorker1.RunWorkerAsync();
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show((ec != ErrorCode.None ? ec + ":" : String.Empty) + ex.Message);
                }
            }
            if (e.EventType == EventType.DeviceRemoveComplete && e.Device.IdVendor == 0x16C0 && e.Device.IdProduct == 0x05DD)
            {
                timer1.Enabled = false;
                connected = false;
                if (MyUsbDevice != null)
                {
                    if (MyUsbDevice.IsOpen)
                    {
                        // If this is a "whole" usb device (libusb-win32, linux libusb-1.0)
                        // it exposes an IUsbDevice interface. If not (WinUSB) the 
                        // 'wholeUsbDevice' variable will be null indicating this is 
                        // an interface of a device; it does not require or support 
                        // configuration and interface selection.
                        IUsbDevice wholeUsbDevice = MyUsbDevice as IUsbDevice;
                        if (!ReferenceEquals(wholeUsbDevice, null))
                        {
                            // Release interface #0.
                            wholeUsbDevice.ReleaseInterface(0);
                        }

                        MyUsbDevice.Close();
                    }
                    MyUsbDevice = null;

                    // Free usb resources
                    UsbDevice.Exit();
                    Device_l.Text = "Device: Not Connected";
                    Scan_b.Enabled = false;
                    DumpRAM_b.Enabled = false;
                    DumpROM_b.Enabled = false;
                    WriteRAM_b.Enabled = false;
                    Banks_l.Text = "Banks: ";
                    MBC_l.Text = "MBC: ";
                    RAM_l.Text = "RAM Size: ";
                    Size_l.Text = "Size:";
                    Title_l.Text = "Title:";
                    
                }
            }
           // Console.WriteLine(e.ToString()); // Dump the event info to output.

            //Console.WriteLine();
            //Console.Write("[Press any key to exit]");
        }

        static public byte[] FileToByteArray(string _FileName)
        {
            byte[] _Buffer = null;
            try
            {
                // Open file for reading
                System.IO.FileStream _FileStream = new System.IO.FileStream(_FileName, System.IO.FileMode.Open, System.IO.FileAccess.Read);

                // attach filestream to binary reader
                System.IO.BinaryReader _BinaryReader = new System.IO.BinaryReader(_FileStream);

                // get total byte length of the file
                long _TotalBytes = new System.IO.FileInfo(_FileName).Length;

                // read entire file into buffer
                _Buffer = _BinaryReader.ReadBytes((Int32)_TotalBytes);

                // close file reader
                _FileStream.Close();
                _FileStream.Dispose();
                _BinaryReader.Close();
            }
            catch (Exception _Exception)
            {
                Console.WriteLine("Exception caught in process: {0}", _Exception.ToString());
            }
            return _Buffer;
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            if (connected == true)
            {
                try
                {
                    usb_command[0] = 1;
                    ec = writer.Write(usb_command, 1000, out actual_length);
                    ec = reader.Read(usb_command, 1000, out actual_length);
                    if (usb_command[0] == 0xCE && usb_command[1] == 0xED)
                    {
                        timer1.Enabled = false;
                        mode = 4;
                        backgroundWorker1.RunWorkerAsync();
                    }
                    else
                    {
                        Scan_b.Enabled = true;
                        DumpRAM_b.Enabled = false;
                        DumpROM_b.Enabled = false;
                        WriteRAM_b.Enabled = false;
                        Banks_l.Text = "Banks: ";
                        MBC_l.Text = "MBC: ";
                        RAM_l.Text = "RAM Size: ";
                        Size_l.Text = "Size:";
                        Title_l.Text = "Title:";
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show((ec != ErrorCode.None ? ec + ":" : String.Empty) + ex.Message);
                }
            }
        }

        public bool ByteArrayToFile(string _FileName, byte[] _ByteArray)
        {
            try
            {
                // Open file for reading
                System.IO.FileStream _FileStream = new System.IO.FileStream(_FileName, System.IO.FileMode.Create, System.IO.FileAccess.Write);

                // Writes a block of bytes to this stream using data from a byte array.
                _FileStream.Write(_ByteArray, 0, _ByteArray.Length);

                // close file stream
                _FileStream.Close();

                return true;
            }
            catch (Exception _Exception)
            {
                // Error
                Console.WriteLine("Exception caught in process: {0}", _Exception.ToString());
            }

            // error occured, return false
            return false;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            debug = 1;
        }

        private void setButtons_labels()
        {
            byte x = 0;
            byte mbc = rom[0x0147];
		    for(int i=0x0134; i<=0x014C; i++)
		    {
			    x = (byte)(x-rom[i]-1);
		    }
            if(x == rom[0x014D])
            {
                DumpROM_b.Enabled = true;
                if(mbc == 0x03 || mbc == 0x06 || mbc == 0x09 || mbc == 0x0D || mbc == 0x0F || mbc == 0x10 || mbc == 0x13 || mbc == 0x17 || mbc == 0x1B || mbc == 0x1E || mbc == 0xFF)
			    {
                    DumpRAM_b.Enabled = true;
                    WriteRAM_b.Enabled = true;
                }

                switch(rom[0x0148])
		        {
			        case 0: { banks = 2; break; }
			        case 1: { banks = 4; break; }
			        case 2: { banks = 8; break; }
			        case 3: { banks = 16; break; }
			        case 4: { banks = 32; break; }
			        case 5: { if(mbc <= 3) banks = 63; else banks = 64; break; }
			        case 6: { if(rom[0x147] <= 3) banks = 125; else banks = 128; break; }
			        case 7: { banks = 255; break; }
			        default: break;
		        }

                Banks_l.Text = "Banks: " + banks.ToString();

                byte[] Title = new byte[16];
                for (int i = 0; i < 16; i++)
                    Title[i] = rom[0x0134 + i];
                //System.Text.ASCIIEncoding enc = new System.Text.ASCIIEncoding();
                //System.Text.UTF8Encoding
                ROM_Title = new System.Text.ASCIIEncoding().GetString(Title);
                if(ROM_Title.LastIndexOf('?') != -1)
                    ROM_Title.Remove(ROM_Title.LastIndexOf('?') - 4);
                System.Text.UnicodeEncoding enc = new System.Text.UnicodeEncoding();
                Title_l.Text = "Title: " + enc.GetString(Title);
                
                rom_size = rom[0x0148];
                byte ram_size = rom[0x0149];
                
                switch (rom[0x0148])
                {
                    case 0x00: {Size_l.Text = "Size: 32 KBytes"; rom = new byte[32768]; break;}
                    case 0x01: {Size_l.Text = "Size: 64 KBytes"; rom = new byte[65536]; break;}
                    case 0x02: {Size_l.Text = "Size: 128 KBytes"; rom = new byte[131072]; break;}
                    case 0x03: {Size_l.Text = "Size: 256 KBytes"; rom = new byte[262144]; break;}
                    case 0x04: {Size_l.Text = "Size: 512 KBytes"; rom = new byte[524288]; break;}
                    case 0x05: {Size_l.Text = "Size: 1 MBytes"; rom = new byte[1048576]; break;}
                    case 0x06: {Size_l.Text = "Size: 2 MBytes"; rom = new byte[2097152]; break;}
                    case 0x07: {Size_l.Text = "Size: 4 MBytes"; rom = new byte[4194304]; break;}
                    case 0x52: {Size_l.Text = "Size: 1.1 MBytes"; rom = new byte[32768]; break;}
                    case 0x53: {Size_l.Text = "Size: 1.2 MBytes"; rom = new byte[32768]; break;}
                    case 0x54: { Size_l.Text = "Size: 1.5 MBytes"; rom = new byte[1572864]; break; }
                }

                switch (ram_size)
                {
                    case 0x00: {RAM_l.Text = "RAM Size: None"; break;}
                    case 0x01: {RAM_l.Text = "RAM Size: 2 KBytes"; RAM = new byte[2048]; break;}
                    case 0x02: {RAM_l.Text = "RAM Size: 8 KBytes"; RAM = new byte[8192]; break;}
                    case 0x03: { RAM_l.Text = "RAM Size: 32 KBytes"; RAM = new byte[32768]; break; }
                }

                MBC_l.Text = "MBC: None";
                if (mbc <= 0x03 && mbc > 0x00)
			        MBC_l.Text = "MBC: MBC1";
		        if(mbc <= 0x06 && mbc > 0x03)
			        MBC_l.Text = "MBC: MBC2";
		        if(mbc <= 0x13 && mbc > 0x0D)
			        MBC_l.Text = "MBC: MBC3";
		        if(mbc <= 0x17 && mbc > 0x13)
		        	MBC_l.Text = "MBC: MBC4";
		        if(mbc <= 0x0D && mbc > 0x09)
		        	MBC_l.Text = "MBC: MMM01";
		        if(mbc <= 0x1E && mbc > 0x17)
		        	MBC_l.Text = "MBC: MBC5";
		        if(mbc == 0xFD)
		        	MBC_l.Text = "MBC: BANDAI TAMA5";
		        if(mbc == 0xFE)
		        	MBC_l.Text = "MBC: HuC3";
		        if(mbc == 0xFF)
		        	MBC_l.Text = "MBC: HuC1";
		        if(mbc == 0xFC)
		        	MBC_l.Text = "MBC: POCKET CAMERA";	
		    }
        }



        private void Scan_b_Click(object sender, EventArgs e)
        {
            if (debug == 1)
            {
                rom = FileToByteArray("E:\\rom.gb");
                setButtons_labels();
            }
            else
            {
                mode = 4;
                backgroundWorker1.RunWorkerAsync();
            }
        }

        private void saveFileDialogROM_FileOk(object sender, CancelEventArgs e)
        {
            File.WriteAllBytes(saveFileDialogROM.FileName, rom);
        }

        private void DumpROM_b_Click(object sender, EventArgs e)
        {
            timer1.Enabled = false;
            mode = 1;
                stopwatch = new Stopwatch();
            stopwatch.Start();
            progressBar1.Maximum = rom.Length / 64;
            progressBar1.Value = 0;
            backgroundWorker1.RunWorkerAsync();
        }

        private void WriteRAM_b_Click(object sender, EventArgs e)
        {
            timer1.Enabled = false;
            openFileDialog1.ShowDialog();
        }

        private void saveFileDialogRAM_FileOk(object sender, CancelEventArgs e)
        {
            File.WriteAllBytes(saveFileDialogRAM.FileName, RAM);
        }

        private void DumpRAM_b_Click(object sender, EventArgs e)
        {
            timer1.Enabled = false;
            mode = 2;
            progressBar1.Maximum = RAM.Length / 64;
            progressBar1.Value = 0;
            backgroundWorker1.RunWorkerAsync();
        }

        private void backgroundWorker1_DoWork(object sender, DoWorkEventArgs e)
        {
            if (mode == 1)
            {
                usb_command[0] = 0xDD;
                try
                {
                    ec = writer.Write(usb_command, 1000, out actual_length);
                    for (int i = 0; i < (rom.Length / 64); i++)
                    {
                        ec = reader.Read(usb_command, 1000, out actual_length);
                        Buffer.BlockCopy(usb_command, 0, rom, i * 64, 64);
                        backgroundWorker1.ReportProgress(i);
                    }

                }
                catch (Exception ex)
                {
                    MessageBox.Show((ec != ErrorCode.None ? ec + ":" : String.Empty) + ex.Message +" HERE");
                    error = true;
                }
            }
            if (mode == 2)
            {
                usb_command[0] = 0xDA;
                try
                {
                    ec = writer.Write(usb_command, 1000, out actual_length);
                    for (int i = 0; i < ((RAM.Length) / 64); i++)
                    {
                        ec = reader.Read(usb_command, 1000, out actual_length);
                        Buffer.BlockCopy(usb_command, 0, RAM, i * 64, 64);
                        backgroundWorker1.ReportProgress(i);
                    }

                }
                catch (Exception ex)
                {
                    MessageBox.Show((ec != ErrorCode.None ? ec + ":" : String.Empty) + ex.Message);
                    error = true;
                }
            }
            if (mode == 4)
            {
                usb_command[0] = 0x09;
                rom = new byte[16384];
                try
                {
                    help = 1;
                    ec = writer.Write(usb_command, 1000, out actual_length);
                    if (actual_length != 64)
                        MessageBox.Show("Error Writing: " + actual_length.ToString());
                    help = 2;
                    for (int i = 0; i < (30); i++)
                    {
                        
                        ec = reader.Read(usb_command, 1000, out actual_length);
                        if (actual_length != 64)
                            MessageBox.Show("Error Reading: " + actual_length.ToString()+" i: "+i.ToString());
                        help++;
                        Buffer.BlockCopy(usb_command, 0, rom, i * 64, 64);
                        help++;
                        //backgroundWorker1.ReportProgress(i);
                    }

                }
                catch (Exception ex)
                {
                    MessageBox.Show((ec != ErrorCode.None ? ec + ":" : String.Empty) + ex.Message +"help: " + help.ToString());
                    error = true;
                }
            }
        }

        private void backgroundWorker1_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            progressBar1.Value = e.ProgressPercentage;

            int percent = (int)(((double)progressBar1.Value / (double)progressBar1.Maximum) * 100);
            progressBar1.CreateGraphics().DrawString(percent.ToString() + "%", new Font("Arial", (float)8.25, FontStyle.Regular), Brushes.Black, new PointF(progressBar1.Width / 2 - 10, progressBar1.Height / 2 - 7));
        }

        private void backgroundWorker1_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (mode == 1)
            {
                stopwatch.Stop();
                progressBar1.Value = 0;
                saveFileDialogROM.FileName = ROM_Title;
                if(!error)
                    saveFileDialogROM.ShowDialog();
                MessageBox.Show("Time elapsed: " + stopwatch.Elapsed);
                timer1.Enabled = true;
            }
            if (mode == 2)
            {
                saveFileDialogRAM.FileName = ROM_Title;
                if (!error)
                    saveFileDialogRAM.ShowDialog();
                timer1.Enabled = true;
            }
            if (mode == 4)
            {
                //String filename = "C:\\Users\\ViDAR\\Dropbox\\RM01\\dump.bin";
                //ByteArrayToFile(filename, rom);
                setButtons_labels();
                timer1.Enabled = true;
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            try
            {
                MyUsbDevice = UsbDevice.OpenUsbDevice(MyUsbFinder);
                if (MyUsbDevice == null)
                {
                    throw new Exception("Device Not Found.");
                    //connected = false;
                }
                else
                {
                    Device_l.Text = "Device: Connected";
                    //connected = true;

                    Scan_b.Enabled = true;

                    wholeUsbDevice = MyUsbDevice as IUsbDevice;
                    if (!ReferenceEquals(wholeUsbDevice, null))
                    {
                        // This is a "whole" USB device. Before it can be used, 
                        // the desired configuration and interface must be selected.

                        // Select config #1
                        wholeUsbDevice.SetConfiguration(1);

                        // Claim interface #0.
                        wholeUsbDevice.ClaimInterface(0);
                    }

                    reader = MyUsbDevice.OpenEndpointReader(ReadEndpointID.Ep03);
                    writer = MyUsbDevice.OpenEndpointWriter(WriteEndpointID.Ep04);

                }
            }
            catch (Exception ex)
            {
                MessageBox.Show((ec != ErrorCode.None ? ec + ":" : String.Empty) + ex.Message);
            }
        }

        private void button1_Click_1(object sender, EventArgs e)
        {
            usb_command[0] = 0x13;
            try
            {
                ec = writer.Write(usb_command, 1000, out actual_length);
                //MessageBox.Show((ec != ErrorCode.None ? ec + ":" : String.Empty) +  "help: " + actual_length.ToString());
                ec = reader.Read(usb_command, 1000, out actual_length);
                MessageBox.Show((ec != ErrorCode.None ? ec + ":" : String.Empty) + "help: " + actual_length.ToString());
                
            }
            catch (Exception ex)
            {
                MessageBox.Show((ec != ErrorCode.None ? ec + ":" : String.Empty) + ex.Message + "help: " + help.ToString());
                error = true;
            }
        }

        private void button1_Click_2(object sender, EventArgs e)
        {
            usb_command[0] = 0xBB;
            ec = writer.Write(usb_command, 1000, out actual_length);
        }

        private void Form1_KeyDown(object sender, KeyEventArgs e)
        {
            //if (e.KeyData == Keys.A)
                MessageBox.Show("A");
        }

        private void Scan_b_KeyDown(object sender, KeyEventArgs e)
        {
            if (Control.ModifierKeys == Keys.Control)
                nonNumberEntered = true;
        }

        private void Scan_b_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (e.KeyChar == 'b' && nonNumberEntered)
            {
                usb_command[0] = 0xBB;
                ec = writer.Write(usb_command, 1000, out actual_length);
            }
        }

        private void Form1_KeyDown_1(object sender, KeyEventArgs e)
        {
            if (Control.ModifierKeys == Keys.Control)
                nonNumberEntered = true;
        }

        private void Form1_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (e.KeyChar == 'b' && nonNumberEntered)
            {
                usb_command[0] = 0xBB;
                ec = writer.Write(usb_command, 1000, out actual_length);
            }
        }
    }
}
