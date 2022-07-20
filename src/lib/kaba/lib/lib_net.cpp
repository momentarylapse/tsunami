#include "../kaba.h"
#include "lib.h"
#include "../dynamic/exception.h"


#if __has_include("../../net/net.h")
	#include "../../net/net.h"
	#define KABA_EXPORT_NET
#endif

namespace kaba {

#ifdef KABA_EXPORT_NET
	#define net_p(p)		p
	static NetAddress *_addr;
	#define GetDAAddress(x)			int_p(&_addr->x)-int_p(_addr)
#else
	typedef int Socket;
	typedef int BinaryBuffer;
	typedef int NetAddress;
	#define net_p(p)		NULL
	#define GetDAAddress(x)			0
#endif

const Class *TypeNetAddress;
const Class *TypeSocket;
const Class *TypeSocketP;



#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")


Socket* __socket_listen__(int port, bool block) {
	KABA_EXCEPTION_WRAPPER( return Socket::listen(port, block); );
	return nullptr;
}

Socket* __socket_connect__(const string &host, int port) {
	KABA_EXCEPTION_WRAPPER( return Socket::connect(host, port); );
	return nullptr;
}

Socket* __socket_create_udp__(int port) {
	KABA_EXCEPTION_WRAPPER( return Socket::create_udp(port); );
	return nullptr;
}

#pragma GCC pop_options

void SIAddPackageNet() {
	add_package("net");

	TypeNetAddress  = add_type  ("NetAddress", sizeof(NetAddress));
	TypeSocket      = add_type  ("Socket", sizeof(Socket));
	TypeSocketP     = add_type_p(TypeSocket);
	auto TypeBinaryBuffer = add_type  ("BinaryBuffer", sizeof(BinaryBuffer));


	add_class(TypeNetAddress);
		class_add_element("host", TypeString, net_p(&NetAddress::host));
		class_add_element("port", TypeInt, net_p(&NetAddress::port));
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, net_p(&NetAddress::__init__));
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, net_p(&NetAddress::__delete__));

	add_class(TypeSocket);
		class_add_element("uid", TypeInt, net_p(&Socket::uid));
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, net_p(&Socket::__init__));
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, net_p(&Socket::__delete__));
		class_add_func("accept", TypeSocketP, net_p(&Socket::accept));
		class_add_func("close", TypeVoid, net_p(&Socket::close));
		class_add_func("set_blocking", TypeVoid, net_p(&Socket::set_blocking));
			func_add_param("block", TypeBool);
		class_add_func("set_target", TypeVoid, net_p(&Socket::set_target));
			func_add_param("target", TypeNetAddress);
		class_add_func("get_sender", TypeNetAddress, net_p(&Socket::get_sender));
		class_add_func("read", TypeString, net_p(&Socket::read));
			func_add_param("size", TypeInt);
		class_add_func("write", TypeBool, net_p(&Socket::write));
			func_add_param("buf", TypeString);
		class_add_func("can_read", TypeBool, net_p(&Socket::can_read));
		class_add_func("can_write", TypeBool, net_p(&Socket::can_write));
		class_add_func("is_connected", TypeBool, net_p(&Socket::is_connected));

		class_add_func("listen", TypeSocketP, net_p(&__socket_listen__), Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("port", TypeInt);
			func_add_param("block", TypeBool);
		class_add_func("connect", TypeSocketP, net_p(&__socket_connect__), Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("addr", TypeString);
			func_add_param("port", TypeInt);
		class_add_func("create_udp", TypeSocketP, net_p(&__socket_create_udp__), Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("port", TypeInt);

	add_class(TypeBinaryBuffer);
		class_add_element("data", TypeString, net_p(&BinaryBuffer::data));
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, net_p(&BinaryBuffer::__init__));
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, net_p(&BinaryBuffer::__delete__));
		class_add_func("__rshift__", TypeVoid, net_p((void(BinaryBuffer::*)(int&))&BinaryBuffer::operator>>));
			func_add_param("i", TypeInt, Flags::OUT);
		class_add_func("__rshift__", TypeVoid, net_p((void(BinaryBuffer::*)(float&))&BinaryBuffer::operator>>));
			func_add_param("f", TypeFloat32, Flags::OUT);
		class_add_func("__rshift__", TypeVoid, net_p((void(BinaryBuffer::*)(bool&))&BinaryBuffer::operator>>));
			func_add_param("b", TypeBool, Flags::OUT);
		class_add_func("__rshift__", TypeVoid, net_p((void(BinaryBuffer::*)(char&))&BinaryBuffer::operator>>));
			func_add_param("c", TypeChar, Flags::OUT);
		class_add_func("__rshift__", TypeVoid, net_p((void(BinaryBuffer::*)(string&))&BinaryBuffer::operator>>));
			func_add_param("s", TypeString, Flags::OUT);
		class_add_func("__rshift__", TypeVoid, net_p((void(BinaryBuffer::*)(vector&))&BinaryBuffer::operator>>));
			func_add_param("v", TypeVector, Flags::OUT);
		class_add_func("clear", TypeVoid, net_p(&BinaryBuffer::clear));
		class_add_func("start_block", TypeVoid, net_p(&BinaryBuffer::start_block));
		class_add_func("end_block", TypeVoid, net_p(&BinaryBuffer::end_block));
		class_add_func("set_pos", TypeVoid, net_p(&BinaryBuffer::set_pos));
			func_add_param("pos", TypeInt);
		class_add_func("get_pos", TypeInt, net_p(&BinaryBuffer::get_pos));
		class_add_func("__lshift__", TypeVoid, net_p((void(BinaryBuffer::*)(int))&BinaryBuffer::operator<<));
			func_add_param("i", TypeInt);
		class_add_func("__lshift__", TypeVoid, net_p((void(BinaryBuffer::*)(float))&BinaryBuffer::operator<<));
			func_add_param("f", TypeFloat32);
		class_add_func("__lshift__", TypeVoid, net_p((void(BinaryBuffer::*)(bool))&BinaryBuffer::operator<<));
			func_add_param("b", TypeBool);
		class_add_func("__lshift__", TypeVoid, net_p((void(BinaryBuffer::*)(char))&BinaryBuffer::operator<<));
			func_add_param("c", TypeChar);
		class_add_func("__lshift__", TypeVoid, net_p((void(BinaryBuffer::*)(const string &))&BinaryBuffer::operator<<));
			func_add_param("s", TypeString);
		class_add_func("__lshift__", TypeVoid, net_p((void(BinaryBuffer::*)(const vector &))&BinaryBuffer::operator<<));
			func_add_param("v", TypeVector);
}

};
