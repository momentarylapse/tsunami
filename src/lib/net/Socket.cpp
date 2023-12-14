/*
 * Socket.cpp
 *
 *  Created on: Jan 9, 2021
 *      Author: michi
 */


#include "net.h"
#include "Socket.h"
#include "../os/msg.h"

#if defined(OS_WINDOWS) || defined(OS_MINGW)
//#if defined(OS_WINDOWS)
#define USE_WINSOCK
#endif

#ifdef USE_WINSOCK
	#include <winsock.h>
	#pragma comment(lib,"wsock32.lib")

	static WSADATA wsaData;
#else //OS_LINUX/MAC
	#include <stdio.h>
	//#include <stdio.h>
	#include <string.h>
	#include <errno.h>
	#include <sys/fcntl.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <unistd.h>

	#include <netdb.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>


#endif


#define NET_DEBUG			0

float NetConnectTimeout=5.0f;

static int NetCurrentSocketID = 1;


void so(int dbg,const string &str) {
	if (dbg<=NET_DEBUG)
		msg_write(str);
}

void so(int dbg,int i) {
	if (dbg<=NET_DEBUG)
		msg_write(i);
}




void NetAddress::__init__() {
	new(this) NetAddress();
}

void NetAddress::__delete__() {
	this->NetAddress::~NetAddress();
}

Socket::Socket(Type _type) {
	s = -1;
	type = _type;
	last_op_reading = false;
	uid = NetCurrentSocketID ++;
}

Socket::~Socket() {
	if (s >= 0)
		close();
}

void Socket::__init__() {
	new(this) Socket(Type::TCP);
}

void Socket::__delete__() {
	this->Socket::~Socket();
}


void Socket::close() {
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

void Socket::set_blocking(bool blocking) {
#ifdef USE_WINSOCK
	unsigned long l = blocking ? 0 : 1;
	ioctlsocket(s, FIONBIO, &l);
#else
	fcntl(s, F_SETFL, blocking ? 0 : O_NONBLOCK);
#endif
}

int _NetCreateSocket_() {
	so(1,"socket...");
	int s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		so(0,"  -ERROR (CreateSocket)");
		throw SocketError("failed to create socket");
	}
	so(1,"  -ok");
	return s;
}

// host
void Socket::_create() {
	so(1,"socket...");
	if (type == Type::UDP)
		s = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	else
		s = ::socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		so(0,"  -ERROR (socket creation)");
		throw SocketError("failed to create socket");
	}
	so(1,"  -ok");
}

void Socket::_bind(int port) {
	struct sockaddr_in my_addr;
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = INADDR_ANY; // wait on any device

	so(1,"bind...");
	if (::bind(s, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1) {
		so(0,"  -ERROR (bind)");
		close();
		throw SocketError("failed to bind");
	}
	so(1,"  -ok");
}


void Socket::_listen() {
	so(1,"listen...");
	if (::listen(s, 1) == -1) {
		so(0,"  -ERROR (listen)");
		throw SocketError("failed to listen");
	}
	so(1,"  -ok");
}

Socket *Socket::listen(int port, bool block) {
	Socket *s = new Socket(Socket::Type::TCP);
	try {
		s->_create();
		s->set_blocking(block);
		s->_bind(port);
		s->_listen();
		return s;
	} catch (...) {
		delete s;
	}
	return nullptr;
}

// host
Socket *Socket::accept() {
//	so(1,"accept...");
	struct sockaddr_in remote_addr;
	int size = sizeof(remote_addr);
#ifdef USE_WINSOCK
	auto as = ::accept(s, (struct sockaddr *)&remote_addr, &size);
#else
	socklen_t len = (socklen_t)size;
	auto as = ::accept(s, (struct sockaddr *)&remote_addr, &len);
#endif

	if (as < 0) {
		//throw SocketException("failed to accept");
		return nullptr;
	}
	so(1,"accept...");
	so(1,"  -ok");

	Socket *con = new Socket(type);
	con->s = as;

	so(1,"  -client found");
#ifdef USE_WINSOCK
		so(1, inet_ntoa(remote_addr.sin_addr));//.s_addr));
#endif
	con->set_blocking(true);

	return con;
}

// client
void Socket::_connect(const string &addr, int port) {
	struct sockaddr_in host_addr;
	struct hostent *host;


	so(1,"GetHostByName...");
		so(1,addr);
	host = gethostbyname(addr.c_str());
	if (host == nullptr) {
		so(1,"  -ERROR (GetHostByName)");
		throw SocketError("host not found");
	}
	so(1,"  -ok");

	s = _NetCreateSocket_();
	so(2,format("s: %d\n",s));

	host_addr.sin_family = AF_INET;
	host_addr.sin_addr = *((struct in_addr *)host->h_addr);
	host_addr.sin_port = htons(port);


	set_blocking(false);

	so(1,"connect...");
	int status = ::connect(s, (struct sockaddr *)&host_addr, sizeof(host_addr));
	so(2,status);
	//so(-1,errno);
	//so(-1,EINPROGRESS);
	float ttt = 0;
	while(ttt < NetConnectTimeout) {
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
		if ((FD_ISSET(s,&w)>0) and (status>0)) {
			so(2,"test");
			so(2,status);
			struct sockaddr address;
#ifdef USE_WINSOCK
				int address_len = sizeof(address);
#else
				socklen_t address_len = sizeof(address);
#endif
			if (getpeername(s,&address,&address_len) < 0) {
				so(1,"peer name :(");
				ttt = NetConnectTimeout;
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
	if (ttt>0) {
		so(1,"  -ERROR (connect)");
#ifdef USE_WINSOCK
			so(0,WSAGetLastError());
#endif
		close();
		throw SocketError("could not connect");
	}

	/*if (connect(s, (struct sockaddr *)&host_addr, sizeof(host_addr))==-1) {
		so(0,"  -ERROR (connect)");
		#ifdef USE_WINSOCK
			so(0,WSAGetLastError());
		#endif
		NetClose(s);
		return -1;
	}else
		so(1,"  -ok");*/
	set_blocking(true);
}

Socket *Socket::connect(const string &addr, int port) {
	Socket *s = new Socket(Socket::Type::TCP);
	try {
		s->_connect(addr, port);
		return s;
	} catch (...) {
		msg_write("EEEE net");
		delete s;
		throw;
	}
	return nullptr;
}

Socket *Socket::create_udp(int port) {
	Socket *s = new Socket(Socket::Type::UDP);
	try {
		s->_create();
		s->set_blocking(true);
		if (port >= 0)
			s->_bind(port);
		return s;
	} catch (...) {
		delete s;
		throw;
	}
}

bool Socket::is_connected() {
	return (s >= 0);
}

char _net_temp_buf_[65536];

bytes Socket::read(int size) {
	if (s < 0)
		return "";

	int r;
	sockaddr_in addr;
#ifdef USE_WINSOCK
	int addr_len = sizeof(addr);
#else
	socklen_t addr_len = sizeof(addr);
#endif
	if (type == Type::UDP)
		r = recvfrom(s, _net_temp_buf_, min<int>(size, sizeof(_net_temp_buf_)), 0, (sockaddr*)&addr, &addr_len);
	else
		r = recv(s, _net_temp_buf_, min<int>(size, sizeof(_net_temp_buf_)), 0);

	if (r == -EWOULDBLOCK) {
		return "";
	}

	if (r < 0) {
		//msg_error("recv");
		close();
		throw SocketConnectionLostException();
	}
	if (type == Type::UDP) {
		sender.host = inet_ntoa(addr.sin_addr);
		sender.port = ntohs(addr.sin_port);
	}
	//msg_write("Read: " + string(_net_temp_buf_, r).hex(false));
	return bytes(_net_temp_buf_, r);
}

bool Socket::write(const bytes &buf) {
	if (s < 0)
		return false;

	sockaddr_in addr;
	int sent = 0;
	char *b = (char*)buf.data;
	//msg_write("Write: " + buf.hex(false));

	if (type == Type::UDP) {
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

	while (sent < buf.num) {
		int r;
		if (type == Type::UDP)
			r = sendto(s, b, buf.num - sent, 0, (sockaddr*)&addr, sizeof(addr));
		else
			r = send(s, b, buf.num - sent, 0);
		if (r <= 0) {
			//msg_error("send");
			close();
			throw SocketConnectionLostException();
		}
		b += r;
		sent += r;
	}
	return true;
}

void Socket::set_target(NetAddress &_target) {
	target = _target;
}

NetAddress Socket::get_sender() {
	return sender;
}



bool Socket::can_write() {
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

bool Socket::can_read() {
	if (s < 0)
		return false;
	fd_set r;
	FD_ZERO(&r);
	FD_SET((unsigned)s, &r);
	struct timeval t;
	t.tv_sec = 0;
	t.tv_usec = 10;
	select(s+1, &r, nullptr, nullptr, &t);
	//printf("can read... %d\n", FD_ISSET(s, &r));
	return (FD_ISSET(s, &r) > 0);
}


