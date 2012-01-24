#include <iostream>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/config/SourcePrefix.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TextOutputter.h>

#include "hardcodedsettingtest.h"

using namespace std;

int main(int argc, char * argv[])
{
	CppUnit::TextUi::TestRunner runner;


	runner.setOutputter( new CppUnit::TextOutputter( 
                          &runner.result(),
                          std::cerr ) );


	runner.addTest( HardcodedSettingTestCase::suite() );

	runner.run();

	return 0;
}

