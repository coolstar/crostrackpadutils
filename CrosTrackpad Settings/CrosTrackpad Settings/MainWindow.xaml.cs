using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Pipes;
using System.Linq;
using System.Runtime.InteropServices;
using System.Security.Principal;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace CrosTrackpad_Settings
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        [DllImport("crostrackpadsettingslib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int ConnectCrosTpClient();

        [DllImport("crostrackpadsettingslib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int DisconnectCrosTpClient();

        [DllImport("crostrackpadsettingslib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int CrosTpClientChangeSetting(int settingRegister, int settingValue);

        [DllImport("crostrackpadsettingslib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int CrosTpType();

        [DllImport("crostrackpadsettingslib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr CrosTpDriverVersion();

        [DllImport("crostrackpadsettingslib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr CrosTpProductId();

        [DllImport("crostrackpadsettingslib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr CrosTpFirmwareVersion();

        private bool enableSetting = false;

        public MainWindow()
        {
            int connect = ConnectCrosTpClient();
            if (connect != 0)
            {
                MessageBox.Show("Error Connecting to CrosTrackpad!");
            }
            InitializeComponent();

            if (CrosTpType() == 0)
                deviceName.Content += "Cypress Gen3 Trackpad";
            else if (CrosTpType() == 1)
                deviceName.Content += "Elan I2C Trackpad";
            else if (CrosTpType() == 2)
                deviceName.Content += "Atmel MaxTouch Trackpad";
            else if (CrosTpType() == 3)
                deviceName.Content += "Synaptics RMI Trackpad";
            else
                deviceName.Content += "Unknown Trackpad";

            Marshal.PtrToStringAnsi(CrosTpDriverVersion());

            driverVersion.Content += Marshal.PtrToStringAnsi(CrosTpDriverVersion());
            productName.Content += Marshal.PtrToStringAnsi(CrosTpProductId());
            firmwareVersion.Content += Marshal.PtrToStringAnsi(CrosTpFirmwareVersion());

            loadSettings();

            enableSetting = true;
        }

        private void loadSettings()
        {
            if (File.Exists(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData) + "\\crostp-driversettings.txt"))
            {
                String settings = File.ReadAllText(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData) + "\\crostp-driversettings.txt");

                String[] lines = settings.Split('\n');
                foreach (String line in lines)
                {
                    if (line.Length < 3)
                        continue;
                    String[] parameters = line.Split(' ');

                    int cmd = int.Parse(parameters[0]);
                    int value = int.Parse(parameters[1]);

                    switch (cmd)
                    {
                        case 0:
                            panMultiplier.Value = value / 10.0f;
                            break;
                        case 1:
                            invertClick.IsChecked = value == 1;
                            break;
                        case 2:
                            noFingerClick.IsChecked = value == 1;
                            break;
                        case 3:
                            multiFingerClick.IsChecked = value == 1;
                            break;
                         case 4:
                            bottomRightClick.IsChecked = value == 1;
                            break;


                        case 5:
                            tapToClick.IsChecked = value == 1;
                            break;
                        case 6:
                            tapToRightClick.IsChecked = value == 1;
                            break;
                        case 7:
                            tapDrag.IsChecked = value == 1;
                            break;
                        case 8:
                            threeFingerTap.SelectedIndex = value;
                            break;
                        case 9:
                            fourFingerTap.IsChecked = value == 1;
                            break;

                        case 10:
                            enableScroll.IsChecked = value == 1;
                            break;

                        case 11:
                            threeFingerUp.SelectedIndex = value;
                            break;
                        case 12:
                            threeFingerDown.SelectedIndex = value;
                            break;
                        case 13:
                            threeFingerLeftRight.SelectedIndex = value;
                            break;
                        case 14:
                            fourFingerUp.SelectedIndex = value;
                            break;
                        case 15:
                            fourFingerDown.SelectedIndex = value;
                            break;
                        case 16:
                            fourFingerLeftRight.SelectedIndex = value;
                            break;
                    }
                }
            }

            if (File.Exists(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData) + "\\crostp-scrollsettings.txt"))
            {
                String settings = File.ReadAllText(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData) + "\\crostp-scrollsettings.txt");

                String[] lines = settings.Split('\n');
                foreach (String line in lines)
                {
                    if (line.Length < 3)
                        continue;
                    String[] parameters = line.Split(' ');

                    int cmd = int.Parse(parameters[0]);
                    int value = int.Parse(parameters[1]);

                    switch (cmd)
                    {
                        case 0:
                            naturalScroll.IsChecked = value == 1;
                            break;
                        case 1:
                            horizontalScroll.IsChecked = value == 1;
                            break;
                        case 2:
                            scrollMultiplier.Value = value / 10.0f;
                            break;
                        case 3:
                            inertiaScroll.IsChecked = value == 1;
                            break;
                    }
                }
            }
        }

        private void settingChanged(object sender, EventArgs e)
        {
            if (!enableSetting)
                return;
            String contents = "";

            CrosTpClientChangeSetting(0, (int)(panMultiplier.Value * 10));
            contents += String.Format("0 {0}\n", (int)(panMultiplier.Value * 10));
            CrosTpClientChangeSetting(1, invertClick.IsChecked.Value ? 1 : 0);
            contents += String.Format("1 {0}\n", invertClick.IsChecked.Value ? 1 : 0);
            CrosTpClientChangeSetting(2, noFingerClick.IsChecked.Value ? 1 : 0);
            contents += String.Format("2 {0}\n", noFingerClick.IsChecked.Value ? 1 : 0);
            CrosTpClientChangeSetting(3, multiFingerClick.IsChecked.Value ? 1 : 0);
            contents += String.Format("3 {0}\n", multiFingerClick.IsChecked.Value ? 1 : 0);
            CrosTpClientChangeSetting(4, bottomRightClick.IsChecked.Value ? 1 : 0);
            contents += String.Format("4 {0}\n", bottomRightClick.IsChecked.Value ? 1 : 0);


            CrosTpClientChangeSetting(5, tapToClick.IsChecked.Value ? 1 : 0);
            contents += String.Format("5 {0}\n", tapToClick.IsChecked.Value ? 1 : 0);
            CrosTpClientChangeSetting(6, tapToRightClick.IsChecked.Value ? 1 : 0);
            contents += String.Format("6 {0}\n", tapToRightClick.IsChecked.Value ? 1 : 0);
            CrosTpClientChangeSetting(7, tapDrag.IsChecked.Value ? 1 : 0);
            contents += String.Format("7 {0}\n", tapDrag.IsChecked.Value ? 1 : 0);
            CrosTpClientChangeSetting(8, threeFingerTap.SelectedIndex);
            contents += String.Format("8 {0}\n", threeFingerTap.SelectedIndex);
            CrosTpClientChangeSetting(9, fourFingerTap.IsChecked.Value ? 1 : 0);
            contents += String.Format("9 {0}\n", fourFingerTap.IsChecked.Value ? 1 : 0);


            CrosTpClientChangeSetting(10, enableScroll.IsChecked.Value ? 1 : 0);
            contents += String.Format("10 {0}\n", enableScroll.IsChecked.Value ? 1 : 0);


            CrosTpClientChangeSetting(11, threeFingerUp.SelectedIndex);
            contents += String.Format("11 {0}\n", threeFingerUp.SelectedIndex);
            CrosTpClientChangeSetting(12, threeFingerDown.SelectedIndex);
            contents += String.Format("11 {0}\n", threeFingerDown.SelectedIndex);
            CrosTpClientChangeSetting(13, threeFingerLeftRight.SelectedIndex);
            contents += String.Format("13 {0}\n", threeFingerLeftRight.SelectedIndex);
            CrosTpClientChangeSetting(14, fourFingerUp.SelectedIndex);
            contents += String.Format("14 {0}\n", fourFingerUp.SelectedIndex);
            CrosTpClientChangeSetting(15, fourFingerDown.SelectedIndex);
            contents += String.Format("15 {0}\n", fourFingerDown.SelectedIndex);
            CrosTpClientChangeSetting(16, fourFingerLeftRight.SelectedIndex);
            contents += String.Format("16 {0}\n", fourFingerLeftRight.SelectedIndex);

            File.WriteAllText(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData) + "\\crostp-driversettings.txt", contents);
        }

        private void scrollSettingChanged(object sender, EventArgs e)
        {
            if (!enableSetting)
                return;
            String contents = "";
            contents += String.Format("0 {0}\n", naturalScroll.IsChecked.Value ? 1 : 0);
            contents += String.Format("1 {0}\n", horizontalScroll.IsChecked.Value ? 1 : 0);
            contents += String.Format("2 {0}\n", (int)(scrollMultiplier.Value * 10));
            contents += String.Format("3 {0}\n", inertiaScroll.IsChecked.Value ? 1 : 0);
            contents += String.Format("4 {0}\n", pinchZoom.IsChecked.Value ? 1 : 0);

            File.WriteAllText(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData) + "\\crostp-scrollsettings.txt", contents);

            NamedPipeClientStream pipeClient =
                    new NamedPipeClientStream(".", "CrosTrackpadSettings",
                        PipeDirection.InOut, PipeOptions.None,
                        TokenImpersonationLevel.Impersonation);
            pipeClient.Connect();

            var sw = new StreamWriter(pipeClient, new ASCIIEncoding());
            sw.Write(contents);
            sw.Flush();
            sw.Dispose();
            pipeClient.Close();
        }
    }
}
