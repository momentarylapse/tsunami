#include "../../file/file.h"
#include "../script.h"
#include "../../config.h"
#include "script_data_common.h"


#ifdef _X_USE_NET_
	#include "../../net/net.h"
#endif

#ifdef _X_ALLOW_X_
	#include "../../../networking.h"
#endif

namespace Script{

#ifdef _X_USE_NET_
	#define net_p(p)		(void*)p
#else
	typedef int Socket;
	#define net_p(p)		NULL
#endif
#ifdef _X_ALLOW_X_
	#define x_p(p)		(void*)p
#else
	#define x_p(p)		NULL
#endif

extern Type *TypeIntPs;
extern Type *TypeFloatPs;
extern Type *TypeBoolPs;
extern Type *TypeCharPs;

Type *TypeSocket;
Type *TypeSocketP;
Type *TypeSocketList;

void SIAddPackageNet()
{
	msg_db_f("SIAddPackageNet", 3);

	add_package("net", false);

	TypeSocket     = add_type  ("Socket",		sizeof(Socket));
	TypeSocketP    = add_type_p("Socket*",	TypeSocket);
	TypeSocketList = add_type_a("Socket[]",	TypeSocket, -1);

	add_class(TypeSocket);
		class_add_element("uid", TypeInt,0);
		class_add_func("__init__",		TypeVoid,	net_p(mf(&Socket::__init__)));
		class_add_func("__delete__",		TypeVoid,	net_p(mf(&Socket::__delete__)));
		class_add_func("__assign__",		TypeVoid,	net_p(mf(&Socket::__assign__)));
			func_add_param("other",		TypeSocket);
		class_add_func("Create",		TypeBool,	net_p(mf(&Socket::Create)));
			func_add_param("port",		TypeInt);
		class_add_func("Connect",		TypeBool,	net_p(mf(&Socket::Connect)));
			func_add_param("addr",		TypeString);
			func_add_param("port",		TypeInt);
		class_add_func("Accept",		TypeBool,	net_p(mf(&Socket::Accept)));
			func_add_param("con",		TypeSocket);
		class_add_func("Close",		TypeVoid,	net_p(mf(&Socket::Close)));
		class_add_func("SetBlocking",		TypeVoid,	net_p(mf(&Socket::SetBlocking)));
			func_add_param("block",		TypeBool);
		class_add_func("Read",		TypeString,	net_p(mf(&Socket::Read)));
		class_add_func("Write",		TypeBool,	net_p(mf(&Socket::Write)));
			func_add_param("buf",		TypeString);
		class_add_func("CanRead",		TypeBool,	net_p(mf(&Socket::CanRead)));
		class_add_func("CanWrite",		TypeBool,	net_p(mf(&Socket::CanWrite)));
		class_add_func("IsConnected",	TypeBool,	net_p(mf(&Socket::IsConnected)));
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
		class_add_func("WriteBuffer",		TypeBool,	net_p(mf(&Socket::WriteBuffer)));
		class_add_func("__lshift__",		TypeVoid,	net_p(mf((void(Socket::*)(int))&Socket::operator<<)));
			func_add_param("i",		TypeInt);
		class_add_func("__lshift__",		TypeVoid,	net_p(mf((void(Socket::*)(float))&Socket::operator<<)));
			func_add_param("f",		TypeFloat);
		class_add_func("__lshift__",		TypeVoid,	net_p(mf((void(Socket::*)(bool))&Socket::operator<<)));
			func_add_param("b",		TypeBool);
		class_add_func("__lshift__",		TypeVoid,	net_p(mf((void(Socket::*)(char))&Socket::operator<<)));
			func_add_param("c",		TypeChar);
		class_add_func("__lshift__",		TypeVoid,	net_p(mf((void(Socket::*)(const string &))&Socket::operator<<)));
			func_add_param("s",		TypeString);
		class_add_func("__lshift__",		TypeVoid,	net_p(mf((void(Socket::*)(const vector &))&Socket::operator<<)));
			func_add_param("v",		TypeVector);

	
	add_func("XNetAddMsgHandler",		TypeVoid,			x_p(&XNetAddMsgHandler));
		func_add_param("name",		TypeString);
		func_add_param("func",		TypePointer);
	add_func("XNetSendMsgStart",		TypeVoid,			x_p(&XNetSendMsgStart));
		func_add_param("name",		TypeString);
		func_add_param("target",		TypeInt);
	add_func("XNetSendMsgEnd",		TypeVoid,			x_p(&XNetSendMsgEnd));
	add_func("XNetWriteInt",		TypeVoid,			x_p(&XNetWriteInt));
		func_add_param("i",		TypeInt);
	add_func("XNetWriteFloat",		TypeVoid,			x_p(&XNetWriteFloat));
		func_add_param("f",		TypeFloat);
	add_func("XNetWriteBool",		TypeVoid,			x_p(&XNetWriteBool));
		func_add_param("b",		TypeBool);
	add_func("XNetWriteVector",		TypeVoid,			x_p(&XNetWriteVector));
		func_add_param("v",		TypeVector);
	add_func("XNetWriteString",		TypeVoid,			x_p(&XNetWriteString));
		func_add_param("str",		TypeString);
}

};
