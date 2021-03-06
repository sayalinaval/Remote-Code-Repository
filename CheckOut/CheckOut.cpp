///////////////////////////////////////////////////////////////////////
// CheckOut.cpp   - Testing of Check out operations                  //
// ver 1.1                                                           //
//                                                                   //
// Author: Sayali Naval, snaval@syr.edu                              //
// Source: Dr. Jim Fowcett                                           //
// Application: CSE687 Project #4 - Remote Code Repository           //
// Environment: C++                                                  //
///////////////////////////////////////////////////////////////////////

#include "CheckOut.h"
#include "../ServerPrototype/ServerPrototype.h"
#define CHECKOUT_TEST
#ifdef CHECKOUT_TEST
using namespace NoSqlDb;

int main()
{
	DbProvider dbp;
	CheckOut c1;
	DbCore<PayLoad> db = dbp.db();
	c1.copyToLocal("Query::Query.h", "../SaveFiles");
	return 0;
}
#endif
