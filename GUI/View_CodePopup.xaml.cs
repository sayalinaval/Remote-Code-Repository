///////////////////////////////////////////////////////////////////////////
// View_CodePopup.xaml.cs.xaml.cs - GUI for Popup to View file contents  //
// ver 2.0                                                               //
// Author: Sayali Naval, snaval@syr.edu                                  //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2018             //
// Application: CSE687 Project 3-Remote Repository Prototypes            //
// Environment: C#                                                       //
///////////////////////////////////////////////////////////////////////////

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace WpfApp1
{
    /// <summary>
    /// Interaction logic for View_CodePopup.xaml
    /// </summary>
    public partial class View_CodePopup : Window
    {
        private static double leftOffset = 500.0;
        private static double topOffset = -20.0;

        public View_CodePopup()
        {
            InitializeComponent();
            double Left = Application.Current.MainWindow.Left;
            double Top = Application.Current.MainWindow.Top;
            this.Left = Left + leftOffset;
            this.Top = Top + topOffset;
            this.Width = 600.0;
            this.Height = 800.0;
            leftOffset += 20.0;
            topOffset += 20.0;
            if (leftOffset > 700.0)
                leftOffset = 500.0;
            if (topOffset > 180.0)
                topOffset = -20.0;
        }

        private void exitButton_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }
    }
}
