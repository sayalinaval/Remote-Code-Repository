///////////////////////////////////////////////////////////////////////
// CheckOut.h   - Functionalities used for Check out operations      //
// ver 1.1                                                           //
//                                                                   //
// Author: Sayali Naval, snaval@syr.edu                              //
// Source: Dr. Jim Fowcett                                           //
// Application: CSE687 Project #4 - Remote Code Repository           //
// Environment: C++                                                  //
///////////////////////////////////////////////////////////////////////
/*
*  Package Operations:
* ---------------------
*  Package contains one class, CheckOut, that contains checkOut functions to checkOut a file and its children
*  to local client.
*
*  Required Files:
* -----------------
*  CheckOut.h, CheckOut.cpp
*  DateTime.h, DateTime.cpp
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

#pragma once

#include <stdio.h>
#include <tchar.h>
#include <string>
#include <iostream>
#include "../PayLoad/PayLoad.h"
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"
#include "../CppCommWithFileXfer/Message/Message.h"
#include "../CppCommWithFileXfer/MsgPassingComm/Comm.h"
#include "../Version/Version.h"

using namespace FileSystem;

namespace NoSqlDb
{
	//Defines the functions used for checkout
	class CheckOut
	{
	public:
		void copyToLocal(std::string file, std::string client);
	};

	//remove version number from file and check out to local client
	void CheckOut::copyToLocal(std::string file, std::string client)
	{
		std::cout << "\n\n";
		Version v1;
		std::string package = v1.getPackageName(file);
		std::string f1 = v1.getFileName(file);
		std::string source = client + "/" + f1;
		std::string f2 = v1.removeVersion(f1, '.');
		std::string local;
		if (client == "../SaveFiles")
			local = "../Storage";
		else
			local = "../LocalRepo";
		std::string dest = local + "/" + package + "/" + f2;
		Directory d;
		d.create(local + "/" + package);
		bool v = File::copy(source, dest);
		if (v)
			std::cout << file << " checked out successfully\n";
		else
			std::cout << file << " check out failed\n";
	}
}
