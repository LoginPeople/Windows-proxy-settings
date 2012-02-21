#include <iostream>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/config/SourcePrefix.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TextOutputter.h>

#include "hardcodedsettingtest.h"
#include "proxyparser.h"

using namespace std;

int main(int argc, char * argv[])
{
	if(argc > 1)
	{
		ProxySetting proxy;
		ProxyParser::getProxySettingForUrl(argv[1], proxy);
		cout << "chosen proxy: " << proxy.protocol << "://" << proxy.domain << ":" << proxy.port << endl;
		return 0;
	}

	CppUnit::TextUi::TestRunner runner;

	runner.setOutputter( new CppUnit::TextOutputter( 
                          &runner.result(),
                          std::cerr ) );


	runner.addTest( HardcodedSettingTestCase::suite() );

	runner.run();

	return 0;
}

