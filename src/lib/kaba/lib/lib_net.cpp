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
	[[maybe_unused]] static net::NetAddress *_addr;
	#define GetDAAddress(x)			int_p(&_addr->x)-int_p(_addr)
#else
	typedef int Socket;
	typedef int BinaryBuffer;
	typedef int NetAddress;
	#define net_p(p)		NULL
	#define GetDAAddress(x)			0
#endif

KABA_LINK_GROUP_BEGIN

xfer<net::Socket> __socket_listen__(int port, bool block) {
	KABA_EXCEPTION_WRAPPER( return net::listen(port, block); );
	return nullptr;
}

xfer<net::Socket> __socket_connect__(const string &host, int port) {
	KABA_EXCEPTION_WRAPPER( return net::connect(host, port); );
	return nullptr;
}

xfer<net::Socket> __socket_create_udp__(int port) {
	KABA_EXCEPTION_WRAPPER( return net::create_udp(port); );
	return nullptr;
}

KABA_LINK_GROUP_END

void SIAddPackageNet(Context *c) {
	add_package(c, "net");

	auto TypeAddress  = add_type("Address", sizeof(net::NetAddress));
	auto TypeSocket      = add_type("Socket", sizeof(net::Socket));
	auto TypeSocketXfer  = add_type_p_xfer(TypeSocket);
	auto TypeBinaryBuffer = add_type("BinaryBuffer", sizeof(BinaryBuffer));

	lib_create_pointer_xfer(TypeSocketXfer);


	add_class(TypeAddress);
		class_add_element("host", TypeString, net_p(&net::NetAddress::host));
		class_add_element("port", TypeInt32, net_p(&net::NetAddress::port));
		class_add_func(Identifier::func::Init, TypeVoid, net_p(&generic_init<net::NetAddress>), Flags::Mutable);
		class_add_func(Identifier::func::Delete, TypeVoid, net_p(&generic_delete<net::NetAddress>), Flags::Mutable);

	add_class(TypeSocket);
		class_add_element("uid", TypeInt32, net_p(&net::Socket::uid));
		class_add_func(Identifier::func::Init, TypeVoid, net_p(&generic_init<net::Socket>), Flags::Mutable);
		class_add_func(Identifier::func::Delete, TypeVoid, net_p(&generic_delete<net::Socket>), Flags::Mutable);
		class_add_func("accept", TypeSocketXfer, net_p(&net::Socket::accept), Flags::Mutable);
		class_add_func("close", TypeVoid, net_p(&net::Socket::close), Flags::Mutable);
		class_add_func("set_blocking", TypeVoid, net_p(&net::Socket::set_blocking), Flags::Mutable);
			func_add_param("block", TypeBool);
		class_add_func("set_target", TypeVoid, net_p(&net::Socket::set_target), Flags::Mutable);
			func_add_param("target", TypeAddress);
		class_add_func("get_sender", TypeAddress, net_p(&net::Socket::get_sender), Flags::Mutable);
		class_add_func("read", TypeBytes, net_p(&net::Socket::read), Flags::Mutable);
			func_add_param("size", TypeInt32);
		class_add_func("write", TypeBool, net_p(&net::Socket::write), Flags::Mutable);
			func_add_param("buf", TypeBytes);
		class_add_func("can_read", TypeBool, net_p(&net::Socket::can_read));
		class_add_func("can_write", TypeBool, net_p(&net::Socket::can_write));
		class_add_func("is_connected", TypeBool, net_p(&net::Socket::is_connected));

	add_func("listen", TypeSocketXfer, net_p(&__socket_listen__), Flags::Static | Flags::RaisesExceptions);
		func_add_param("port", TypeInt32);
		func_add_param("block", TypeBool);
	add_func("connect", TypeSocketXfer, net_p(&__socket_connect__), Flags::Static | Flags::RaisesExceptions);
		func_add_param("addr", TypeString);
		func_add_param("port", TypeInt32);
	add_func("create_udp", TypeSocketXfer, net_p(&__socket_create_udp__), Flags::Static | Flags::RaisesExceptions);
		func_add_param("port", TypeInt32);

	add_class(TypeBinaryBuffer);
		class_add_element("data", TypeBytes, net_p(&BinaryBuffer::data));
		class_add_func(Identifier::func::Init, TypeVoid, net_p(&BinaryBuffer::__init__), Flags::Mutable);
		class_add_func(Identifier::func::Delete, TypeVoid, net_p(&BinaryBuffer::__delete__), Flags::Mutable);
		class_add_func("__rshift__", TypeVoid, net_p((void(BinaryBuffer::*)(int&))&BinaryBuffer::operator>>), Flags::Mutable);
			func_add_param("i", TypeInt32, Flags::Out);
		class_add_func("__rshift__", TypeVoid, net_p((void(BinaryBuffer::*)(float&))&BinaryBuffer::operator>>), Flags::Mutable);
			func_add_param("f", TypeFloat32, Flags::Out);
		class_add_func("__rshift__", TypeVoid, net_p((void(BinaryBuffer::*)(bool&))&BinaryBuffer::operator>>), Flags::Mutable);
			func_add_param("b", TypeBool, Flags::Out);
		class_add_func("__rshift__", TypeVoid, net_p((void(BinaryBuffer::*)(char&))&BinaryBuffer::operator>>), Flags::Mutable);
			func_add_param("c", TypeUInt8, Flags::Out);
		class_add_func("__rshift__", TypeVoid, net_p((void(BinaryBuffer::*)(string&))&BinaryBuffer::operator>>), Flags::Mutable);
			func_add_param("s", TypeString, Flags::Out);
		class_add_func("__rshift__", TypeVoid, net_p((void(BinaryBuffer::*)(vec3&))&BinaryBuffer::operator>>), Flags::Mutable);
			func_add_param("v", TypeVec3, Flags::Out);
		class_add_func("clear", TypeVoid, net_p(&BinaryBuffer::clear), Flags::Mutable);
		class_add_func("start_block", TypeVoid, net_p(&BinaryBuffer::start_block), Flags::Mutable);
		class_add_func("end_block", TypeVoid, net_p(&BinaryBuffer::end_block), Flags::Mutable);
		class_add_func("set_pos", TypeVoid, net_p(&BinaryBuffer::set_pos), Flags::Mutable);
			func_add_param("pos", TypeInt32);
		class_add_func("get_pos", TypeInt32, net_p(&BinaryBuffer::get_pos));
		class_add_func("__lshift__", TypeVoid, net_p((void(BinaryBuffer::*)(int))&BinaryBuffer::operator<<), Flags::Mutable);
			func_add_param("i", TypeInt32);
		class_add_func("__lshift__", TypeVoid, net_p((void(BinaryBuffer::*)(float))&BinaryBuffer::operator<<), Flags::Mutable);
			func_add_param("f", TypeFloat32);
		class_add_func("__lshift__", TypeVoid, net_p((void(BinaryBuffer::*)(bool))&BinaryBuffer::operator<<), Flags::Mutable);
			func_add_param("b", TypeBool);
		class_add_func("__lshift__", TypeVoid, net_p((void(BinaryBuffer::*)(char))&BinaryBuffer::operator<<), Flags::Mutable);
			func_add_param("c", TypeUInt8);
		class_add_func("__lshift__", TypeVoid, net_p((void(BinaryBuffer::*)(const string &))&BinaryBuffer::operator<<), Flags::Mutable);
			func_add_param("s", TypeString);
		class_add_func("__lshift__", TypeVoid, net_p((void(BinaryBuffer::*)(const vec3 &))&BinaryBuffer::operator<<), Flags::Mutable);
			func_add_param("v", TypeVec3);
}

};
