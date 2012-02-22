#include "proxyparser.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Winhttp.h>
#include <Ws2tcpip.h>
#include <Winsock2.h>
#include <iostream>
#include <sstream>

bool ProxyParser::getProxySettingForUrl(string url, ProxySetting & proxy)
{
	WINHTTP_CURRENT_USER_IE_PROXY_CONFIG	ieProxyConfig;
	WINHTTP_AUTOPROXY_OPTIONS				autoProxyOptions;
	WINHTTP_PROXY_INFO						autoProxyInfo;	
	BOOL									autoProxy	= FALSE;
	bool									result = false;

	memset(&ieProxyConfig,0,sizeof(WINHTTP_CURRENT_USER_IE_PROXY_CONFIG));
	memset(&autoProxyOptions,0,sizeof(WINHTTP_AUTOPROXY_OPTIONS));
	memset(&autoProxyInfo,0,sizeof(WINHTTP_PROXY_INFO));

	if( WinHttpGetIEProxyConfigForCurrentUser( &ieProxyConfig ) ){			
		if( ieProxyConfig.fAutoDetect ){
		// WPAD 									
			autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
			autoProxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
			autoProxyOptions.fAutoLogonIfChallenged = TRUE;				
			autoProxy = TRUE;
			cout << "proxy configured with WPAD" << endl;
		}
		else if( ieProxyConfig.lpszAutoConfigUrl != NULL ){
		// Normal PAC file									
			autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
			autoProxyOptions.lpszAutoConfigUrl = ieProxyConfig.lpszAutoConfigUrl;
			autoProxy = TRUE;
			wcout << L"download PAC file from: " << ieProxyConfig.lpszAutoConfigUrl << endl;
		}
		else if ( ieProxyConfig.lpszProxy != NULL ){
			autoProxy = FALSE;
			wcout << L"hardcoded proxy address: "  << ieProxyConfig.lpszProxy << endl;
			if(ieProxyConfig.lpszProxyBypass != NULL)
				wcout << L"proxy bypass list: " << ieProxyConfig.lpszProxyBypass << endl;
			else
				wcout << L"proxy bypass list: NONE" << endl;
		}
	}

	if(autoProxy)
	{
		cout << "testing proxy autoconfiguration for this URL: " << url << endl;
		std::wstring wUrl(url.begin() , url.end());
		
		HINTERNET session = WinHttpOpen(0, // no agent string
                          WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                          WINHTTP_NO_PROXY_NAME,
                          WINHTTP_NO_PROXY_BYPASS,
                          WINHTTP_FLAG_ASYNC);

		autoProxy = WinHttpGetProxyForUrl( session, wUrl.c_str(), &autoProxyOptions, &autoProxyInfo );
		if (!autoProxy)
			cout << "WinHttpGetProxyForUrl failed with error: " << GetLastError() << endl;
		else
		{
			if (NULL != autoProxyInfo.lpszProxy)
			{
				wcout << L"got proxy list: " << autoProxyInfo.lpszProxy << endl;
				wstring wproxyList(autoProxyInfo.lpszProxy);
				string proxyList(wproxyList.begin(), wproxyList.end());

				if(proxyList.empty())
					result = false;
				else
				{
					getProxySettingForProtocolFromProxyList("http", proxyList, proxy);
					result = true;
				}

				GlobalFree(autoProxyInfo.lpszProxy);
				if(NULL != autoProxyInfo.lpszProxyBypass)
				{
					wcout << L"and proxy bypass list: " <<  autoProxyInfo.lpszProxyBypass << endl;
					GlobalFree(autoProxyInfo.lpszProxyBypass);
				}
				return result;
			}
		}
	}
	else
	{
		result = getStaticProxySettingForUrl(url, ieProxyConfig.lpszProxy, ieProxyConfig.lpszProxyBypass, proxy);
	}

	if(ieProxyConfig.lpszAutoConfigUrl != NULL)
		GlobalFree( ieProxyConfig.lpszAutoConfigUrl );
	if(ieProxyConfig.lpszProxy != NULL)
		GlobalFree( ieProxyConfig.lpszProxy );
	if(ieProxyConfig.lpszProxyBypass != NULL)
		GlobalFree( ieProxyConfig.lpszProxyBypass );

	return result;
}

void ProxyParser::getProxySettingForProtocolFromProxyList(string protocol, string proxyList, ProxySetting & proxy)
{
	size_t token, precedent_token = 0;
	token = proxyList.find(";");

	do
	{
		string proxyItem = proxyList.substr(precedent_token, token-precedent_token);
		getProxySettingForProxyListItem(proxyItem, proxy);
		cout << "found proxy setting wioth protocol: " << proxy.protocol << endl;
		if(proxy.protocol== protocol || proxy.protocol == "all")
			break;

		precedent_token = token+1;
		token = proxyList.find(";", precedent_token);//can be separated by whitespace too?

	}while(token != string::npos);
}

void ProxyParser::getProxySettingForProxyListItem(string proxyItem, ProxySetting & proxy)
{
	//proxy item strings: ([<scheme>=][<scheme>"://"]<server>[":"<port>])
	size_t proxyToken = proxyItem.find("=");

	if(proxyToken != string::npos)
		proxy.protocol = proxyItem.substr(0, proxyToken);
	else
		proxy.protocol = "all";

	proxyItem = proxyItem.erase(0, proxyToken+1);

	cout << "remaining proxy item: " <<proxyItem << endl;
	proxyToken = proxyItem.find("/");
	if(proxyToken != string::npos)
		proxyItem = proxyItem.erase(0, proxyToken+2);

	proxyToken = proxyItem.find(":");
	proxy.domain = proxyItem.substr(0, proxyToken);
	stringstream s(proxyItem.substr(proxyToken+1, string::npos));
	s >> proxy.port;
}

bool ProxyParser::getStaticProxySettingForUrl(string url, wstring wproxylist, wstring proxybypass, ProxySetting & proxy)
{
	URL_COMPONENTS components = {0};
	components.dwStructSize = sizeof(URL_COMPONENTS);
	wchar_t scheme[20] = {0};
	wchar_t domain[MAX_PATH] = {0};
	components.lpszScheme = scheme;
	components.dwSchemeLength = 20;
	components.lpszHostName = domain;
	components.dwHostNameLength = MAX_PATH;

	wstring wurl(url.begin(), url.end());

	WinHttpCrackUrl( wurl.c_str(), (DWORD)wurl.length(), 0, &components);

	wstring whost(domain);
	string host(whost.begin(), whost.end());
	//FIXME
	if(!testHostForBypassList(host, proxybypass))
	{
		string proxylist(wproxylist.begin(), wproxylist.end());
		wstring wscheme(scheme);
		string protocol(wscheme.begin(), wscheme.end());
		getProxySettingForProtocolFromProxyList(protocol, proxylist, proxy);
		return true;
	}
	else
		return false;
}

bool ProxyParser::testHostForBypassList(string host, wstring wproxybypass)
{
	string proxybypass(wproxybypass.begin(), wproxybypass.end());
	size_t token, precedent_token = 0;
	token = proxybypass.find(";");
	
	while(token != string::npos)
	{
		string bypass = proxybypass.substr(precedent_token, token-precedent_token);

		if(testHostForBypass(host, bypass))
			return true;

		precedent_token = token+1;
		token = proxybypass.find(";", precedent_token);

	}

	string bypass = proxybypass.substr(precedent_token, token);
	return testHostForBypass(host, bypass);
}

bool ProxyParser::testHostForBypass(string host, string bypass)
{
	cout << "testing domain \"" << host << "\" for bypass: " << bypass << " ";
	if(isDomain(host))
	{
		bool result = testDomainForBypass(host, bypass);
		if(result)
		{
			cout << " => MATCH" << endl;
			return true;
		}
		else
		{
			cout << " => NO MATCH" << endl;
			return false;
		}
	}
	else
	{
		bool result = testIpForBypass(host, bypass);
		if(result)
		{
			cout << " => MATCH" << endl;
			return true;
		}
		else
		{
			cout << " => NO MATCH" << endl;
			return false;
		}
	}
}

bool ProxyParser::testDomainForBypass(string domain, string bypass)
{
	if(isDomain(bypass))
	{
		if(domain == bypass)
			return true;

		if(bypass.at(0) == '*')
		{
			string suffix = bypass.substr(1, string::npos);
			size_t found = domain.rfind(suffix);
			return(found+suffix.length() == domain.length());//the suffix is the end of the domain
		}

		if(bypass == "<local>" && domain.find('.') == string::npos)
			return true;
	}
	else
	{
		vector<string> ips = getIPForHost(domain);
		vector<string>::iterator ipIt;

		for(ipIt = ips.begin(); ipIt != ips.end(); ipIt++)
		{
			string ip = *ipIt;
			if(testIpForBypass(ip, bypass))
				return true;
		}
	}

	return false;
}

bool ProxyParser::testIpForBypass(string ip, string bypass)
{
	if(!isDomain(bypass))
	{
		if(ip == bypass)
			return true;

		//special case for IP like 172.16*
		if(bypass.at(bypass.size()-1) == '*')
		{
			if(ip.substr(0, bypass.size()-1) == bypass.substr(0, bypass.size()-1))
				return true;
		}


		size_t token, iptoken, precedent_token = 0,  precedent_iptoken = 0;
		token = bypass.find(".");
		iptoken = ip.find(".");

		while(token != string::npos)
		{
			string bypassnum = bypass.substr(precedent_token, token-precedent_token);
			string ipnum = ip.substr(precedent_iptoken, iptoken-precedent_iptoken);

			if(!(bypassnum == ipnum || bypassnum == "*"))
				return false;

			precedent_token = token+1;
			precedent_iptoken = iptoken+1;
			token = bypass.find(".", precedent_token);
			iptoken = ip.find(".", precedent_iptoken);

		}
		string bypassnum = bypass.substr(precedent_token, token-precedent_token);
		string ipnum = ip.substr(precedent_iptoken, iptoken-precedent_iptoken);
		if(bypassnum == ipnum || bypassnum == "*")
			return true;

	}
	return false;
}

vector<string> ProxyParser::getIPForHost(string host)
{
	vector<string> ips;

	WSADATA wsaData;
	int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if(res != 0)
		return ips;

	struct addrinfo * result;
	res = getaddrinfo(host.c_str(), NULL, NULL, &result);
	if(res == 0)
	{
		struct addrinfo * current = result;
		do{
			struct sockaddr_in  *sockaddr_ipv4 = (struct sockaddr_in *) current->ai_addr;
			string ip = inet_ntoa(sockaddr_ipv4->sin_addr);

			ips.push_back(ip);

			current = current->ai_next;
		}while(current != NULL);

		freeaddrinfo(result);
	}
	WSACleanup();
	return ips;
}

bool ProxyParser::isDomain(string host)
{
	return (isalpha(host.at(0))
		|| (host.at(0) == '*' && host.at(1) == '.' && isalpha(host.at(2)))
		|| host == "<local>");
}
