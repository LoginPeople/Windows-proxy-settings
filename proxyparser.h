#pragma once
#include <string>
#include <vector>

using namespace std;

class ProxySetting
{
public:
	string protocol;
	string domain;
	unsigned int port;
};


class ProxyParser
{
public:
	ProxyParser(string url);
	~ProxyParser(void);

	static void getProxySettingForUrl(string url, ProxySetting & proxy);
	static void getStaticProxySettingForUrl(string url, wstring proxylist, wstring proxybypass, ProxySetting & proxy);
	static void getProxySettingForProtocolFromProxyList(string protocol, string proxyList, ProxySetting & proxy);
	static void getProxySettingForProxyListItem(string proxyItem, ProxySetting & proxy);
	static bool testHostForBypassList(string host, wstring wproxybypass);
	static bool testHostForBypass(string host, string bypass);
	static bool testDomainForBypass(string domain, string bypass);
	static bool testIpForBypass(string ip, string bypass);
	static vector<string> getIPForHost(string host);
	static bool isDomain(string host);

};
