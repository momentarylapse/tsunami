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
#include "../base/pointer.h"

class vector;

class SocketError : public Exception {
public:
	SocketError() : Exception() {}
	SocketError(const string &msg) : Exception(msg) {}
};

class SocketConnectionLostException : public SocketError {};


class NetAddress {
public:
	string host;
	int port;

	void __init__();
	void __delete__();
};

class BinaryBuffer {
public:
	BinaryBuffer();

	string data;
	int pos;
	void set_pos(int pos);
	int get_pos();
	void clear();

	void read(void *p, int size);

	// read
	void operator>>(int &i);
	void operator>>(float &f);
	void operator>>(bool &b);
	void operator>>(char &c);
	void operator>>(string &s);
	void operator>>(vector &v);

	// write
	void operator<<(int i);
	void operator<<(float f);
	void operator<<(bool b);
	void operator<<(char c);
	void operator<<(const string &s);
	void operator<<(const vector &v);
};

class Socket : public Sharable<Empty> {
public:
	enum class Type {
		DUMMY,
		TCP,
		UDP
	};

	Socket(Type type);
	~Socket();

	void _create();
	void _bind(int port);
	void _listen();
	void _connect(const string &addr, int port);

	Socket *accept();
	void close();
	void set_blocking(bool blocking);
	bool is_connected();

	BinaryBuffer buffer;

	// send / receive directly
	string read();
	bool write(const string &buf);
	bool can_write();
	bool can_read();
	bool read_buffer(int size);

	void _read_buffered_(void *p, int size);
	bool write_buffer();



	void set_buffer_pos(int pos);
	int get_buffer_pos();
	void clear_buffer();

	// argh...
	int block_pos;
	void start_block();
	void end_block();

	// read
	void operator>>(int &i);
	void operator>>(float &f);
	void operator>>(bool &b);
	void operator>>(char &c);
	void operator>>(string &s);
	void operator>>(vector &v);

	// write
	void operator<<(int i);
	void operator<<(float f);
	void operator<<(bool b);
	void operator<<(char c);
	void operator<<(const string &s);
	void operator<<(const vector &v);

	// kaba interface
	void __init__();
	void __delete__();

	int uid;
	int s;
	Type type;
	bool last_op_reading;

	// udp
	void set_target(NetAddress &target);
	NetAddress get_sender();
	NetAddress target;
	NetAddress sender;



	static Socket *listen(int port, bool block);
	static Socket *connect(const string &host, int port);
	static Socket *create_udp(int port);
};

void NetInit();

// ...
void NetSendBugReport(const string &sender, const string &program, const string &version, const string &comment);

#endif
