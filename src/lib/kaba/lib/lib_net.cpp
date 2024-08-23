#include "../kaba.h"
#include "lib.h"
#include "shared.h"
#include "../dynamic/exception.h"


#if __has_include("../../net/net.h")
	#include "../../net/net.h"
	#define KABA_EXPORT_NET
#endif

namespace kaba {

#ifdef KABA_EXPORT_NET
	#define net_p(p)		p
	[[maybe_unused]] static NetAddress *_addr;
	#define GetDAAddress(x)			int_p(&_addr->x)-int_p(_addr)
#else
	typedef int Socket;
	typedef int BinaryBuffer;
	typedef int NetAddress;
	#define net_p(p)		NULL
	#define GetDAAddress(x)			0
#endif


#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")


xfer<Socket> __socket_listen__(int port, bool block) {
	KABA_EXCEPTION_WRAPPER( return Socket::listen(port, block); );
	return nullptr;
}

xfer<Socket> __socket_connect__(const string &host, int port) {
	KABA_EXCEPTION_WRAPPER( return Socket::connect(host, port); );
	return nullptr;
}

xfer<Socket> __socket_create_udp__(int port) {
	KABA_EXCEPTION_WRAPPER( return Socket::create_udp(port); );
	return nullptr;
}

#pragma GCC pop_options

void SIAddPackageNet(Context *c) {
	add_package(c, "net");

	auto TypeNetAddress  = add_type("NetAddress", sizeof(NetAddress));
	auto TypeSocket      = add_type("Socket", sizeof(Socket));
	auto TypeSocketXfer  = add_type_p_xfer(TypeSocket);
	auto TypeBinaryBuffer = add_type("BinaryBuffer", sizeof(BinaryBuffer));

	lib_create_pointer_xfer(TypeSocketXfer);


	add_class(TypeNetAddress);
		class_add_element("host", TypeString, net_p(&NetAddress::host));
		class_add_element("port", TypeInt32, net_p(&NetAddress::port));
		class_add_func(Identifier::Func::INIT, TypeVoid, net_p(&NetAddress::__init__), Flags::MUTABLE);
		class_add_func(Identifier::Func::DELETE, TypeVoid, net_p(&NetAddress::__delete__), Flags::MUTABLE);

	add_class(TypeSocket);
		class_add_element("uid", TypeInt32, net_p(&Socket::uid));
		class_add_func(Identifier::Func::INIT, TypeVoid, net_p(&Socket::__init__), Flags::MUTABLE);
		class_add_func(Identifier::Func::DELETE, TypeVoid, net_p(&Socket::__delete__), Flags::MUTABLE);
		class_add_func("accept", TypeSocketXfer, net_p(&Socket::accept), Flags::MUTABLE);
		class_add_func("close", TypeVoid, net_p(&Socket::close), Flags::MUTABLE);
		class_add_func("set_blocking", TypeVoid, net_p(&Socket::set_blocking), Flags::MUTABLE);
			func_add_param("block", TypeBool);
		class_add_func("set_target", TypeVoid, net_p(&Socket::set_target), Flags::MUTABLE);
			func_add_param("target", TypeNetAddress);
		class_add_func("get_sender", TypeNetAddress, net_p(&Socket::get_sender), Flags::MUTABLE);
		class_add_func("read", TypeBytes, net_p(&Socket::read), Flags::MUTABLE);
			func_add_param("size", TypeInt32);
		class_add_func("write", TypeBool, net_p(&Socket::write), Flags::MUTABLE);
			func_add_param("buf", TypeBytes);
		class_add_func("can_read", TypeBool, net_p(&Socket::can_read));
		class_add_func("can_write", TypeBool, net_p(&Socket::can_write));
		class_add_func("is_connected", TypeBool, net_p(&Socket::is_connected));

		class_add_func("listen", TypeSocketXfer, net_p(&__socket_listen__), Flags::STATIC | Flags::RAISES_EXCEPTIONS);
			func_add_param("port", TypeInt32);
			func_add_param("block", TypeBool);
		class_add_func("connect", TypeSocketXfer, net_p(&__socket_connect__), Flags::STATIC | Flags::RAISES_EXCEPTIONS);
			func_add_param("addr", TypeString);
			func_add_param("port", TypeInt32);
		class_add_func("create_udp", TypeSocketXfer, net_p(&__socket_create_udp__), Flags::STATIC | Flags::RAISES_EXCEPTIONS);
			func_add_param("port", TypeInt32);

	add_class(TypeBinaryBuffer);
		class_add_element("data", TypeBytes, net_p(&BinaryBuffer::data));
		class_add_func(Identifier::Func::INIT, TypeVoid, net_p(&BinaryBuffer::__init__), Flags::MUTABLE);
		class_add_func(Identifier::Func::DELETE, TypeVoid, net_p(&BinaryBuffer::__delete__), Flags::MUTABLE);
		class_add_func("__rshift__", TypeVoid, net_p((void(BinaryBuffer::*)(int&))&BinaryBuffer::operator>>), Flags::MUTABLE);
			func_add_param("i", TypeInt32, Flags::OUT);
		class_add_func("__rshift__", TypeVoid, net_p((void(BinaryBuffer::*)(float&))&BinaryBuffer::operator>>), Flags::MUTABLE);
			func_add_param("f", TypeFloat32, Flags::OUT);
		class_add_func("__rshift__", TypeVoid, net_p((void(BinaryBuffer::*)(bool&))&BinaryBuffer::operator>>), Flags::MUTABLE);
			func_add_param("b", TypeBool, Flags::OUT);
		class_add_func("__rshift__", TypeVoid, net_p((void(BinaryBuffer::*)(char&))&BinaryBuffer::operator>>), Flags::MUTABLE);
			func_add_param("c", TypeUInt8, Flags::OUT);
		class_add_func("__rshift__", TypeVoid, net_p((void(BinaryBuffer::*)(string&))&BinaryBuffer::operator>>), Flags::MUTABLE);
			func_add_param("s", TypeString, Flags::OUT);
		class_add_func("__rshift__", TypeVoid, net_p((void(BinaryBuffer::*)(vec3&))&BinaryBuffer::operator>>), Flags::MUTABLE);
			func_add_param("v", TypeVec3, Flags::OUT);
		class_add_func("clear", TypeVoid, net_p(&BinaryBuffer::clear), Flags::MUTABLE);
		class_add_func("start_block", TypeVoid, net_p(&BinaryBuffer::start_block), Flags::MUTABLE);
		class_add_func("end_block", TypeVoid, net_p(&BinaryBuffer::end_block), Flags::MUTABLE);
		class_add_func("set_pos", TypeVoid, net_p(&BinaryBuffer::set_pos), Flags::MUTABLE);
			func_add_param("pos", TypeInt32);
		class_add_func("get_pos", TypeInt32, net_p(&BinaryBuffer::get_pos));
		class_add_func("__lshift__", TypeVoid, net_p((void(BinaryBuffer::*)(int))&BinaryBuffer::operator<<), Flags::MUTABLE);
			func_add_param("i", TypeInt32);
		class_add_func("__lshift__", TypeVoid, net_p((void(BinaryBuffer::*)(float))&BinaryBuffer::operator<<), Flags::MUTABLE);
			func_add_param("f", TypeFloat32);
		class_add_func("__lshift__", TypeVoid, net_p((void(BinaryBuffer::*)(bool))&BinaryBuffer::operator<<), Flags::MUTABLE);
			func_add_param("b", TypeBool);
		class_add_func("__lshift__", TypeVoid, net_p((void(BinaryBuffer::*)(char))&BinaryBuffer::operator<<), Flags::MUTABLE);
			func_add_param("c", TypeUInt8);
		class_add_func("__lshift__", TypeVoid, net_p((void(BinaryBuffer::*)(const string &))&BinaryBuffer::operator<<), Flags::MUTABLE);
			func_add_param("s", TypeString);
		class_add_func("__lshift__", TypeVoid, net_p((void(BinaryBuffer::*)(const vec3 &))&BinaryBuffer::operator<<), Flags::MUTABLE);
			func_add_param("v", TypeVec3);
}

};
