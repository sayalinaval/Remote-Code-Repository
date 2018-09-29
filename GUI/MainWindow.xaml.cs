///////////////////////////////////////////////////////////////////////
// MainWindow.xaml.cs - GUI for Project3HelpWPF                      //
// ver 2.0                                                           //
// Author: Sayali Naval, snaval@syr.edu                              //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2018         //
// Application: CSE687 Project #4 - Remote Code Repository           //
// Environment: C#                                                   //
///////////////////////////////////////////////////////////////////////
/*
 * Package Operations:
 * -------------------
 * It's responsibilities are to:
 * - Provide a display of directory contents of a remote ServerPrototype.
 * - It provides a subdirectory list and a filelist for the selected directory.
 * - You can navigate into subdirectories by double-clicking on subdirectory
 *   or the parent directory, indicated by the name "..".
 * - It supports Chcek in, Chcek out, Browse Files, View Meta Data, View File and Query operations
 * - It sends and receives messages from server RemoteRepo (port 8080).
 * - It shows windows for 2 clients viz Client Interface 1 (port: 8082) and Client Interface 2 (port: 8083).
 *   
 * Required Files:
 * ---------------
 * Mainwindow.xaml, MainWindow.xaml.cs
 * View_CodePopup.xaml View_CodePopup.xaml.cs
 * Translater.dll
 * 
 * Maintenance History:
 * --------------------
 * ver 1.1 : 1 May 2018
 * - second release
 * ver 1.0 : 30 Mar 2018
 * - first release
 * - Several early prototypes were discussed in class. Those are all superceded
 *   by this package.
 */

// Translater has to be statically linked with CommLibWrapper
// - loader can't find Translater.dll dependent CommLibWrapper.dll

using System;
using System.Collections.Generic;
using System.IO;
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
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Threading;
using MsgPassingCommunication;

namespace WpfApp1
{
  public partial class MainWindow : Window
  {
    List<View_CodePopup> popups = new List<View_CodePopup>();
    public MainWindow()
    {
      InitializeComponent();
      Console.Title = "Project4 GUI Console";
    }

    //initialize component and assign port number for client
    public MainWindow(string port)
    {
       InitializeComponent();
       Console.Title = "Project4 GUI Console";
       port_ = port;
    }

    private Stack<string> pathStack_ = new Stack<string>();
    private Stack<string> pathStackCheckout_ = new Stack<string>();
    private Stack<string> pathStackBrowse_ = new Stack<string>();
    private Translater translater;
    private string port_;
    private CsEndPoint endPoint_;
    private List<string> categories = new List<string>();
    private List<string> dependencies = new List<string>();
    private Thread rcvThrd = null;
    private Dictionary<string, Action<CsMessage>> dispatcher_ 
      = new Dictionary<string, Action<CsMessage>>();

    //----< process incoming messages on child thread >----------------
    private void processMessages()
    {
      ThreadStart thrdProc = () => {
        while (true)
        {
          CsMessage msg = translater.getMessage();
          if (msg.attributes.Count() == 0)
            continue;
          string msgId = msg.value("command");
          if (dispatcher_.ContainsKey(msgId))
            dispatcher_[msgId].Invoke(msg);
          msg.show();
        }
      };
      rcvThrd = new Thread(thrdProc);
      rcvThrd.IsBackground = true;
      rcvThrd.Start();
    }

    //----< clear directory list for CheckIn tab >-------
    private void clearDirs()
    {
      DirList.Items.Clear();
    }

    //----< add directories in CheckIn tab >-------
    private void addDir(string dir)
    {
      DirList.Items.Add(dir);
    }

    //----< add parent directory in CheckIn tab >-------
    private void insertParent()
    {
      DirList.Items.Insert(0, "..");
    }

    //----< clear files in CheckIn tab >-------
    private void clearFiles()
    {
      FileList.Items.Clear();
    }

    //----< add files in CheckIn tab >-------
    private void addFile(string file)
    {
      FileList.Items.Add(file);
    }

    //----< add client processing for message with key >---------------
    private void addClientProc(string key, Action<CsMessage> clientProc)
    {
      dispatcher_[key] = clientProc;
    }

    //----< load getDirs processing into dispatcher dictionary >-------
    private void DispatcherLoadGetDirs()
    {
      Action<CsMessage> getDirs = (CsMessage rcvMsg) =>
      {
        Action clrDirs = () =>
        {
            if(CheckOutTab.IsSelected == true)
                DirListCheckout.Items.Clear();
            else
                DirListBrowse.Items.Clear();
        };
        Dispatcher.Invoke(clrDirs, new Object[] { });
        var enumer = rcvMsg.attributes.GetEnumerator();
        while (enumer.MoveNext())
        {
          string key = enumer.Current.Key;
          if (key.Contains("dir"))
          {
            Action<string> doDir = (string dir) =>
            {
                if (CheckOutTab.IsSelected == true)
                    DirListCheckout.Items.Add(dir);
                else
                    DirListBrowse.Items.Add(dir);
            };
            Dispatcher.Invoke(doDir, new Object[] { enumer.Current.Value });
          }
        }
        Action insertUp = () =>
        {
            if (CheckOutTab.IsSelected == true)
                DirListCheckout.Items.Insert(0, "..");
            else
                DirListBrowse.Items.Insert(0, "..");
        };
        Dispatcher.Invoke(insertUp, new Object[] { });
      };
      addClientProc("getDirs", getDirs);
    }

    //----< load getFiles processing into dispatcher dictionary >------
    private void DispatcherLoadGetFiles()
    {
      Action<CsMessage> getFiles = (CsMessage rcvMsg) =>
      {
        Action clrFiles = () =>
        {
            if (CheckOutTab.IsSelected == true)
                FileListCheckout.Items.Clear();
            else
                FileListBrowse.Items.Clear();
        };
        Dispatcher.Invoke(clrFiles, new Object[] { });
        var enumer = rcvMsg.attributes.GetEnumerator();
        while (enumer.MoveNext())
        {
          string key = enumer.Current.Key;
          if (key.Contains("file"))
          {
            Action<string> doFile = (string file) =>
            {
                if (CheckOutTab.IsSelected == true)
                    FileListCheckout.Items.Add(file);
                else
                    FileListBrowse.Items.Add(file);
            };
            Dispatcher.Invoke(doFile, new Object[] { enumer.Current.Value });
          }
        }
      };
      addClientProc("getFiles", getFiles);
    }

        //----< load checkIn processing into dispatcher dictionary >-------
        private void DispatcherCheckinFiles()
        {
            Action<CsMessage> checkIn = (CsMessage rcvMsg) =>
            {
                Action checkInMsg = () =>
                {
                    string file = rcvMsg.value("name");
                    CsEndPoint serverEndPoint = new CsEndPoint();
                    serverEndPoint.machineAddress = "localhost";
                    serverEndPoint.port = 8080;
                    CsMessage msg1 = new CsMessage();
                    msg1.add("to", CsEndPoint.toString(serverEndPoint));
                    msg1.add("from", CsEndPoint.toString(endPoint_));
                    msg1.add("command", "checkIn");
                    msg1.add("toPath", "../RemoteFiles");
                    msg1.add("file", file);
                    msg1.add("path", rcvMsg.value("path") + "/" + file);
                    msg1.add("fileName", rcvMsg.value("fileName"));
                    msg1.add("fileDesc", rcvMsg.value("fileDesc"));
                    msg1.add("fileCategories", rcvMsg.value("fileCategories"));
                    msg1.add("fileDependencies", rcvMsg.value("fileDependencies"));
                    translater.postMessage(msg1);
                    statusBarText.Text = "Status: File checked in successfully";
                };
                Dispatcher.Invoke(checkInMsg, new Object[] { });
            };
            addClientProc("checkInReady", checkIn);
        }

        //----< load checkOutReceived processing into dispatcher dictionary >-------
        private void DispatcherCheckoutFiles()
        {
          Action<CsMessage> checkOutReceived = (CsMessage rcvMsg) =>
          {
              Action checkOutMsg = () =>
              {
                  string file = rcvMsg.value("name");
                  CsEndPoint serverEndPoint = new CsEndPoint();
                  serverEndPoint.machineAddress = "localhost";
                  serverEndPoint.port = 8080;
                  CsMessage msg1 = new CsMessage();
                  msg1.add("to", CsEndPoint.toString(serverEndPoint));
                  msg1.add("from", CsEndPoint.toString(endPoint_));
                  msg1.add("command", "checkOutReceived");
                  msg1.add("path", rcvMsg.value("path"));
                  msg1.add("port", rcvMsg.value("port"));
                  msg1.add("toPath", rcvMsg.value("toPath"));
                  msg1.add("name", file);
                  translater.postMessage(msg1);
                  statusBarText.Text = "Status: Files checked out successfully";
              };
              Dispatcher.Invoke(checkOutMsg, new Object[] { });
          };
          addClientProc("checkOut", checkOutReceived);
        }

        //----< load metaData processing into dispatcher dictionary >-------
        private void DispatcherViewMetaData()
        {
            Action<CsMessage> sendData = (CsMessage rcvMsg) =>
            {
                Action viewMetaData = () =>
                {
                    string data = "Author name: " + rcvMsg.value("author") + "\nDescription: " 
                    + rcvMsg.value("description") + "\nDateTime: " + rcvMsg.value("dateTime") 
                    + "\nChildren: " + rcvMsg.value("children") + "\nPayLoad Value: "
                    + rcvMsg.value("value") + "\nCheckIn Status: " 
                    + rcvMsg.value("checkInStatus") + "\nCategories: " + rcvMsg.value("categories");
                    if (BrowseTab.IsSelected == true)
                    {
                        metaData.Text = data;
                        statusBarText.Text = "Status: Meta Data received for file " + rcvMsg.value("name");
                    }                        
                    CsEndPoint serverEndPoint = new CsEndPoint();
                    serverEndPoint.machineAddress = "localhost";
                    serverEndPoint.port = 8080;
                    CsMessage msg1 = new CsMessage();
                    msg1.add("to", CsEndPoint.toString(serverEndPoint));
                    msg1.add("from", CsEndPoint.toString(endPoint_));
                    msg1.add("command", "metaDataReceived");
                    msg1.add("path", rcvMsg.value("path"));
                    msg1.add("data", data);
                    translater.postMessage(msg1);
                };
                Dispatcher.Invoke(viewMetaData, new Object[] { });
            };
            addClientProc("metaData", sendData);
        }

        //----< load viewFile processing into dispatcher dictionary >-------
        private void DispatcherViewFile()
        {
            Action<CsMessage> viewFile = (CsMessage rcvMsg) =>
            {
                    Action openFile = () =>
                    {
                            View_CodePopup codePopup = new View_CodePopup();
                            codePopup.Show();
                            popups.Add(codePopup);
                            codePopup.codeView.Blocks.Clear();
                            string path = rcvMsg.value("toPath") + "/" + rcvMsg.value("file");
                            Paragraph paragraph = new Paragraph();
                            string fileText = "";
                            try
                            {
                                fileText = System.IO.File.ReadAllText(path);
                            }
                            finally
                            {
                                paragraph.Inlines.Add(new Run(fileText));
                            }
                            // add code text to code view
                            codePopup.codeView.Blocks.Clear();
                            codePopup.codeLabel.Text = rcvMsg.value("file");
                            codePopup.codeView.Blocks.Add(paragraph);
                            CsEndPoint serverEndPoint = new CsEndPoint();
                            serverEndPoint.machineAddress = "localhost";
                            serverEndPoint.port = 8080;
                            CsMessage msg1 = new CsMessage();
                            msg1.add("to", CsEndPoint.toString(serverEndPoint));
                            msg1.add("from", CsEndPoint.toString(endPoint_));
                            msg1.add("command", "fileReceived");
                            msg1.add("name", rcvMsg.value("file"));
                            msg1.add("path", rcvMsg.value("path"));
                            translater.postMessage(msg1);
                            statusBarText.Text = "Status: File " + rcvMsg.value("file") + " received";
                    };
                    Dispatcher.Invoke(openFile, new Object[] { });
            };
            addClientProc("viewFile", viewFile);
        }

        //----< load queryResult processing into dispatcher dictionary >-------
        private void DispatcherQueryResult()
        {
            Action<CsMessage> queryResult = (CsMessage rcvMsg) =>
            {
                Action clrQuery = () =>
                {
                    FileListQuery.Items.Clear();
                };
                Dispatcher.Invoke(clrQuery, new Object[] { });

                var enumer = rcvMsg.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    string key = enumer.Current.Key;
                    if (key.Contains("file"))
                    {
                        Action<string> showQuery = (string file) =>
                        {
                            FileListQuery.Items.Add(file);
                        };
                        Dispatcher.Invoke(showQuery, new Object[] { enumer.Current.Value });
                    }
                }
            };
            addClientProc("queryResult", queryResult);
        }

        //----< load noParentFiles processing into dispatcher dictionary >-------
        private void DispatcherNoParentFile()
        {
            Action<CsMessage> noParentResult = (CsMessage rcvMsg) =>
            {
                Action clrParent = () =>
                {
                    FileListQuery.Items.Clear();
                };
                Dispatcher.Invoke(clrParent, new Object[] { });

                var enumer = rcvMsg.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    string key = enumer.Current.Key;
                    if (key.Contains("file"))
                    {
                        Action<string> noParent = (string file) =>
                        {
                            FileListQuery.Items.Add(file);
                        };
                        Dispatcher.Invoke(noParent, new Object[] { enumer.Current.Value });
                    }
                }
            };
            addClientProc("noParentFiles", noParentResult);
        }

        //call all dispatcher processing functions
        private void loadDispatcher()
        {
           DispatcherLoadGetDirs();
           DispatcherLoadGetFiles();
           DispatcherCheckinFiles();
           DispatcherCheckoutFiles();
           DispatcherViewMetaData();
           DispatcherViewFile();
           DispatcherQueryResult();
           DispatcherNoParentFile();
        }

        //send connctRequest message to Server
        void connectServer()
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;

            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "connectRequest");
            translater.postMessage(msg);
        }
        
        //add categories entered by user in a list of strings categories
        void addCategory(string category)
        {
            categories.Add(category);
        }

        //add dependencies entered by user in a list of strings dependencies
        void addDependency(string dependency)
        {
            dependencies.Add(dependency);
        }

        //send checkInRequest message to Server
        void checkInFile(string file,string name,string desc,string cat,string depend)
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;

            CsMessage msg1 = new CsMessage();
            msg1.add("to", CsEndPoint.toString(serverEndPoint));
            msg1.add("from", CsEndPoint.toString(endPoint_));
            msg1.add("command", "checkInRequest");
            msg1.add("path", pathStack_.Peek());
            msg1.add("name", file);
            msg1.add("fileName", name);
            msg1.add("fileDesc", desc);
            msg1.add("fileCategories", cat);
            msg1.add("fileDependencies", depend);
            translater.postMessage(msg1);
        }

        //send getDirs and getFiles messages to Server for CheckOut tab
        void getFilesCheckOut()
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;

            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "getDirs");
            msg.add("path", pathStackCheckout_.Peek());
            translater.postMessage(msg);
            msg.remove("command");
            msg.add("command", "getFiles");
            translater.postMessage(msg);
        }

        //send checkOutRequest message to Server
        void checkOutFile(string file)
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;

            CsMessage msg1 = new CsMessage();
            msg1.add("to", CsEndPoint.toString(serverEndPoint));
            msg1.add("from", CsEndPoint.toString(endPoint_));
            msg1.add("command", "checkOutRequest");
            msg1.add("path", pathStackCheckout_.Peek());
            msg1.add("name", file);
            msg1.add("port", port_);
            if (port_ == "8082")
                msg1.add("toPath", "../../../../SaveFiles");
            else
                msg1.add("toPath", "../../../../SaveFiles1");
            translater.postMessage(msg1);
        }

        //send changeStatusRequest message to Server
        void changeStatus(string file)
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;

            CsMessage msg1 = new CsMessage();
            msg1.add("to", CsEndPoint.toString(serverEndPoint));
            msg1.add("from", CsEndPoint.toString(endPoint_));
            msg1.add("command", "changeStatusRequest");
            msg1.add("path", pathStackCheckout_.Peek());
            msg1.add("name", file);
            translater.postMessage(msg1);
        }

        //send getDirs and getFiles messages to Server for Browse tab
        void getFilesBrowse()
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;

            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "getDirs");
            msg.add("path", pathStackBrowse_.Peek());
            translater.postMessage(msg);
            msg.remove("command");
            msg.add("command", "getFiles");
            translater.postMessage(msg);
        }

        //send metaDataRequest message to Server
        void requestMetaData(string file)
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;

            CsMessage msg1 = new CsMessage();
            msg1.add("to", CsEndPoint.toString(serverEndPoint));
            msg1.add("from", CsEndPoint.toString(endPoint_));
            msg1.add("command", "metaDataRequest");
            msg1.add("path", pathStackBrowse_.Peek());
            msg1.add("name", file);
            translater.postMessage(msg1);
        }

        //send viewRequest message to Server
        void viewFile(string file)
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;

            CsMessage msg1 = new CsMessage();
            msg1.add("to", CsEndPoint.toString(serverEndPoint));
            msg1.add("from", CsEndPoint.toString(endPoint_));
            msg1.add("command", "viewRequest");
            msg1.add("path", pathStackBrowse_.Peek());
            msg1.add("name", file);
            if (port_ == "8082")
                msg1.add("toPath", "../../../../SaveFiles");
            else
                msg1.add("toPath", "../../../../SaveFiles1");
            translater.postMessage(msg1);
        }

        //send queryRequest message to Server
        void queryGetFiles(string name,string desc,string cat,string file,string child,string version)
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;

            CsMessage msg1 = new CsMessage();
            msg1.add("to", CsEndPoint.toString(serverEndPoint));
            msg1.add("from", CsEndPoint.toString(endPoint_));
            msg1.add("command", "queryRequest");
            msg1.add("path", pathStackBrowse_.Peek());
            msg1.add("queryName", name);
            msg1.add("queryDesc", desc);
            msg1.add("queryCategory", cat);
            msg1.add("queryFile", file);
            msg1.add("queryChild", child);
            msg1.add("queryVersion", version);
            translater.postMessage(msg1);
        }

        //send noParentRequest message to Server
        void queryNoParents()
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;

            CsMessage msg1 = new CsMessage();
            msg1.add("to", CsEndPoint.toString(serverEndPoint));
            msg1.add("from", CsEndPoint.toString(endPoint_));
            msg1.add("command", "noParentRequest");
            msg1.add("path", pathStackBrowse_.Peek());
            translater.postMessage(msg1);
        }

        //Test cases for checkIn
        void testCases1()
        {
            Console.WriteLine("\n\nConnect Local Client 8082 to Remote Server 8080");
            connectServer();

            pathStack_.Push("../../../../Storage/Version");
            Console.WriteLine("\n\nCheckIn new file Version.h from Storage to RemoteRepo");
            checkInFile("Version.h", "Sayali", "version file", "version-header", "");

            Console.WriteLine("\n\nCheckIn new file Version.cpp as parent of Version.h.1 from Storage to RemoteRepo");
            checkInFile("Version.cpp", "Sayali Naval", "version package", "cpp", "Version::Version.h.1");

            Console.WriteLine("\n\nCheckIn existing file Version.h from Storage to RemoteRepo");
            checkInFile("Version.h", "", "", "package", "");

            pathStackCheckout_.Push("../RemoteRepo/Process");
            Console.WriteLine("\n\nChange CheckIn Status of Process.h.1 to close");
            changeStatus("Process.h.1");

            pathStack_.Push("../../../../Storage/Process");
            Console.WriteLine("\n\nCheckIn exising file Process.h for checkIn status close from Storage to RemoteRepo");
            checkInFile("Process.h", "", "new version process file", "", "");

            pathStackCheckout_.Push("../RemoteRepo/Query");
            Console.WriteLine("\n\nChange CheckIn Status of Query.cpp.1 to close");
            changeStatus("Query.cpp.1");

            pathStack_.Push("../../../../Storage/Query");
            Console.WriteLine("\n\nCheckIn exising file Query.cpp for checkIn status pending from Storage to RemoteRepo");
            checkInFile("Query.cpp", "Sayali", "query file v2", "", "");
        }

        //test cases for checkOut, browse and queries
        void testCases2()
        {
            Console.WriteLine("\n\nConnect Local Client 8083 to Remote Server 8080");
            connectServer();

            pathStackCheckout_.Push("../RemoteRepo/Query");
            Console.WriteLine("\n\nCheckOut file Query.h.1 to LocalRepo");
            checkOutFile("Query.h.1");

            pathStackCheckout_.Push("../RemoteRepo/Process");
            Console.WriteLine("\n\nCheckOut file Process.cpp.1 and its children to LocalRepo");
            checkOutFile("Process.cpp.1");

            pathStackBrowse_.Push("../RemoteRepo/Version");
            Console.WriteLine("\n\nView file Version.h.1 from RemoteRepo in a new pop up window");
            viewFile("Version.h.1");

            pathStackBrowse_.Push("../RemoteRepo/Query");
            Console.WriteLine("\n\nView meta data for file Query.cpp.1 from RemoteRepo");
            requestMetaData("Query.cpp.1");

            pathStackBrowse_.Push("../RemoteRepo");
            Console.WriteLine("\n\nShow file list for query result of ANDing of Description containing package and version 1");
            queryGetFiles("", "package", "", "", "", "1");

            Console.WriteLine("\n\nShow file list for query result of ANDing of Author name containing Naval and child containing Query.h");
            queryGetFiles("Naval", "", "", "", "Query.h", "");

            Console.WriteLine("\n\nShow list of files having no parents");
            queryNoParents();
        }

        //----< start Comm >------
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            endPoint_ = new CsEndPoint();
            endPoint_.machineAddress = "localhost";
            endPoint_.port = Int32.Parse(port_);
            translater = new Translater();
            translater.listen(endPoint_);

            PathTextBlockCheckout.Text = "/RemoteRepo";
            pathStackCheckout_.Push("../RemoteRepo");

            PathTextBlockBrowse.Text = "/RemoteRepo";
            pathStackBrowse_.Push("../RemoteRepo");

            CheckInTab.IsEnabled = false;
            CheckOutTab.IsEnabled = false;
            BrowseTab.IsEnabled = false;
            QueryTab.IsEnabled = false;

            // start processing messages
            processMessages();

            // load dispatcher
            loadDispatcher();

            //set path for client based on it's port number and run automated unit test cases on both
            if (port_ == "8082")
            {
                PathTextBlock.Text = "/Storage";
                testCases1();
                pathStack_.Push("../../../../Storage");
            }

            if (port_ == "8083")
            {
                PathTextBlock.Text = "/LocalRepo";
                testCases2();
                pathStack_.Push("../../../../LocalRepo");
            }
            pathStackCheckout_.Push("../RemoteRepo");
            pathStackBrowse_.Push("../RemoteRepo");
        }

        //set actions for when Connect button is clicked in Connect Tab
        private void Connect_Server(object sender, RoutedEventArgs e)
        {
            connectServer();
            CheckInTab.IsEnabled = true;
            CheckOutTab.IsEnabled = true;
            BrowseTab.IsEnabled = true;
            QueryTab.IsEnabled = true;
            statusBarText.Text = "Status: Successfully Connected to the Server RemoteRepo 8080";
        }

        //set actions for when CheckIn Window is loaded
        //add directory and file names in CheckIn Tab
        private void CheckIn_Loaded(object sender, RoutedEventArgs e)
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;

            clearDirs();
            string[] listOfDirs = Directory.GetDirectories(pathStack_.Peek());
            foreach (string dir in listOfDirs)
            {
                addDir(System.IO.Path.GetFileName(dir));
            }
            insertParent();

            clearFiles();
            string[] listOfFiles = Directory.GetFiles(pathStack_.Peek());
            foreach (string file in listOfFiles)
            {
                addFile(System.IO.Path.GetFileName(file));
            }
        }

        //set actions for when CheckIn button is clicked in CheckIn Tab
        private void CheckIn_Files(object sender, RoutedEventArgs e)
        {
            if (FileList.SelectedItem == null)
                statusBarText.Text = "Status: Please select a file";
            else
            {
                string cat = string.Join("-", categories);
                string depend = string.Join("-", dependencies);
                checkInFile(FileList.SelectedItem.ToString(),FileName.Text,FileDescription.Text,cat,depend);
            }
            FileList.UnselectAll();
            FileName.Clear();
            FileDescription.Clear();
            FileCategories.Clear();
            categories.Clear();
            FileDependency.Clear();
            dependencies.Clear();
        }

        //set actions for when Add Category button is clicked in CheckIn Tab
        private void Add_Category(object sender, RoutedEventArgs e)
        {
            if (FileList.SelectedItem == null)
                statusBarText.Text = "Status: Please select a file";
            else
            {
                if (FileCategories.Text != null)
                {
                    addCategory(FileCategories.Text);
                    statusBarText.Text = "Status: Category " + FileCategories.Text + " added";
                }
            }
            FileCategories.Clear();
        }

        //set actions for when Add Dependencies button is clicked in CheckIn Tab
        private void Add_Dependency(object sender, RoutedEventArgs e)
        {
            if (FileList.SelectedItem == null)
                statusBarText.Text = "Status: Please select a file";
            else
            {
                if (FileDependency.Text != null)
                {
                    addDependency(FileDependency.Text);
                    statusBarText.Text = "Status: Dependency " + FileDependency.Text + " added";
                }
            }
            FileDependency.Clear();
        }

        //set actions for when Get Dirs and Files button is clicked in CheckOut Tab
        private void CheckOut_GetFiles(object sender, RoutedEventArgs e)
        {
            getFilesCheckOut();
            statusBarText.Text = "Status: Select files to Check Out";
        }

        //set actions for when CheckOut button is clicked in CheckOut Tab
        private void CheckOut_Files(object sender, RoutedEventArgs e)
        {
            if (FileListCheckout.SelectedItems.Count == 0)
                statusBarText.Text = "Status: Please select at least one file";
            else
            {
                foreach (string file in FileListCheckout.SelectedItems)
                {
                    checkOutFile(file);
                }
                FileListCheckout.UnselectAll();
            }
        }

        //set actions for when Change CheckIn Status button is clicked in CheckOut Tab
        private void Change_Status(object sender, RoutedEventArgs e)
        {
            if (FileListCheckout.SelectedItems.Count == 0)
                statusBarText.Text = "Status: Please select at least one file";
            else
            {
                foreach (string file in FileListCheckout.SelectedItems)
                {
                    changeStatus(file);
                }
                FileListCheckout.UnselectAll();
            }
        }

        //set actions for when Browse Files button is clicked in Browse Tab
        private void Browse_GetFiles(object sender, RoutedEventArgs e)
        {
            getFilesBrowse();
            statusBarText.Text = "Status: Select a file to View Meta Data or View File";
        }

        //set actions for when View Meta Data button is clicked in Browse Tab
        private void MetaData_Request(object sender, RoutedEventArgs e)
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;

            if((string)FileListBrowse.SelectedItem == null)
                statusBarText.Text = "Status: Please select a file";
            else
            {
                string file = (string)FileListBrowse.SelectedItem;
                requestMetaData(file);
            }
        }

        //set actions for when View File button is clicked in Browse Tab
        private void View_File(object sender, RoutedEventArgs e)
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;

            if ((string)FileListBrowse.SelectedItem == null)
                statusBarText.Text = "Status: Please select a file";
            else
            {
                string file = (string)FileListBrowse.SelectedItem;
                viewFile(file);
                FileListBrowse.UnselectAll();
            }
        }

        //set actions for when Browse Query button is clicked in Query Tab
        private void Query_GetFiles(object sender, RoutedEventArgs e)
        {
            if (CategoryQuery.Text == "" && FileQuery.Text == "" && ChildQuery.Text == "" && VersionQuery.Text == "" && NameQuery.Text == "" && DescQuery.Text == "")
                statusBarText.Text = "Status: Please give atleast one criterion";
            else
            {
                statusBarText.Text = "Status: List of files for given query";
                queryGetFiles(NameQuery.Text, DescQuery.Text, CategoryQuery.Text, FileQuery.Text, ChildQuery.Text, VersionQuery.Text);
                NameQuery.Clear();
                DescQuery.Clear();
                CategoryQuery.Clear();
                FileQuery.Clear();
                ChildQuery.Clear();
                VersionQuery.Clear();
            }
        }

        //set actions for when Get files with no parents button is clicked in Query Tab
        private void Query_NoParents(object sender, RoutedEventArgs e)
        {
                statusBarText.Text = "Status: List of files that have no parents";
                queryNoParents();
        }

        //strip off initial dots from the path
        private string removeFirstDir(string path)
        {
            string modifiedPath = path;
            int pos = path.LastIndexOf(".");
            modifiedPath = path.Substring(pos + 1, path.Length - pos - 1);
            return modifiedPath;
        }

    //----< respond to mouse double-click on dir name for CheckIn >----------------
    private void DirList_MouseDoubleClick(object sender, MouseButtonEventArgs e)
    {
      // build path for selected dir
      string selectedDir = (string)DirList.SelectedItem;
      string path;
      if(selectedDir == "..")
      {
        if (pathStack_.Count > 1) 
          pathStack_.Pop();
        else
          return;
      }
      else
      {
        path = pathStack_.Peek() + "/" + selectedDir;
        pathStack_.Push(path);
      }
      // display path in Dir TextBlcok
      PathTextBlock.Text = removeFirstDir(pathStack_.Peek());

      clearDirs();
      string[] listOfDirs = Directory.GetDirectories(pathStack_.Peek());
      foreach (string dir in listOfDirs)
      {
          addDir(System.IO.Path.GetFileName(dir));
      }
      insertParent();

      clearFiles();
      string[] listOfFiles = Directory.GetFiles(pathStack_.Peek());
      foreach (string file in listOfFiles)
      {
          addFile(System.IO.Path.GetFileName(file));
      }
    }

        //----< respond to mouse double-click on dir name for CheckOut >----------------
        private void DirListCheckout_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            // build path for selected dir
            string selectedDir = (string)DirListCheckout.SelectedItem;
            string path;
            if (selectedDir == "..")
            {
                if (pathStackCheckout_.Count > 1)  
                    pathStackCheckout_.Pop();
                else
                    return;
            }
            else
            {
                path = pathStackCheckout_.Peek() + "/" + selectedDir;
                pathStackCheckout_.Push(path);
            }
            // display path in Dir TextBlcok
            PathTextBlockCheckout.Text = removeFirstDir(pathStackCheckout_.Peek());

            // build message to get dirs and post it
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "getDirs");
            msg.add("path", pathStackCheckout_.Peek());
            translater.postMessage(msg);

            // build message to get files and post it
            msg.remove("command");
            msg.add("command", "getFiles");
            translater.postMessage(msg);
        }

        //----< respond to mouse double-click on dir name for Browse >----------------
        private void DirListBrowse_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            // build path for selected dir
            string selectedDir = (string)DirListBrowse.SelectedItem;
            string path;
            if (selectedDir == "..")
            {
                if (pathStackBrowse_.Count > 1)  
                    pathStackBrowse_.Pop();
                else
                    return;
            }
            else
            {
                path = pathStackBrowse_.Peek() + "/" + selectedDir;
                pathStackBrowse_.Push(path);
            }
            // display path in Dir TextBlcok
            PathTextBlockBrowse.Text = removeFirstDir(pathStackBrowse_.Peek());

            // build message to get dirs and post it
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "getDirs");
            msg.add("path", pathStackBrowse_.Peek());
            translater.postMessage(msg);

            // build message to get files and post it
            msg.remove("command");
            msg.add("command", "getFiles");

            translater.postMessage(msg);
        }
    }
}
