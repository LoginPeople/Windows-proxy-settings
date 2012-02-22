Windows proxy settings
=============

This is a library designed for code running on Windows, using the IE proxy settings but not the WinHTTP API for network communication.

Gathering the proxy settings is already a tedious process, but using the hardcoded settings to decide if an URL should be retrieved through a proxy is even harder.

This library gives a simple function call to handle all the cases (WPAD, proxy.pac file, ahrdcoded proxy settings, hradcoded proxy bypass list).


Usage
-------

    #include <proxyparser.h>
    ProxySetting proxy;
    string url = "http://www.loginpeople.com";
    if(ProxyParser::getProxySettingForUrl(argv[1], proxy))
        cout << "chosen proxy: domain=" << proxy.domain << " port=" << proxy.port << endl;
    else
        cout << "no proxy for this URL" << endl;

You need to link to Winhttp.lib and Ws2_32.lib to include this code in your project.

