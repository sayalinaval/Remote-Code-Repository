///////////////////////////////////////////////////////////////////////
// CheckIn.cpp   - Testing of Check in operations                    //
// ver 1.1                                                           //
//                                                                   //
// Author: Sayali Naval, snaval@syr.edu                              //
// Source: Dr. Jim Fowcett                                           //
// Application: CSE687 Project #4 - Remote Code Repository           //
// Environment: C++                                                  //
///////////////////////////////////////////////////////////////////////

#include "CheckIn.h"
#include "../ServerPrototype/ServerPrototype.h"

#define CHECKIN_TEST
#ifdef CHECKIN_TEST
using namespace NoSqlDb;

int main()
{
	DbProvider dbp;
	DbCore<PayLoad> db = dbp.db();
	CheckIn c1;
	db = c1.copyToRemote(db, "Edit::Edit.cpp.1", "Sayali Naval", "edit package", "edit-cpp", "");
    return 0;
}
#endif

