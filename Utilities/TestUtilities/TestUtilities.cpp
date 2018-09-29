///////////////////////////////////////////////////////////////////////
// TestUtilities.cpp - provides single-user test harness             //
// ver 1.0                                                           //
// Language:    C++, Visual Studio 2017                              //
// Application: Most Projects, CSE687 - Object Oriented Design       //
// Author:      Jim Fawcett, Syracuse University, CST 4-187          //
//              jfawcett@twcny.rr.com                                //
///////////////////////////////////////////////////////////////////////

#include <cctype>
#include <iostream>
#include <functional>
#include "TestUtilities.h"
#include "../StringUtilities/StringUtilities.h"

#ifdef TEST_TESTUTILITIES

using namespace Utilities;

///////////////////////////////////////////////////////////////////////
// define demo tests

bool test_always_passes() { return true; }
bool test_always_fails() { return false; }
bool test_always_throws() {
  std::exception ex("exception\n         -- msg: this test always throws -- ");
  throw(ex);
}

int main()
{
  Title("Testing TestUtilities Package");
  putline();

  TestExecutive ex;

  TestExecutive::TestStr ts1;
  ts1.test(test_always_passes);
  ts1.testName("test_always_passes");

  TestExecutive::TestStr ts2;
  ts2.test(test_always_fails);
  ts2.testName("test_always_fails");

  TestExecutive::TestStr ts3;
  ts3.test(test_always_throws);
  ts3.testName("test_always_throws");
  
  ex.registerTest(ts1);
  ex.registerTest(ts2);
  ex.registerTest(ts3);

  bool result = ex.doTests();
  if (result == true)
    std::cout << "\n  all tests passed";
  else
    std::cout << "\n  at least one test failed";

  putline(2);
  return 0;
}
#endif
