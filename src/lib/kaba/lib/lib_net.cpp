#include "../../file/file.h"
#include "../kaba.h"
#include "../../config.h"
#include "common.h"


#ifdef _X_USE_NET_
	#include "../../net/net.h"
#endif

namespace Kaba{

#ifdef _X_USE_NET_
	#define net_p(p)		(void*)p
	static NetAddress *_addr;
	#define GetDAAddress(x)			int_p(&_addr->x)-int_p(_addr)
#else
	typedef int Socket;
	typedef int NetAddress;
	#define net_p(p)		NULL
#endif
#ifdef _X_ALLOW_X_
	#define x_p(p)		(void*)p
#else
	#define x_p(p)		NULL
#endif

extern Class *TypeIntPs;
extern Class *TypeFloatPs;
extern Class *TypeBoolPs;
extern Class *TypeCharPs;

Class *TypeNetAddress;
Class *TypeSocket;
Class *TypeSocketP;
Class *TypeSocketPList;

void SIAddPackageNet()
{
	add_package("net", false);

	TypeNetAddress = add_type  ("NetAddress",	sizeof(NetAddress));
	TypeSocket     = add_type  ("Socket",		sizeof(Socket));
	TypeSocketP    = add_type_p("Socket*",	TypeSocket);
	TypeSocketPList = add_type_a("Socket*[]",	TypeSocketP, -1);


	add_class(TypeNetAddress);
		class_add_element("host", TypeString, GetDAAddress(host));
		class_add_element("port", TypeInt, GetDAAddress(port));
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, net_p(mf(&NetAddress::__init__)));
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, net_p(mf(&NetAddress::__delete__)));

	add_class(TypeSocket);
		class_add_element("uid", TypeInt,0);
		class_add_func(IDENTIFIER_FUNC_INIT,		TypeVoid,	net_p(mf(&Socket::__init__)));
		class_add_func(IDENTIFIER_FUNC_DELETE,		TypeVoid,	net_p(mf(&Socket::__delete__)));
		class_add_func("accept",		TypeSocketP,	net_p(mf(&Socket::accept)));
		class_add_func("close",		TypeVoid,	net_p(mf(&Socket::close)));
		class_add_func("setBlocking",		TypeVoid,	net_p(mf(&Socket::setBlocking)));
			func_add_param("block",		TypeBool);
		class_add_func("setTarget",		TypeVoid,	net_p(mf(&Socket::setTarget)));
			func_add_param("target",		TypeNetAddress);
		class_add_func("getSender",		TypeNetAddress,	net_p(mf(&Socket::getSender)));
		class_add_func("read",		TypeString,	net_p(mf(&Socket::read)));
		class_add_func("write",		TypeBool,	net_p(mf(&Socket::write)));
			func_add_param("buf",		TypeString);
		class_add_func("canRead",		TypeBool,	net_p(mf(&Socket::canRead)));
		class_add_func("canWrite",		TypeBool,	net_p(mf(&Socket::canWrite)));
		class_add_func("isConnected",	TypeBool,	net_p(mf(&Socket::isConnected)));
		class_add_func("__rshift__",		TypeVoid,	net_p(mf((void(Socket::*)(int&))&Socket::operator>>)));
			func_add_param("i",		TypeIntPs);
		class_add_func("__rshift__",		TypeVoid,	net_p(mf((void(Socket::*)(float&))&Socket::operator>>)));
			func_add_param("f",		TypeFloatPs);
		class_add_func("__rshift__",		TypeVoid,	net_p(mf((void(Socket::*)(bool&))&Socket::operator>>)));
			func_add_param("b",		TypeBoolPs);
		class_add_func("__rshift__",		TypeVoid,	net_p(mf((void(Socket::*)(char&))&Socket::operator>>)));
			func_add_param("c",		TypeCharPs);
		class_add_func("__rshift__",		TypeVoid,	net_p(mf((void(Socket::*)(string&))&Socket::operator>>)));
			func_add_param("s",		TypeString);
		class_add_func("__rshift__",		TypeVoid,	net_p(mf((void(Socket::*)(vector&))&Socket::operator>>)));
			func_add_param("v",		TypeVector);
		class_add_func("writeBuffer",		TypeBool,	net_p(mf(&Socket::writeBuffer)));
		class_add_func("__lshift__",		TypeVoid,	net_p(mf((void(Socket::*)(int))&Socket::operator<<)));
			func_add_param("i",		TypeInt);
		class_add_func("__lshift__",		TypeVoid,	net_p(mf((void(Socket::*)(float))&Socket::operator<<)));
			func_add_param("f",		TypeFloat32);
		class_add_func("__lshift__",		TypeVoid,	net_p(mf((void(Socket::*)(bool))&Socket::operator<<)));
			func_add_param("b",		TypeBool);
		class_add_func("__lshift__",		TypeVoid,	net_p(mf((void(Socket::*)(char))&Socket::operator<<)));
			func_add_param("c",		TypeChar);
		class_add_func("__lshift__",		TypeVoid,	net_p(mf((void(Socket::*)(const string &))&Socket::operator<<)));
			func_add_param("s",		TypeString);
		class_add_func("__lshift__",		TypeVoid,	net_p(mf((void(Socket::*)(const vector &))&Socket::operator<<)));
			func_add_param("v",		TypeVector);

	add_func("SocketListen",		TypeSocketP,	net_p(&NetListen));
		func_add_param("port",		TypeInt);
		func_add_param("block",		TypeBool);
	add_func("SocketConnect",		TypeSocketP,	net_p(&NetConnect));
		func_add_param("addr",		TypeString);
		func_add_param("port",		TypeInt);
	add_func("CreateUDP",		TypeSocketP,	net_p(&NetCreateUDP));
		func_add_param("port",		TypeInt);
}

};
