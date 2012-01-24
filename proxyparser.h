#pragma once
#include <string>

using namespace std;

class ProxySetting
{
	string protocol;
	string domain;
	unsigned int port;
};


class ProxyParser
{
public:
	ProxyParser(string url);
	~ProxyParser(void);

	static void getStaticProxySettingForUrl(string url, wstring proxylist, wstring proxybypass, ProxySetting & proxy);
	static bool testDomainForBypassList(string domain, wstring wproxybypass);
};
