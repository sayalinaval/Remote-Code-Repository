/////////////////////////////////////////////////////////////////////////
// ServerPrototype.cpp - Console App that processes incoming messages  //
// ver 1.1                                                             //
// Author: Sayali Naval, snaval@syr.edu                                //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2018           //
// Application: CSE687 Project #4 - Remote Code Repository             //
// Environment: C++                                                    //
/////////////////////////////////////////////////////////////////////////

#include "ServerPrototype.h"
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"
#include <chrono>

namespace MsgPassComm = MsgPassingCommunication;

using namespace Repository;
using namespace FileSystem;
using Msg = MsgPassingCommunication::Message;

//Create DB records for files present in Remote Repository
void createDbRecords()
{
	std::cout << "\nCreate DB records for Process.cpp.1, Process.h.1, Query.cpp.1 and Query.h.1\n";
	DbProvider dbp;
	DbCore<PayLoad> db = dbp.db();
	DbElement<PayLoad> elem = makeElement<PayLoad>("Sayali Naval", "process package");
	elem.dateTime(DateTime("Sat Apr 30 12:50:18 2018"));
	PayLoad pl;
	std::string value = "../RemoteRepo/Process/";
	pl.value(value);
	pl.categories().push_back("cpp");
	pl.categories().push_back("process");
	pl.checkin_status("open");
	elem.payLoad(pl);
	db["Process::Process.cpp.1"] = elem;
	elem.dateTime(DateTime("Sat Apr 28 12:50:18 2018"));
	PayLoad pl1;
	pl1.value(value);
	pl1.categories().push_back("header");
	pl1.categories().push_back("process");
	pl1.checkin_status("open");
	elem.payLoad(pl1);
	db["Process::Process.h.1"] = elem;
	db["Process::Process.cpp.1"].children().push_back("Process::Process.h.1");
	elem = makeElement<PayLoad>("Sayali Naval", "query package");
	PayLoad pl3;
	std::string value1 = "../RemoteRepo/Query/";
	pl3.value(value1);
	pl3.categories().push_back("cpp");
	pl3.categories().push_back("query");
	pl3.checkin_status("open");
	elem.payLoad(pl3);
	db["Query::Query.cpp.1"] = elem;
	PayLoad pl4;
	pl4.value(value1);
	pl4.categories().push_back("header");
	pl4.categories().push_back("query");
	pl4.checkin_status("open");
	elem.payLoad(pl4);
	db["Query::Query.h.1"] = elem;
	db["Query::Query.cpp.1"].children().push_back("Query::Query.h.1");
	showDb(db);
	std::cout << "\n\n";
	elem.payLoad().showDb(db);
	std::cout << "\n\n";
	dbp.db() = db;
}

Files Server::getFiles(const Repository::SearchPath& path)
{
  return Directory::getFiles(path);
}

Dirs Server::getDirs(const Repository::SearchPath& path)
{
  return Directory::getDirectories(path);
}

//display message
template<typename T>
void show(const T& t, const std::string& msg)
{
  std::cout << "\n  " << msg.c_str();
  for (auto item : t)
  {
    std::cout << "\n    " << item.c_str();
  }
}

//send message to self
std::function<Msg(Msg)> echo = [](Msg msg) {
  Msg reply = msg;
  reply.to(msg.from());
  reply.command(msg.command());
  reply.from(msg.to());
  return reply;
};

//send file names from given path to the requesting client
std::function<Msg(Msg)> getFiles = [](Msg msg) {
  Msg reply;
  reply.to(msg.from());
  reply.from(msg.to());
  reply.command("getFiles");
  std::string path = msg.value("path");
  if (path != "")
  {
    std::string searchPath = storageRoot;
    if (path != ".")
      searchPath = searchPath + "\\" + path;
    Files files = Server::getFiles(searchPath);
    size_t count = 0;
    for (auto item : files)
    {
      std::string countStr = Utilities::Converter<size_t>::toString(++count);
      reply.attribute("file" + countStr, item);
    }
  }
  else
  {
    std::cout << "\n  getFiles message did not define a path attribute";
  }
  return reply;
};

//send directory names from given path to the requesting client
std::function<Msg(Msg)> getDirs = [](Msg msg) {
  Msg reply;
  reply.to(msg.from());
  reply.from(msg.to());
  reply.command("getDirs");
  std::string path = msg.value("path");
  if (path != "")
  {
    std::string searchPath = storageRoot;
	if (path != ".")
		searchPath = searchPath + "\\" + path;
    Files dirs = Server::getDirs(searchPath);
    size_t count = 0;
    for (auto item : dirs)
    {
      if (item != ".." && item != ".")
      {
        std::string countStr = Utilities::Converter<size_t>::toString(++count);
        reply.attribute("dir" + countStr, item);
      }
    }
  }
  else
  {
    std::cout << "\n  getDirs message did not define a path attribute";
  }
  return reply;
};

//send connected message to the requesting client
std::function<Msg(Msg)> connectRequest = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("connected");
	return reply;
};

//send checkInReady message to the requesting client
std::function<Msg(Msg)> checkInRequest = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("checkInReady");
	reply.name(msg.name());
	reply.attribute("Ready to checkin file", msg.name());
	return reply;
};

//send checkInReceived message to the requesting client
std::function<Msg(Msg)> checkIn = [](Msg msg) {
	std::string path = msg.value("path");
	std::string first = path.substr(0, path.find_last_of('/'));
	std::string file = path.substr(path.find_last_of('/') + 1, path.length());
	std::string package = first.substr(first.find_last_of('/') + 1, first.length());
	CheckIn c1;
	DbProvider dbp;
	DbCore<PayLoad> db = dbp.db();
	db = c1.copyToRemote(db, package + "::" + file, msg.value("fileName"), msg.value("fileDesc"), msg.value("fileCategories"), msg.value("fileDependencies"));
	dbp.db() = db;
	showDb(db);
	std::cout << "\n\n";
	PayLoad pl;
	pl.showDb(db);
	std::cout << "\n\n";
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("checkInReceived");
	reply.name(msg.name());
	return reply;
};

//send changeStatus message to the requesting client
std::function<Msg(Msg)> changeStatusRequest = [](Msg msg) {
	std::string path = msg.value("path");
	std::string package = path.substr(path.find_last_of('/') + 1, path.length());
	std::string file = package + "::" + msg.name();
	CheckIn c1;
	DbProvider dbp;
	DbCore<PayLoad> db = dbp.db();
	db = c1.changeCheckinStatus(db, file);
	dbp.db() = db;
	showDb(db);
	std::cout << "\n\n";
	PayLoad pl;
	pl.showDb(db);
	std::cout << "\n\n";
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("changeStatus");
	reply.attribute("CheckIn Status", db[file].payLoad().checkin_status());
	return reply;
};

//send checkInRequest message to the requesting client
std::function<Msg(Msg)> checkOutRequest = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("checkOut");
	std::string path = msg.value("path");
	reply.file(msg.name());
	reply.attribute("path", path + "/" + reply.file());
	reply.attribute("port", msg.value("port"));
	reply.attribute("toPath", msg.value("toPath"));
	reply.attribute("Sending file for check out", msg.name());
	return reply;
};

//send requested file to the requesting client at requested path
std::function<Msg(Msg)> viewRequest = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("viewFile");
	std::string path = msg.value("path");
	reply.file(msg.name());
	reply.attribute("path", path + "/" + reply.file());
	reply.attribute("Sending file to view", msg.name());
	return reply;
};

//send meta data of the requested file to the requesting client
std::function<Msg(Msg)> metaDataRequest = [](Msg msg) {
	std::string path = msg.value("path");
	std::string package = path.substr(path.find_last_of('/') + 1, path.length());
	std::string file = package + "::" + msg.name();
	DbProvider dbp;
	DbCore<PayLoad> db = dbp.db();
	DbElement<PayLoad> elem = db[file];
	std::string children = "";
	for (auto child : elem.children())
	{
		children = children + " " + child;
	}
	std::string categories = "";
	for (auto category : elem.payLoad().categories())
	{
		categories = categories + " " + category;
	}
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("metaData");
	reply.attribute("author", elem.name());
	reply.attribute("description", elem.descrip());
	reply.attribute("dateTime", elem.dateTime());
	reply.attribute("children", children);
	reply.attribute("value", elem.payLoad().value());
	reply.attribute("checkInStatus", elem.payLoad().checkin_status());
	reply.attribute("categories", categories);
	return reply;
};

//send file list from query result to the requesting client
std::function<Msg(Msg)> queryRequest = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("queryResult");

	DbCore<PayLoad> db;
	DbProvider dbp;
	db = dbp.db();

	Query<PayLoad> q1(db),q2(db),q3(db);
	Conditions<PayLoad> c1,c2;

	c1.name(msg.value("queryName"));
	c1.description(msg.value("queryDesc"));
	if (msg.value("queryChild") != "")
	{
		c2.key(msg.value("queryChild"));
		q3.select(c2);
		Keys keys = q3.keys();
		c1.children(keys);
	}
	c1.key(msg.value("queryFile"));
	c1.version(msg.value("queryVersion"));
	q1.select(c1);

	if (msg.value("queryCategory") != "")
	{
		std::string category = msg.value("queryCategory");
		auto hasCategory = [&category](DbElement<PayLoad>& elem) {
			return (elem.payLoad()).hasCategory(category);
		};
		q2.select(hasCategory);
		q1.query_or(q2);
	}
	q1.show();
	std::cout << "\n\n";

	size_t count = 0;
	for (auto item : q1.keys())
	{
		std::string countStr = Utilities::Converter<size_t>::toString(++count);
		reply.attribute("file" + countStr, item);
	}
	return reply;
};

//send list of files having no parents to the requesting client
std::function<Msg(Msg)> noParentRequest = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("noParentFiles");
	DbProvider dbp;
	DbCore<PayLoad> db = dbp.db();
	size_t count = 0;
	std::cout << "\nFiles with no parents are\n";
	for (auto key : db.keys())
	{
		Query<PayLoad> q1(db);
		Keys keys{ key };
		Conditions<PayLoad> c1;
		c1.children(keys);
		q1.select(c1);
		if (q1.keys().empty())
		{
			std::cout << key << "\n";
			std::string countStr = Utilities::Converter<size_t>::toString(++count);
			reply.attribute("file" + countStr, key);
		}
	}
	return reply;
};

int main()
{
  SetConsoleTitleA("Project4 Server Console");
  std::cout << "\n  Server Prototype";
  std::cout << "\n ==========================";
  std::cout << "\n";

  createDbRecords();

  Server server(serverEndPoint, "ServerPrototype");
  server.start();

  std::cout << "\n  Server message processing";
  std::cout << "\n ----------------------------";
  //setting message types for processing different messages received by server
  server.addMsgProc("echo", echo);
  server.addMsgProc("getFiles", getFiles);
  server.addMsgProc("getDirs", getDirs);
  server.addMsgProc("connectRequest", connectRequest);
  server.addMsgProc("checkInRequest", checkInRequest);
  server.addMsgProc("checkIn", checkIn);
  server.addMsgProc("changeStatusRequest", changeStatusRequest);
  server.addMsgProc("checkOutRequest", checkOutRequest);
  server.addMsgProc("metaDataRequest", metaDataRequest);
  server.addMsgProc("viewRequest", viewRequest);
  server.addMsgProc("queryRequest", queryRequest);
  server.addMsgProc("noParentRequest", noParentRequest);
  server.addMsgProc("serverQuit", echo);
  server.processMessages();
  
  std::cout << "\n  press enter to exit";
  std::cin.get();
  std::cout << "\n";

  //send quit message to server to exit server
  Msg msg(serverEndPoint, serverEndPoint);
  msg.command("serverQuit");
  server.postMessage(msg);
  server.stop();
  return 0;
}

