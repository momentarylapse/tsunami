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
#include "../types/vector.h"


#ifdef OS_WINDOWS
	#include <winsock.h>
	#pragma comment(lib,"wsock32.lib")

	static WSADATA wsaData;
#endif
#ifdef OS_LINUX
	#include <stdio.h>
	//#include <stdio.h>
	#include <string.h>
	#include <sys/fcntl.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <unistd.h>

	#include <netdb.h>

#endif


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
	if (!msg_inited)
		msg_init();
#ifdef OS_WINDOWS
	if (WSAStartup(MAKEWORD(1,1),&wsaData)!=0){
		msg_error("WSAStartup  (Network....)");
	}
#endif
	NetInitialized = true;
}



Socket::Socket()
{
	__init__();
}

Socket::~Socket()
{
	__delete__();
}

void Socket::__init__()
{
	s = -1;
	buffer = new string;
	buffer_pos = 0;
	last_op_reading = false;
	uid = NetCurrentSocketID ++;
}

void Socket::__delete__()
{
	if (s >= 0)
		Close();
	delete(buffer);
}

Socket Socket::operator=(const Socket &other)
{
	s = other.s;
	return *this;
}

void Socket::__assign__(const Socket &other)
{
	*this = other;
}


void Socket::Close()
{
	if (s < 0)
		return;
	//so("close");
#ifdef OS_WINDOWS
	closesocket(s);
#endif
#ifdef OS_LINUX
	close(s);
#endif
	s = -1;
}

void Socket::SetBlocking(bool blocking)
{
#ifdef OS_WINDOWS
	unsigned long l = blocking ? 0 : 1;
	ioctlsocket(s, FIONBIO, &l);
#endif
#ifdef OS_LINUX
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
bool Socket::Create(int port)
{
	so(1,"socket...");
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0){
		so(0,"  -ERROR (socket creation)");
		return false;
	}else
		so(1,"  -ok");

	SetBlocking(true);

	struct sockaddr_in my_addr;
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = INADDR_ANY; /// An jedem Device warten

	so(1,"bind...");
	if (bind(s, (struct sockaddr *)&my_addr, sizeof(my_addr))==-1){
		so(0,"  -ERROR (bind)");
		Close();
		return false;
	}else
		so(1,"  -ok");

	so(1,"listen...");
	if (listen(s, 1) == -1){
		so(0,"  -ERROR (listen)");
		return false;
	}else
		so(1,"  -ok");

	return true;
}

// host
bool Socket::Accept(Socket &con)
{
//	so(1,"accept...");
	struct sockaddr_in remote_addr;
	int size = sizeof(remote_addr);
#ifdef OS_WINDOWS
	con.s = accept(s, (struct sockaddr *)&remote_addr, &size);
#endif
#ifdef OS_LINUX
	socklen_t len = *(socklen_t*)&size;
	con.s = accept(s, (struct sockaddr *)&remote_addr, &len);
#endif

	if (con.s < 0){
		//so("  -FEHLER");
		return false;
	}else{
		so(1,"accept...");
		so(1,"  -ok");
	}

	so(1,"  -client found");
	#ifdef OS_WINDOWS
		so(1, inet_ntoa(remote_addr.sin_addr));//.s_addr));
	#endif
	con.SetBlocking(true);

	return true;
}

// client
bool Socket::Connect(const string &addr,int port)
{
	struct sockaddr_in host_addr;
	struct hostent *host;


	so(1,"GetHostByName...");
		so(1,addr);
	host = gethostbyname(addr.c_str());
	if (host == NULL){
		so(1,"  -ERROR (GetHostByName)");
		return false;
	}else
		so(1,"  -ok");

	s = _NetCreateSocket_();
	so(2,format("s: %d\n",s));

	host_addr.sin_family = AF_INET;
	host_addr.sin_addr = *((struct in_addr *)host->h_addr);
	host_addr.sin_port = htons(port);


	SetBlocking(false);

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
		status=select(s+1,&r,&w,NULL,&t);
		so(2,format("select: %d  r=%d  w=%d",status,FD_ISSET(s,&r),FD_ISSET(s,&w)));
		if ((FD_ISSET(s,&w)>0)&&(status>0)){
			so(2,"test");
			so(2,status);
			struct sockaddr address;
			#ifdef OS_WINDOWS
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
		#ifdef OS_WINDOWS
			so(0,WSAGetLastError());
		#endif
		Close();
		return -1;
	}

	/*if (connect(s, (struct sockaddr *)&host_addr, sizeof(host_addr))==-1){
		so(0,"  -ERROR (connect)");
		#ifdef OS_WINDOWS
			so(0,WSAGetLastError());
		#endif
		NetClose(s);
		return -1;
	}else
		so(1,"  -ok");*/
	SetBlocking(true);

	return true;
}

bool Socket::IsConnected()
{
	return (s >= 0);
}

char _net_temp_buf_[65536];

string Socket::Read()
{
	if (s < 0)
		return "";
	int r = recv(s, _net_temp_buf_, sizeof(_net_temp_buf_), 0);
	if (r > 0){
		//msg_write("Read: " + string(_net_temp_buf_, r).hex(false));
		return string(_net_temp_buf_, r);
	}
	msg_error("recv");
	Close();
	return "";
}

bool Socket::Write(const string &buf)
{
	if (s < 0)
		return false;
	int sent = 0;
	char *b = (char*)buf.data;
	//msg_write("Write: " + buf.hex(false));
	while (sent < buf.num){
		int r = send(s, b, buf.num - sent, 0);
		if (r <= 0){
			msg_error("send");
			Close();
			return false;
		}
		b += r;
		sent += r;
	}
	return true;
}



bool Socket::CanWrite()
{
	if (s < 0)
		return false;
	fd_set w;
	FD_ZERO(&w);
	FD_SET(((unsigned)s),&w);
	struct timeval t;
	t.tv_sec=0;
	t.tv_usec=10;
	select(s+1, NULL, &w, NULL, &t);
	return (FD_ISSET(s, &w) > 0);
}

bool Socket::CanRead()
{
	if (s < 0)
		return false;
	fd_set r;
	FD_ZERO(&r);
	FD_SET(((unsigned)s),&r);
	struct timeval t;
	t.tv_sec=0;
	t.tv_usec=10;
	select(s+1, &r, NULL, NULL, &t);
	return (FD_ISSET(s, &r) > 0);
}


void Socket::_read_buffered_(void *p, int size)
{
	if (s < 0)
		return;
	last_op_reading = true;
	//msg_write(format("r s=%d  p=%d  b=%d", size, buffer_pos, buffer->num));
	while (size > (buffer->num - buffer_pos))
		*buffer += Read();
	memcpy(p, (char*)buffer->data + buffer_pos, size);
	buffer_pos += size;
}

int Socket::ReadInt()
{
	int i;
	_read_buffered_(&i, sizeof(i));
	return i;
}

float Socket::ReadFloat()
{
	float f;
	_read_buffered_(&f, sizeof(f));
	return f;
}

bool Socket::ReadBool()
{
	bool b;
	_read_buffered_(&b, sizeof(b));
	return b;
}

char Socket::ReadChar()
{
	char c;
	_read_buffered_(&c, sizeof(c));
	return c;
}

vector Socket::ReadVector()
{
	vector v;
	_read_buffered_(&v, sizeof(v));
	return v;
}

string Socket::ReadString()
{
	string s;
	int n = ReadInt();
	s.resize(n);
	_read_buffered_(s.data, n);
	return s;
}

void Socket::operator>>(int &i)
{	i = ReadInt();	}

void Socket::operator>>(float &f)
{	f = ReadFloat();	}

void Socket::operator>>(bool &b)
{	b = ReadBool();	}

void Socket::operator>>(char &c)
{	c = ReadChar();	}

void Socket::operator>>(string &s)
{	s = ReadString();	}

void Socket::operator>>(vector &v)
{	v = ReadVector();	}

int Socket::GetBufferPos()
{
	return buffer_pos;
}

void Socket::SetBufferPos(int pos)
{
	if (s < 0)
		return;
	while (pos > buffer->num)
		*buffer += Read();
	buffer_pos = pos;
}


#define _test_first_write_() \
	if (last_op_reading){ \
		last_op_reading = false; \
		buffer->clear(); \
		buffer_pos = 0; \
	}

void Socket::WriteInt(int i)
{
	_test_first_write_();
	*buffer += string((char*)&i, sizeof(i));
}
void Socket::WriteFloat(float f)
{
	_test_first_write_();
	*buffer += string((char*)&f, sizeof(f));
}
void Socket::WriteBool(bool b)
{
	_test_first_write_();
	*buffer += string((char*)&b, sizeof(b));
}
void Socket::WriteChar(char c)
{
	_test_first_write_();
	*buffer += string((char*)&c, sizeof(c));
}
void Socket::WriteString(const string &s)
{
	_test_first_write_();
	WriteInt(s.num);
	*buffer += s;
}

void Socket::WriteVector(const vector &v)
{
	_test_first_write_();
	*buffer += string((char*)&v, sizeof(v));
}

bool Socket::WriteBuffer()
{
	bool r = Write(*buffer);
	buffer->clear();
	buffer_pos = 0;
	return r;
}


#define MAX_REPORT	16384

bool _cdecl NetSendBugReport(const string &sender, const string &program, const string &version, const string &comment, string &return_msg)
{
	NetInit();

	Socket s;
	if (s.Connect("michisoft.michi.is-a-geek.org", 80)){
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
		if (!s.Write(temp)){
			return_msg = "Server accepted a connection but could not send any data";
			s.Close();
			return false;
		}

		// get response
		temp = s.Read();
		s.Close();
		if (temp.num == 0){
			return_msg = "Server does not respond";
			return false;
		}
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

