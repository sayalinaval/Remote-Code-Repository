#pragma once
///////////////////////////////////////////////////////////////////////
// ServerPrototype.h - Console App that processes incoming messages  //
// ver 1.1                                                           //
// Author: Sayali Naval, snaval@syr.edu                              //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2018         //
// Application: CSE687 Project #4 - Remote Code Repository           //
// Environment: C++                                                  //
///////////////////////////////////////////////////////////////////////
/*
*  Package Operations:
* ---------------------
*  Package contains one class, Server, that contains a Message-Passing Communication
*  facility. It processes each message by invoking an installed callable object
*  defined by the message's command key.
*
*  Message handling runs on a child thread, so the Server main thread is free to do
*  any necessary background processing (none, so far).
*
*  Required Files:
* -----------------
*  ServerPrototype.h, ServerPrototype.cpp
*  Comm.h, Comm.cpp, IComm.h
*  Message.h, Message.cpp
*  FileSystem.h, FileSystem.cpp
*  Utilities.h
*
*  Maintenance History:
* ----------------------
*  ver 1.1 : 5/1/2018
*  - second release
*  ver 1.0 : 3/27/2018
*  - first release
*/
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <thread>
#include "../CppCommWithFileXfer/Message/Message.h"
#include "../CppCommWithFileXfer/MsgPassingComm/Comm.h"
#include "../CheckIn/CheckIn.h"
#include "../CheckOut/CheckOut.h"
#include "../Version/Version.h"
#include <windows.h>
#include <tchar.h>

using namespace NoSqlDb;

class DbProvider
{
public:
	DbCore<PayLoad>& db() { return db_; }
private:
	static DbCore<PayLoad> db_;
};

DbCore<PayLoad> DbProvider::db_;

namespace Repository
{
  using File = std::string;
  using Files = std::vector<File>;
  using Dir = std::string;
  using Dirs = std::vector<Dir>;
  using SearchPath = std::string;
  using Key = std::string;
  using Msg = MsgPassingCommunication::Message;
  using ServerProc = std::function<Msg(Msg)>;
  using MsgDispatcher = std::unordered_map<Key,ServerProc>;
  
  const SearchPath storageRoot = "../RemoteRepo";  // root for all server file storage
  const MsgPassingCommunication::EndPoint serverEndPoint("localhost", 8080);  // listening endpoint

  class Server
  {
  public:
    Server(MsgPassingCommunication::EndPoint ep, const std::string& name);
    void start();
    void stop();
    void addMsgProc(Key key, ServerProc proc);
	void checkOutMessage(Msg msg);
	void otherMsgCommands(Msg msg);
    void processMessages();
    void postMessage(MsgPassingCommunication::Message msg);
    MsgPassingCommunication::Message getMessage();
    static Dirs getDirs(const SearchPath& path = storageRoot);
    static Files getFiles(const SearchPath& path = storageRoot);
  private:
    MsgPassingCommunication::Comm comm_;
    MsgDispatcher dispatcher_;
    std::thread msgProcThrd_;
  };

  //----< initialize server endpoint and give server a name >----------
  inline Server::Server(MsgPassingCommunication::EndPoint ep, const std::string& name)
    : comm_(ep, name) {}

  //----< start server's instance of Comm >----------------------------
  inline void Server::start()
  {
    comm_.start();
  }

  //----< stop Comm instance >-----------------------------------------
  inline void Server::stop()
  {
    if(msgProcThrd_.joinable())
      msgProcThrd_.join();
    comm_.stop();
  }

  //----< pass message to Comm for sending >---------------------------
  inline void Server::postMessage(MsgPassingCommunication::Message msg)
  {
    comm_.postMessage(msg);
  }

  //----< get message from Comm >--------------------------------------
  inline MsgPassingCommunication::Message Server::getMessage()
  {
    Msg msg = comm_.getMessage();
    return msg;
  }

  //----< add ServerProc callable object to server's dispatcher >------
  inline void Server::addMsgProc(Key key, ServerProc proc)
  {
    dispatcher_[key] = proc;
  }

  //Process checkOutReceived message to checkOut files to local repo
  inline void Server::checkOutMessage(Msg msg)
  {
	  CheckOut c1;
	  std::string path = msg.value("path");
	  std::string first = path.substr(0, path.find_last_of('/'));
	  std::string file = path.substr(path.find_last_of('/') + 1, path.length());
	  std::string package = first.substr(first.find_last_of('/') + 1, first.length());
	  std::string toPath;
	  if (msg.value("port") == "8082")
		  toPath = "../SaveFiles";
	  else
		  toPath = "../Savefiles1";
	  DbProvider dbp;
	  DbCore<PayLoad> db = dbp.db();
	  c1.copyToLocal(package + "::" + file, toPath);
	  msg.show();

	  DbElement<PayLoad> elem = db[package + "::" + file];
	  if (!elem.children().empty())
	  {
		  std::cout << "\nCheckOut children of " << file << "\n";
		  for (auto child : elem.children())
		  {
			  Version v2;
			  std::string package1 = v2.getPackageName(child);
			  std::string c1 = v2.getFileName(child);
			  Msg reply = msg;
			  reply.to(msg.from());
			  reply.from(msg.to());
			  reply.command("checkOut");
			  std::string path = msg.value("path");
			  reply.file(c1);
			  reply.name(c1);
			  reply.attribute("path", "../RemoteRepo/" + package1 + "/" + c1);
			  reply.attribute("port", msg.value("port"));
			  reply.attribute("toPath", msg.value("toPath"));
			  reply.attribute("Sending file for check out", c1);
			  postMessage(reply);
		  }
	  }
  }

  //----< Process messages for other commands such as checkIn, checkOutReceived, metaDataReceived, fileReceived, etc >------
  inline void Server::otherMsgCommands(Msg msg)
  {
	  if (msg.to().port != msg.from().port)  // avoid infinite message loop
	  {
			  if (msg.command() == "checkOutReceived")
			  {
				  checkOutMessage(msg);
			  }
			  else
			  {
				  if (msg.command() == "metaDataReceived")
				  {
					  std::cout << "\nClient received meta data for file " << msg.name();
					  std::cout << "\nMeta Data: " << msg.value("data");
					  msg.show();
				  }
				  else
				  {
					  if (msg.command() == "fileReceived")
					  {
						  std::cout << "\nClient received the file " << msg.name();
						  msg.show();
					  }
					  else
					  {
						  msg.show();
						  Msg reply = dispatcher_[msg.command()](msg);
						  postMessage(reply);
					  }
				  }
			  }
	  }
	  else
		  std::cout << "\n  server attempting to post to self";
  }

  //----< start processing messages on child thread >------------------
  inline void Server::processMessages()
  {
    auto proc = [&]()
    {
      if (dispatcher_.size() == 0)
      {
        std::cout << "\n  no server procs to call";
        return;
      }
      while (true)
      {
        Msg msg = getMessage();
        std::cout << "\n  received message: " << msg.command() << " from " << msg.from().toString();
        if (msg.containsKey("verbose"))
        {
          std::cout << "\n";
          msg.show();
        }
        if (msg.command() == "serverQuit")
          break;
		otherMsgCommands(msg);
      }
      std::cout << "\n  server message processing thread is shutting down";
    };
    std::thread t(proc);
    std::cout << "\n  starting server thread to process messages";
    msgProcThrd_ = std::move(t);
  }
}