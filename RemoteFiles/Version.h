///////////////////////////////////////////////////////////////////////
// Version.h   - Functionalities used for Versioning operations      //
// ver 1.0                                                           //
//                                                                   //
// Author: Sayali Naval, snaval@syr.edu                              //
// Source: Dr. Jim Fowcett                                           //
// Application: CSE687 Project 2-Software Repository Testbed         //
// Environment: C++                                                  //
///////////////////////////////////////////////////////////////////////

//Here we have Version class that defines all the functions related to Versioning functionality

#pragma once

#include <stdio.h>
#include <tchar.h>
#include <string>
#include <iostream>
#include "../PayLoad/PayLoad.h"
#include "../Query/Query.h"

namespace NoSqlDb
{
	//Defines the functions used for versioning
	class Version
	{
	public:
		int getLatestVersion(std::string s, char c);
		std::string removeVersion(std::string s, char c);
		std::string getPackageName(std::string file);
		std::string getFileName(std::string file);
	};

	//this function returns the latest version number of the given file
	int Version::getLatestVersion(std::string s, char c)
	{
		std::string::size_type i = s.find_last_of(c);
		std::string::size_type j = s.length();
		int v = stoi(s.substr(i + 1, j));
		return v;
	}

	//this function returns the file name after removing the version number from the given file
	std::string Version::removeVersion(std::string s, char c)
	{
		std::string::size_type j = s.find_last_of(c);
		std::string v = s.substr(0, j);
		return v;
	}

	//this function returns the namespace(package) name from the given namespace::file name
	std::string Version::getPackageName(std::string file)
	{
		std::string::size_type j = file.find_first_of(':');
		std::string v = file.substr(0, j);
		return v;
	}

	//this function returns the file name from the given namespace::file name
	std::string Version::getFileName(std::string file)
	{
		std::string::size_type i = file.find_last_of(':');
		std::string::size_type j = file.length();
		std::string v = file.substr(i + 1, j);
		return v;
	}
}


// TODO: reference additional headers your program requires here
