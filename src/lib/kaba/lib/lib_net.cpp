#include "../../file/file.h"
#include "../kaba.h"
#include "../../config.h"
#include "common.h"


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

void SIAddPackageNet()
{
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
		class_add_element("uid", TypeInt,0);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, net_p(mf(&Socket::__init__)));
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, net_p(mf(&Socket::__delete__)));
		class_add_func("accept", TypeSocketP, net_p(mf(&Socket::accept)));
		class_add_func("close", TypeVoid, net_p(mf(&Socket::close)));
		class_add_func("set_blocking", TypeVoid, net_p(mf(&Socket::setBlocking)));
			func_add_param("block", TypeBool);
		class_add_func("set_target", TypeVoid, net_p(mf(&Socket::setTarget)));
			func_add_param("target", TypeNetAddress);
		class_add_func("get_sender", TypeNetAddress, net_p(mf(&Socket::getSender)));
		class_add_func("read", TypeString, net_p(mf(&Socket::read)));
		class_add_func("write", TypeBool, net_p(mf(&Socket::write)));
			func_add_param("buf", TypeString);
		class_add_func("can_read", TypeBool, net_p(mf(&Socket::canRead)));
		class_add_func("can_write", TypeBool, net_p(mf(&Socket::canWrite)));
		class_add_func("is_connected", TypeBool, net_p(mf(&Socket::isConnected)));
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
		class_add_func("write_buffer", TypeBool, net_p(mf(&Socket::writeBuffer)));
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

		class_add_func("listen", TypeSocketP, net_p(&NetListen), Flags::STATIC);
			func_add_param("port", TypeInt);
			func_add_param("block", TypeBool);
		class_add_func("connect", TypeSocketP, net_p(&NetConnect), Flags::STATIC);
			func_add_param("addr", TypeString);
			func_add_param("port", TypeInt);
		class_add_func("create_udp", TypeSocketP, net_p(&NetCreateUDP), Flags::STATIC);
			func_add_param("port", TypeInt);
}

};
