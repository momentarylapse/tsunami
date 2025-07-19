/*----------------------------------------------------------------------------*\
|  net                                                                      |
| -> network functions                                                         |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2008.12.13 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#include "net.h"
#include "../os/msg.h"

static bool NetInitialized = false;


void NetInit() {
#ifdef USE_WINSOCK
	if (WSAStartup(MAKEWORD(1,1),&wsaData)!=0) {
		msg_error("WSAStartup  (Network....)");
	}
#endif
	NetInitialized = true;
}


#define MAX_REPORT	16384

void _cdecl NetSendBugReport(const string &sender, const string &program, const string &version, const string &comment) {
	NetInit();

	auto s = ownify(net::connect("michisoft.michi.is-a-geek.org", 80));
	string temp, report;

	// actual data to send
	report = "program=" + program + "&";
	report += "version=" + version + "&";
	report += "sender=" + sender + "&";//str_m2ascii(sender)));
	report += "comment=" + comment + "&";//str_m2ascii(comment)));
	report += "report=";
	report += msg_get_buffer(MAX_REPORT);



	// http header to wrap around
	temp += "POST /report.php HTTP/1.1\r\n";
	temp += "Host: michisoft.michi.is-a-geek.org\r\n";
	temp += "Content-Type: application/x-www-form-urlencoded\r\n";
	temp += format("Content-Length: %d\r\n", report.num);
	temp += "\r\n";
	temp += report;

	// try and send
	try {
		s->write(temp);
	} catch (SocketError &e) {
		throw Exception("Server accepted a connection but could not send any data");
	}

	// get response
	try {
		temp = s->read(1024);
	} catch(SocketError &e) {
		throw Exception("Server does not respond");
	}
	if (temp.find(" 200 OK") >= 0) {
		if (temp.find("report: ok") >= 0)
			return;
		throw Exception("server could not save report to database :----(");
	}
	throw Exception("bad server http response: " + temp);
}
