#include "hardcodedsettingtest.h"
#include "proxyparser.h"
#include <string>

using namespace std;

void HardcodedSettingTestCase::testProxy(void)
{
	CPPUNIT_ASSERT( true );
}


void HardcodedSettingTestCase::compareHostToBypass()
{
	CPPUNIT_ASSERT( ProxyParser::testHostForBypass("www.loginpeople.com", "www.loginpeople.com") );
	CPPUNIT_ASSERT( ProxyParser::testHostForBypass("www.loginpeople.com", "*.loginpeople.com") );
	CPPUNIT_ASSERT( !ProxyParser::testHostForBypass("www.loginpeople.com", "loginpeople.com") );
	CPPUNIT_ASSERT( !ProxyParser::testHostForBypass("loginpeople", "*.loginpeople.com") );

	CPPUNIT_ASSERT( !ProxyParser::testHostForBypass("www.loginpeople.com", "<local>") );
	CPPUNIT_ASSERT( ProxyParser::testHostForBypass("loginpeople", "<local>") );
}