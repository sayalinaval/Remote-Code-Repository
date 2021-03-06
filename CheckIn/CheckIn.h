///////////////////////////////////////////////////////////////////////
// CheckIn.h   - Functionalities used for Check in operations        //
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
*  Package contains one class, CheckIn, that contains checkIn functions for new file, checkIn status open, 
*  close and pending.
*
*  Required Files:
* -----------------
*  CheckIn.h, CheckIn.cpp
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
#include "../Version/Version.h"
#include "../Query/Query.h"

using namespace FileSystem;

namespace NoSqlDb
{
	//Defines all the functions used for checkin
	class CheckIn
	{
	public:
		DbCore<PayLoad> changeCheckinStatus(DbCore<PayLoad> db, std::string file);
		DbCore<PayLoad> copyNewFile(DbCore<PayLoad> db, std::string package, std::string file, std::string fileName, std::string fileDesc, std::string fileCategories, std::string fileDependencies);
		DbCore<PayLoad> overwriteExistingFile(DbCore<PayLoad> db, std::string key, std::string package, std::string file, std::string fileName, std::string fileDesc, std::string fileCategories, std::string fileDependencies);
		DbCore<PayLoad> copyWithNewVersion(DbCore<PayLoad> db, std::string package, DbElement<PayLoad> elem, std::string file, int k3, std::string fileName, std::string fileDesc, std::string fileCategories, std::string fileDependencies);
		DbCore<PayLoad> copyToRemote(DbCore<PayLoad> db, std::string file, std::string fileName, std::string fileDesc, std::string fileCategories, std::string fileDependencies);
	};

	//change the checkin status of a file to close or pending
	DbCore<PayLoad> CheckIn::changeCheckinStatus(DbCore<PayLoad> db, std::string file)
	{
		Query<PayLoad> q1(db);
		Conditions<PayLoad> conds1;
		conds1.key(file);
		q1.select(conds1);
		int k3 = 0;
		std::string key;
		if (!q1.keys().empty())
		{
			for (auto k1 : q1.keys())
			{
				Version c;
				int k2 = c.getLatestVersion(k1, '.');
				if (k2 > k3)
					k3 = k2;
				key = k1;
			}
			DbElement<PayLoad> elem = db[key];
			bool canChange = true;
			for (auto child : elem.children())
			{
				DbElement<PayLoad> elem1 = db[child];
				if (elem1.payLoad().checkin_status() == "open")
					canChange = false;
			}
			//if check in status of all children is either close or pending,
			//change the check in status of parent to close
			if (canChange)
			{
				elem.payLoad().checkin_status("close");
				std::cout << "\nstatus changed to close\n";
				db[key] = elem;
			}
			//if check in status of any child is open,
			//change the check in status of parent to pending
			else
			{
				elem.payLoad().checkin_status("pending");
				std::cout << "\nstatus changed to pending\n";
				db[key] = elem;
			}
		}
		else
		{
			std::cout << "\nfile not present\n";
		}
		return db;
	}

	//copy the file to be checked in and give it version 1 to Remote Repo
	DbCore<PayLoad> CheckIn::copyNewFile(DbCore<PayLoad> db, std::string package, std::string file, std::string fileName, std::string fileDesc, std::string fileCategories, std::string fileDependencies)
	{
		std::string source = "../RemoteFiles/" + file;
		std::string file1 = file + ".1";
		std::string dest = "../RemoteRepo/" + package + "/" + file1;
		Directory d;
		d.create("../RemoteRepo/" + package);
		bool v = File::copy(source, dest);
		if (v)
		{
			std::cout << file << " checked in successfully\n";
			std::string key1 = package + "::" + file1;
			DbElement<PayLoad> elem;
			if (fileName.length() != 0)
				elem.name(fileName);
			if (fileDesc.length() != 0)
				elem.descrip(fileDesc);
			elem.payLoad().value("../RemoteRepo/" + package + "/");
			if (fileCategories.length() != 0)
			{
				size_t pos = 0;
				std::string token;
				while ((pos = fileCategories.find("-")) != std::string::npos)
				{
					token = fileCategories.substr(0, pos);
					elem.payLoad().categories().push_back(token);
					fileCategories.erase(0, pos + 1);
				}
				elem.payLoad().categories().push_back(fileCategories);
			}
			if (fileDependencies.length() != 0)
			{
				size_t pos = 0;
				std::string token;
				while ((pos = fileDependencies.find("-")) != std::string::npos)
				{
					token = fileDependencies.substr(0, pos);
					elem.children().push_back(token);
					fileDependencies.erase(0, pos + 1);
				}
				elem.children().push_back(fileDependencies);
			}
			elem.payLoad().checkin_status("open");
			db[key1] = elem;
    		}
		else
			std::cout << file << " check in failed\n";
		return db;
	}

	//overwrite the latest version of the file with the file to be checked in to Remote Repo
	DbCore<PayLoad> CheckIn::overwriteExistingFile(DbCore<PayLoad> db, std::string key, std::string package, std::string file, std::string fileName, std::string fileDesc, std::string fileCategories, std::string fileDependencies)
	{
		std::string source = "../RemoteFiles/" + file;
		std::string dest = "../RemoteRepo/" + package + "/" + key;
		bool v = File::copy(source, dest);
		if (v)
		{
			std::cout << file << " checked in successfully\n";
			DbElement<PayLoad> elem = db[package + "::" + key];
			if (fileName.length() != 0)
				elem.name(fileName);
			if (fileDesc.length() != 0)
				elem.descrip(fileDesc);
			if (fileCategories.length() != 0)
			{
				size_t pos = 0;
				std::string token;
				while ((pos = fileCategories.find("-")) != std::string::npos)
				{
					token = fileCategories.substr(0, pos);
					elem.payLoad().categories().push_back(token);
					fileCategories.erase(0, pos + 1);
				}
				elem.payLoad().categories().push_back(fileCategories);
			}
			if (fileDependencies.length() != 0)
			{
				size_t pos = 0;
				std::string token;
				while ((pos = fileDependencies.find("-")) != std::string::npos)
				{
					token = fileDependencies.substr(0, pos);
					elem.children().push_back(token);
					fileDependencies.erase(0, pos + 1);
				}
				elem.children().push_back(fileDependencies);
			}
			db[package + "::" + key] = elem;
		}
		else
			std::cout << file << " check in failed\n";
		return db;
	}

	//copy the file to be checked in with a new version of that file in Remote Repo
	DbCore<PayLoad> CheckIn::copyWithNewVersion(DbCore<PayLoad> db, std::string package, DbElement<PayLoad> elem, std::string file, int k3, std::string fileName, std::string fileDesc, std::string fileCategories, std::string fileDependencies)
	{
		std::string source = "../RemoteFiles/" + file;
		std::string file1 = file + "." + std::to_string(k3 + 1);
		std::string dest = "../RemoteRepo/" + package + "/" + file1;
		bool v = File::copy(source, dest);
		if (v)
		{
			std::cout << file << " checked in successfully\n";
			if (fileName.length() != 0)
				elem.name(fileName);
			if (fileDesc.length() != 0)
				elem.descrip(fileDesc);
			if (fileCategories.length() != 0)
			{
				size_t pos = 0;
				std::string token;
				while ((pos = fileCategories.find("-")) != std::string::npos)
				{
					token = fileCategories.substr(0, pos);
					elem.payLoad().categories().push_back(token);
					fileCategories.erase(0, pos + 1);
				}
				elem.payLoad().categories().push_back(fileCategories);
			}
			if (fileDependencies.length() != 0)
			{
				size_t pos = 0;
				while ((pos = fileDependencies.find("-")) != std::string::npos)
				{
					std::string token = fileDependencies.substr(0, pos);
					elem.children().push_back(token);
					fileDependencies.erase(0, pos + 1);
				}
				elem.children().push_back(fileDependencies);
			}
			elem.payLoad().checkin_status("open");
			db[package + "::" + file1] = elem;
			Query<PayLoad> q2(db);
			Conditions<PayLoad> c1;
			Keys keys = { package + "::" + file + "." + std::to_string(k3) };
			c1.children(keys);
			q2.select(c1);
			for (auto k1 : q2.keys())
				db[k1].children().push_back(package + "::" + file1);
		}
		else
			std::cout << file << " check in failed\n";
		return db;
	}

	//check if the file to be checked in exists in DB and if does, check it's check in status
	//and check in the file accordingly
	DbCore<PayLoad> CheckIn::copyToRemote(DbCore<PayLoad> db, std::string file, std::string fileName, std::string fileDesc, std::string fileCategories, std::string fileDependencies)
	{
		std::cout << "\n\n";
		Query<PayLoad> q1(db);
		Conditions<PayLoad> conds1;
		std::string file2 = "\\b(" + file + ")([^ ]*)";
		conds1.key(file2);
		q1.select(conds1);
		Version v1;
		std::string package = v1.getPackageName(file);
		std::string f1 = v1.getFileName(file);
		if (q1.keys().empty())
		{
			//this function is called if file to be checked in is not present in remote repo
			db=copyNewFile(db,package,f1,fileName,fileDesc,fileCategories,fileDependencies);
		}
		else
		{
			int k3 = 0;
			std::string key;
			for (auto k1 : q1.keys())
			{
				int k2 = v1.getLatestVersion(k1, '.');
				if (k2 > k3)
					k3 = k2;
					key = k1;
			}
			DbElement<PayLoad> elem = db[key];
			Version v1;
			std::string k1 = v1.getFileName(key);
			if (elem.payLoad().checkin_status() == "open")
			{
				//this function is called if file to be checked in is present in remote repo
				//and it's check in status is open
				db = overwriteExistingFile(db, k1, package, f1, fileName, fileDesc, fileCategories, fileDependencies);
			}
			else
			{
					//this function is called if file to be checked in is present in remote repo
					//and it's check in status is close or pending
					db=copyWithNewVersion(db,package,elem,f1,k3, fileName, fileDesc, fileCategories, fileDependencies);
			}
		}
		return db;
	}
}
