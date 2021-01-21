/*
 * Socket.h
 *
 *  Created on: Jan 9, 2021
 *      Author: michi
 */

#ifndef SRC_LIB_NET_SOCKET_H_
#define SRC_LIB_NET_SOCKET_H_


#include "../base/pointer.h"

class NetAddress {
public:
	string host;
	int port;

	void __init__();
	void __delete__();
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

	//BinaryBuffer buffer;

	// send / receive directly
	string read(int size);
	bool write(const string &buf);
	bool can_write();
	bool can_read();

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


#endif /* SRC_LIB_NET_SOCKET_H_ */
