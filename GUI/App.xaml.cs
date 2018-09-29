///////////////////////////////////////////////////////////////////////
// MainWindow.xaml.cs - GUI to open Multiple Clients                 //
// ver 2.0                                                           //
// Author: Sayali Naval, snaval@syr.edu                              //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2018         //
// Application: CSE687 Project 3-Remote Repository Prototypes        //
// Environment: C#                                                   //
///////////////////////////////////////////////////////////////////////

using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;

namespace WpfApp1
{
  /// <summary>
  /// Interaction logic for App.xaml
  /// </summary>
  public partial class App : Application
  {
        void App_Loaded(object sender, StartupEventArgs e)
        {
            MainWindow w1 = new MainWindow("8082");
            w1.Title = "Client Interface 1";
            w1.Show();
            MainWindow w2 = new MainWindow("8083");
            w2.Title = "Client Interface 2";
            w2.Show();
        }
  }
}
