/*----------------------------------------------------------------------------*\
|  net                                                                      |
| -> network functions                                                         |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2008.12.13 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#include "net.h"
#include "../file/file.h"
#include "../math/vector.h"

#if defined(OS_WINDOWS) || defined(OS_MINGW)
//#if defined(OS_WINDOWS)
#define USE_WINSOCK
#endif

#ifdef USE_WINSOCK
	#include <winsock.h>
	#pragma comment(lib,"wsock32.lib")

	static WSADATA wsaData;
#else //OS_LINUX
	#include <stdio.h>
	//#include <stdio.h>
	#include <string.h>
	#include <sys/fcntl.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <unistd.h>

	#include <netdb.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>


#endif

const int Socket::TYPE_DUMMY = 0;
const int Socket::TYPE_TCP = 1;
const int Socket::TYPE_UDP = 2;

#define NET_DEBUG			0

float NetConnectTimeout=5.0f;

static bool NetInitialized = false;
static int NetCurrentSocketID = 1;


void so(int dbg,const string &str)
{
	if (dbg<=NET_DEBUG)
		msg_write(str);
}

void so(int dbg,int i)
{
	if (dbg<=NET_DEBUG)
		msg_write(i);
}




void NetInit()
{
#ifdef USE_WINSOCK
	if (WSAStartup(MAKEWORD(1,1),&wsaData)!=0){
		msg_error("WSAStartup  (Network....)");
	}
#endif
	NetInitialized = true;
}


void NetAddress::__init__()
{
	new(this) NetAddress();
}

void NetAddress::__delete__()
{
	this->NetAddress::~NetAddress();
}


Socket::Socket(int _type)
{
	s = -1;
	type = _type;
	buffer_pos = 0;
	last_op_reading = false;
	uid = NetCurrentSocketID ++;
}

Socket::~Socket()
{
	if (s >= 0)
		close();
}

void Socket::__init__()
{
	new(this) Socket(TYPE_TCP);
}

void Socket::__delete__()
{
	this->Socket::~Socket();
}


void Socket::close()
{
	if (s < 0)
		return;
	//so("close");
#ifdef USE_WINSOCK
	closesocket(s);
#else
	::close(s);
#endif
	s = -1;
}

void Socket::setBlocking(bool blocking)
{
#ifdef USE_WINSOCK
	unsigned long l = blocking ? 0 : 1;
	ioctlsocket(s, FIONBIO, &l);
#else
	fcntl(s, F_SETFL, blocking ? 0 : O_NONBLOCK);
#endif
}

int _NetCreateSocket_()
{
	so(1,"socket...");
	int s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0){
		so(0,"  -ERROR (CreateSocket)");
		return -1;
	}else
		so(1,"  -ok");
	return s;
}

// host
bool Socket::_create(int port, bool block)
{
	so(1,"socket...");
	if (type == TYPE_UDP)
		s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	else
		s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0){
		so(0,"  -ERROR (socket creation)");
		return false;
	}else
		so(1,"  -ok");

	setBlocking(block);

	if ((type == TYPE_UDP) && (port < 0))
		return true;

	struct sockaddr_in my_addr;
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = INADDR_ANY; /// An jedem Device warten

	so(1,"bind...");
	if (bind(s, (struct sockaddr *)&my_addr, sizeof(my_addr))==-1){
		so(0,"  -ERROR (bind)");
		close();
		return false;
	}else
		so(1,"  -ok");

	if (type == TYPE_UDP)
		return true;

	so(1,"listen...");
	if (listen(s, 1) == -1){
		so(0,"  -ERROR (listen)");
		return false;
	}else
		so(1,"  -ok");

	return true;
}

Socket *NetListen(int port, bool block)
{
	Socket *s = new Socket(Socket::TYPE_TCP);
	if (s->_create(port, block))
		return s;
	delete(s);
	return nullptr;
}

// host
Socket *Socket::accept()
{
	Socket *con = new Socket(type);
//	so(1,"accept...");
	struct sockaddr_in remote_addr;
	int size = sizeof(remote_addr);
#ifdef USE_WINSOCK
	con->s = ::accept(s, (struct sockaddr *)&remote_addr, &size);
#else
	socklen_t len = (socklen_t)size;
	con->s = ::accept(s, (struct sockaddr *)&remote_addr, &len);
#endif

	if (con->s < 0){
		//so("  -FEHLER");
		delete(con);
		return nullptr;
	}else{
		so(1,"accept...");
		so(1,"  -ok");
	}

	so(1,"  -client found");
#ifdef USE_WINSOCK
		so(1, inet_ntoa(remote_addr.sin_addr));//.s_addr));
#endif
	con->setBlocking(true);

	return con;
}

// client
bool Socket::_connect(const string &addr,int port)
{
	struct sockaddr_in host_addr;
	struct hostent *host;


	so(1,"GetHostByName...");
		so(1,addr);
	host = gethostbyname(addr.c_str());
	if (host == nullptr){
		so(1,"  -ERROR (GetHostByName)");
		return false;
	}else
		so(1,"  -ok");

	s = _NetCreateSocket_();
	so(2,format("s: %d\n",s));

	host_addr.sin_family = AF_INET;
	host_addr.sin_addr = *((struct in_addr *)host->h_addr);
	host_addr.sin_port = htons(port);


	setBlocking(false);

	so(1,"connect...");
	int status=connect(s, (struct sockaddr *)&host_addr, sizeof(host_addr));
	so(2,status);
	//so(-1,errno);
	//so(-1,EINPROGRESS);
	float ttt = 0;
	while(ttt < NetConnectTimeout){
		//printf("%f\n",ttt);
		fd_set r,w;
		FD_ZERO(&r);
		FD_SET(((unsigned)s),&r);
		FD_ZERO(&w);
		FD_SET(((unsigned)s),&w);
		struct timeval t;
		t.tv_sec=0;
		t.tv_usec=10000;
		status=select(s+1,&r,&w,nullptr,&t);
		so(2,format("select: %d  r=%d  w=%d",status,FD_ISSET(s,&r),FD_ISSET(s,&w)));
		if ((FD_ISSET(s,&w)>0)&&(status>0)){
			so(2,"test");
			so(2,status);
			struct sockaddr address;
#ifdef USE_WINSOCK
				int address_len=sizeof(address);
#else
				socklen_t address_len=sizeof(address);
#endif
			if (getpeername(s,&address,&address_len)<0){
				so(1,"peer name :(");
				ttt=NetConnectTimeout;
				break;
			}else{
				//so(2,string2("peer: %d    %s",address_len,address.sa_data));
				//so(-1,"ok");
				//so(-1,s);
				ttt=-1;
				so(1,"  -ok");
				break;
			}
		}
		ttt+=0.01f;
	}
	if (ttt>0){
		so(1,"  -ERROR (connect)");
#ifdef USE_WINSOCK
			so(0,WSAGetLastError());
#endif
		close();
		return false;
	}

	/*if (connect(s, (struct sockaddr *)&host_addr, sizeof(host_addr))==-1){
		so(0,"  -ERROR (connect)");
		#ifdef USE_WINSOCK
			so(0,WSAGetLastError());
		#endif
		NetClose(s);
		return -1;
	}else
		so(1,"  -ok");*/
	setBlocking(true);

	return true;
}

Socket *NetConnect(const string &addr, int port)
{
	Socket *s = new Socket(Socket::TYPE_TCP);
	if (s->_connect(addr, port))
		return s;
	delete(s);
	return nullptr;
}

Socket *NetCreateUDP(int port)
{
	Socket *s = new Socket(Socket::TYPE_UDP);
	if (s->_create(port, true))
		return s;
	delete(s);
	return nullptr;
}

bool Socket::isConnected()
{
	return (s >= 0);
}

char _net_temp_buf_[65536];

string Socket::read()
{
	if (s < 0)
		return "";

	int r;
	sockaddr_in addr;
#ifdef USE_WINSOCK
	int addr_len = sizeof(addr);
#else
	socklen_t addr_len = sizeof(addr);
#endif
	if (type == TYPE_UDP)
		r = recvfrom(s, _net_temp_buf_, sizeof(_net_temp_buf_), 0, (sockaddr*)&addr, &addr_len);
	else
		r = recv(s, _net_temp_buf_, sizeof(_net_temp_buf_), 0);

	if (r <= 0){
		//msg_error("recv");
		close();
		throw SocketConnectionLostException();
	}
	if (type == TYPE_UDP){
		sender.host = inet_ntoa(addr.sin_addr);
		sender.port = ntohs(addr.sin_port);
	}
	//msg_write("Read: " + string(_net_temp_buf_, r).hex(false));
	return string(_net_temp_buf_, r);
}

bool Socket::write(const string &buf)
{
	if (s < 0)
		return false;

	sockaddr_in addr;
	int sent = 0;
	char *b = (char*)buf.data;
	//msg_write("Write: " + buf.hex(false));

	if (type == TYPE_UDP){
		memset((char *)&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(target.port);
#ifdef USE_WINSOCK
			msg_error("inet_aton() on windows...\n");
#else
		if (inet_aton(target.host.c_str(), &addr.sin_addr)==0)
			msg_error("inet_aton() failed\n");
#endif
	}

	while (sent < buf.num){
		int r;
		if (type == TYPE_UDP)
			r = sendto(s, b, buf.num - sent, 0, (sockaddr*)&addr, sizeof(addr));
		else
			r = send(s, b, buf.num - sent, 0);
		if (r <= 0){
			//msg_error("send");
			close();
			throw SocketConnectionLostException();
			return false;
		}
		b += r;
		sent += r;
	}
	return true;
}

void Socket::setTarget(NetAddress &_target)
{
	target = _target;
}

NetAddress Socket::getSender()
{
	return sender;
}



bool Socket::canWrite()
{
	if (s < 0)
		return false;
	fd_set w;
	FD_ZERO(&w);
	FD_SET((unsigned)s, &w);
	struct timeval t;
	t.tv_sec = 0;
	t.tv_usec = 10;
	select(s+1, nullptr, &w, nullptr, &t);
	return (FD_ISSET(s, &w) > 0);
}

bool Socket::canRead()
{
	if (s < 0)
		return false;
	fd_set r;
	FD_ZERO(&r);
	FD_SET((unsigned)s, &r);
	struct timeval t;
	t.tv_sec = 0;
	t.tv_usec = 10;
	select(s+1, &r, nullptr, nullptr, &t);
	return (FD_ISSET(s, &r) > 0);
}


void Socket::_read_buffered_(void *p, int size)
{
	if (s < 0)
		return;
	last_op_reading = true;
	//msg_write(format("r s=%d  p=%d  b=%d", size, buffer_pos, buffer.num));
	while (size > (buffer.num - buffer_pos)){
		buffer += read();
	}
	memcpy(p, (char*)buffer.data + buffer_pos, size);
	buffer_pos += size;
}

void Socket::operator>>(int &i)
{
	_read_buffered_(&i, sizeof(i));
}

void Socket::operator>>(float &f)
{
	_read_buffered_(&f, sizeof(f));
}

void Socket::operator>>(bool &b)
{
	_read_buffered_(&b, sizeof(b));
}

void Socket::operator>>(char &c)
{
	_read_buffered_(&c, sizeof(c));
}

void Socket::operator>>(string &s)
{
	int n;
	_read_buffered_(&n, sizeof(n));
	s.resize(n);
	_read_buffered_(s.data, n);
}

void Socket::operator>>(vector &v)
{
	_read_buffered_(&v, sizeof(v));
}

int Socket::getBufferPos()
{
	return buffer_pos;
}

void Socket::setBufferPos(int pos)
{
	if (s < 0)
		return;
	while (pos > buffer.num)
		buffer += read();
	buffer_pos = pos;
}


#define _test_first_write_() \
	if (last_op_reading){ \
		last_op_reading = false; \
		buffer.clear(); \
		buffer_pos = 0; \
	}

void Socket::operator<<(int i)
{
	_test_first_write_();
	buffer += string((char*)&i, sizeof(i));
}
void Socket::operator<<(float f)
{
	_test_first_write_();
	buffer += string((char*)&f, sizeof(f));
}
void Socket::operator<<(bool b)
{
	_test_first_write_();
	buffer += string((char*)&b, sizeof(b));
}
void Socket::operator<<(char c)
{
	_test_first_write_();
	buffer += string((char*)&c, sizeof(c));
}
void Socket::operator<<(const string &s)
{
	_test_first_write_();
	buffer += string((char*)&s.num, sizeof(int));
	buffer += s;
}

void Socket::operator<<(const vector &v)
{
	_test_first_write_();
	buffer += string((char*)&v, sizeof(v));
}

bool Socket::writeBuffer()
{
	bool r = write(buffer);
	clearBuffer();
	return r;
}

void Socket::clearBuffer()
{
	buffer.clear();
	buffer_pos = 0;
}


#define MAX_REPORT	16384

bool _cdecl NetSendBugReport(const string &sender, const string &program, const string &version, const string &comment, string &return_msg)
{
	NetInit();

	Socket *s = NetConnect("michisoft.michi.is-a-geek.org", 80);
	if (s){
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
		try{
			s->write(temp);
		}catch (SocketException &e){
			return_msg = "Server accepted a connection but could not send any data";
			delete(s);
			return false;
		}

		// get response
		try{
			temp = s->read();
		}catch(SocketException &e){
			return_msg = "Server does not respond";
			delete(s);
			return false;
		}
		delete(s);
		if (temp.find(" 200 OK") >= 0){
			if (temp.find("report: ok") >= 0){
				return_msg = "report successfully sent";
				return true;
			}else
				return_msg = "server could not save report to database :----(";
		}else
			return_msg = "bad server http response: " + temp;
	}else
		return_msg = "Could not connect to server";
	return false;
}
