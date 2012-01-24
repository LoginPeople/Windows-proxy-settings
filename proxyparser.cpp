#include "proxyparser.h"
#include <windows.h>
#include <Winhttp.h>
#include <iostream>

ProxyParser::ProxyParser(string url)
{
	WINHTTP_CURRENT_USER_IE_PROXY_CONFIG	ieProxyConfig;
	WINHTTP_AUTOPROXY_OPTIONS				autoProxyOptions;
	WINHTTP_PROXY_INFO						autoProxyInfo;	
	BOOL									autoProxy	= FALSE;

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
			//cout << "proxy bypass list will be ignored by UCK" << endl;
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
				if(NULL != autoProxyInfo.lpszProxyBypass)
					wcout << L"and proxy bypass list: " <<  autoProxyInfo.lpszProxyBypass << endl;
			}
		}
	}
	else
	{
		ProxySetting proxy;
		getStaticProxySettingForUrl(url, ieProxyConfig.lpszProxy, ieProxyConfig.lpszProxyBypass, proxy);
	}
}

ProxyParser::~ProxyParser(void)
{
}

void ProxyParser::getStaticProxySettingForUrl(string url, wstring proxylist, wstring proxybypass, ProxySetting & proxy)
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
	testDomainForBypassList(host, proxybypass);
}

bool ProxyParser::testDomainForBypassList(string domain, wstring wproxybypass)
{
	return false;
}