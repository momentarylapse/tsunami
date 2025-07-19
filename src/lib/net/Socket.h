/*
 * Socket.h
 *
 *  Created on: Jan 9, 2021
 *      Author: michi
 */

#ifndef SRC_LIB_NET_SOCKET_H_
#define SRC_LIB_NET_SOCKET_H_


#include "../base/pointer.h"

namespace net {

struct NetAddress {
	string host;
	int port;
};


class Socket : public Sharable<base::Empty> {
public:
	enum class Type {
		DUMMY,
		TCP,
		UDP
	};

	explicit Socket(Type type = Type::TCP);
	~Socket();

	void _create();
	void _bind(int port);
	void _listen();
	void _connect(const string& addr, int port);

	xfer<Socket> accept();
	void close();
	void set_blocking(bool blocking);
	bool is_connected();

	//BinaryBuffer buffer;

	// send / receive directly
	bytes read(int size);
	bool write(const bytes& buf);
	bool can_write();
	bool can_read();

	int uid;
	int s;
	Type type;
	bool last_op_reading;

	// udp
	void set_target(const NetAddress& target);
	NetAddress get_sender();
	NetAddress target;
	NetAddress sender;
};

xfer<Socket> listen(int port, bool block);
xfer<Socket> connect(const string& host, int port);
xfer<Socket> create_udp(int port);

}

#endif /* SRC_LIB_NET_SOCKET_H_ */
