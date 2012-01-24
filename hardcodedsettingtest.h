#ifndef HARDCODEDSETTINGTEST_H
#define HARDCODEDSETTINGTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/config/SourcePrefix.h>

class HardcodedSettingTestCase : public CPPUNIT_NS::TestFixture
	{
	  CPPUNIT_TEST_SUITE( HardcodedSettingTestCase );
	  CPPUNIT_TEST( testProxy );
	  CPPUNIT_TEST( compareHostToBypass );
	  CPPUNIT_TEST_SUITE_END();

	protected:
		void testProxy();
		void compareHostToBypass();
	};

CPPUNIT_TEST_SUITE_REGISTRATION( HardcodedSettingTestCase );
#endif