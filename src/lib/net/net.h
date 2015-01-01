/*----------------------------------------------------------------------------*\
| net                                                                          |
| -> network functions                                                         |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2008.12.13 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _NET_INCLUDED_
#define _NET_INCLUDED_


#include "../base/base.h"

class vector;

class SocketException
{
};

class SocketConnectionLostException : public SocketException
{
};


class NetAddress
{
public:
	string host;
	int port;

	void __init__();
	void __delete__();
};

class Socket
{
public:
	Socket(int type);
	~Socket();

	bool _create(int port, bool block);
	bool _connect(const string &addr,int port);

	Socket *accept();
	void close();
	void setBlocking(bool blocking);
	bool isConnected();

	// send / receive directly
	string read();
	bool write(const string &buf);
	bool canWrite();
	bool canRead();

	// buffered read
	void operator>>(int &i);
	void operator>>(float &f);
	void operator>>(bool &b);
	void operator>>(char &c);
	void operator>>(string &s);
	void operator>>(vector &v);
	void _read_buffered_(void *p, int size);
	void setBufferPos(int pos);
	int getBufferPos();
	void clearBuffer();

	// buffered write
	void operator<<(int i);
	void operator<<(float f);
	void operator<<(bool b);
	void operator<<(char c);
	void operator<<(const string &s);
	void operator<<(const vector &v);
	bool writeBuffer();

	// kaba interface
	void __init__();
	void __delete__();

	int uid;
	int s;
	int type;
	string buffer;
	int buffer_pos;
	bool last_op_reading;

	// udp
	void setTarget(NetAddress &target);
	NetAddress getSender();
	NetAddress target;
	NetAddress sender;

	static const int TYPE_DUMMY;
	static const int TYPE_TCP;
	static const int TYPE_UDP;
};

void NetInit();

Socket *NetListen(int port, bool block);
Socket *NetConnect(const string &host, int port);
Socket *NetCreateUDP(int port);

// ...
bool NetSendBugReport(const string &sender, const string &program, const string &version, const string &comment, string &return_msg);

#endif
