///////////////////////////////////////////////////////////////////////
// Version.cpp   - Testing of Versioning operations                  //
// ver 1.0                                                           //
//                                                                   //
// Author: Sayali Naval, snaval@syr.edu                              //
// Source: Dr. Jim Fowcett                                           //
// Application: CSE687 Project #4 - Remote Code Repository           //
// Environment: C++                                                  //
///////////////////////////////////////////////////////////////////////

#include "Version.h"

#define VERSION_TEST
#ifdef VERSION_TEST
using namespace NoSqlDb;

int main()
{
	Version v1;
	v1.getLatestVersion("Process.cpp", '.');
	v1.removeVersion("Process.h.1", '.');
    return 0;
}
#endif
