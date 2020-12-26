#include "../../file/file.h"
#include "../kaba.h"
#include "../../config.h"
#include "common.h"
#include "exception.h"


#ifdef _X_USE_NET_
	#include "../../net/net.h"
#endif

namespace kaba {

#ifdef _X_USE_NET_
	#define net_p(p)		(void*)p
	static NetAddress *_addr;
	#define GetDAAddress(x)			int_p(&_addr->x)-int_p(_addr)
#else
	typedef int Socket;
	typedef int NetAddress;
	#define net_p(p)		NULL
	#define GetDAAddress(x)			0
#endif
#ifdef _X_ALLOW_X_
	#define x_p(p)		(void*)p
#else
	#define x_p(p)		NULL
#endif

extern const Class *TypeIntPs;
extern const Class *TypeFloatPs;
extern const Class *TypeBoolPs;
extern const Class *TypeCharPs;

const Class *TypeNetAddress;
const Class *TypeSocket;
const Class *TypeSocketP;
const Class *TypeSocketPList;



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
	TypeSocketPList = add_type_l(TypeSocketP);


	add_class(TypeNetAddress);
		class_add_element("host", TypeString, GetDAAddress(host));
		class_add_element("port", TypeInt, GetDAAddress(port));
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, net_p(mf(&NetAddress::__init__)));
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, net_p(mf(&NetAddress::__delete__)));

	add_class(TypeSocket);
		class_add_elementx("uid", TypeInt, &Socket::uid);
		class_add_elementx("_buffer", TypeString, &Socket::buffer);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, net_p(mf(&Socket::__init__)));
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, net_p(mf(&Socket::__delete__)));
		class_add_func("accept", TypeSocketP, net_p(mf(&Socket::accept)));
		class_add_func("close", TypeVoid, net_p(mf(&Socket::close)));
		class_add_func("set_blocking", TypeVoid, net_p(mf(&Socket::set_blocking)));
			func_add_param("block", TypeBool);
		class_add_func("set_target", TypeVoid, net_p(mf(&Socket::set_target)));
			func_add_param("target", TypeNetAddress);
		class_add_func("get_sender", TypeNetAddress, net_p(mf(&Socket::get_sender)));
		class_add_func("read", TypeString, net_p(mf(&Socket::read)));
		class_add_func("write", TypeBool, net_p(mf(&Socket::write)));
			func_add_param("buf", TypeString);
		class_add_func("can_read", TypeBool, net_p(mf(&Socket::can_read)));
		class_add_func("can_write", TypeBool, net_p(mf(&Socket::can_write)));
		class_add_func("is_connected", TypeBool, net_p(mf(&Socket::is_connected)));
		class_add_func("__rshift__", TypeVoid, net_p(mf((void(Socket::*)(int&))&Socket::operator>>)));
			func_add_param("i", TypeIntPs);
		class_add_func("__rshift__", TypeVoid, net_p(mf((void(Socket::*)(float&))&Socket::operator>>)));
			func_add_param("f", TypeFloatPs);
		class_add_func("__rshift__", TypeVoid, net_p(mf((void(Socket::*)(bool&))&Socket::operator>>)));
			func_add_param("b", TypeBoolPs);
		class_add_func("__rshift__", TypeVoid, net_p(mf((void(Socket::*)(char&))&Socket::operator>>)));
			func_add_param("c", TypeCharPs);
		class_add_func("__rshift__", TypeVoid, net_p(mf((void(Socket::*)(string&))&Socket::operator>>)));
			func_add_param("s", TypeString);
		class_add_func("__rshift__", TypeVoid, net_p(mf((void(Socket::*)(vector&))&Socket::operator>>)));
			func_add_param("v", TypeVector);
		class_add_func("write_buffer", TypeBool, net_p(mf(&Socket::write_buffer)));
		class_add_func("read_buffer", TypeBool, net_p(mf(&Socket::read_buffer)));
			func_add_param("size", TypeInt);
		class_add_func("clear_buffer", TypeVoid, net_p(mf(&Socket::clear_buffer)));
		class_add_func("start_block", TypeVoid, net_p(mf(&Socket::start_block)));
		class_add_func("end_block", TypeVoid, net_p(mf(&Socket::end_block)));
		class_add_func("set_buffer_pos", TypeVoid, net_p(mf(&Socket::set_buffer_pos)));
			func_add_param("pos", TypeInt);
		class_add_func("get_buffer_pos", TypeInt, net_p(mf(&Socket::get_buffer_pos)));
		class_add_func("__lshift__", TypeVoid, net_p(mf((void(Socket::*)(int))&Socket::operator<<)));
			func_add_param("i", TypeInt);
		class_add_func("__lshift__", TypeVoid, net_p(mf((void(Socket::*)(float))&Socket::operator<<)));
			func_add_param("f", TypeFloat32);
		class_add_func("__lshift__", TypeVoid, net_p(mf((void(Socket::*)(bool))&Socket::operator<<)));
			func_add_param("b", TypeBool);
		class_add_func("__lshift__", TypeVoid, net_p(mf((void(Socket::*)(char))&Socket::operator<<)));
			func_add_param("c", TypeChar);
		class_add_func("__lshift__", TypeVoid, net_p(mf((void(Socket::*)(const string &))&Socket::operator<<)));
			func_add_param("s", TypeString);
		class_add_func("__lshift__", TypeVoid, net_p(mf((void(Socket::*)(const vector &))&Socket::operator<<)));
			func_add_param("v", TypeVector);

		class_add_func("listen", TypeSocketP, net_p(&__socket_listen__), Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("port", TypeInt);
			func_add_param("block", TypeBool);
		class_add_func("connect", TypeSocketP, net_p(&__socket_connect__), Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("addr", TypeString);
			func_add_param("port", TypeInt);
		class_add_func("create_udp", TypeSocketP, net_p(&__socket_create_udp__), Flags::_STATIC__RAISES_EXCEPTIONS);
			func_add_param("port", TypeInt);
}

};
