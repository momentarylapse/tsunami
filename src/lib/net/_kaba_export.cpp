#include "../base/base.h"
#include "net.h"
#include "../kabaexport/KabaExporter.h"
#include "../os/msg.h"

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


void export_package_net(kaba::Exporter* e) {
	e->declare_class_size("Address", sizeof(net::NetAddress));
	e->declare_class_element("Address.host", &net::NetAddress::host);
	e->declare_class_element("Address.port", &net::NetAddress::port);
	e->link_class_func("Address.__init__", &kaba::generic_init<net::NetAddress>);
	e->link_class_func("Address.__delete__", &kaba::generic_delete<net::NetAddress>);

	e->declare_class_size("Socket", sizeof(net::Socket));
	e->declare_class_element("Socket.uid", &net::Socket::uid);
	e->link_class_func("Socket.__init__", &kaba::generic_init<net::Socket>);
	e->link_class_func("Socket.__delete__", &kaba::generic_delete<net::Socket>);
	e->link_class_func("Socket.accept", &net::Socket::accept);
	e->link_class_func("Socket.close", &net::Socket::close);
	e->link_class_func("Socket.set_blocking", &net::Socket::set_blocking);
	e->link_class_func("Socket.set_target", &net::Socket::set_target);
	e->link_class_func("Socket.get_sender", &net::Socket::get_sender);
	e->link_class_func("Socket.read", &net::Socket::read);
	e->link_class_func("Socket.write", &net::Socket::write);
	e->link_class_func("Socket.can_read", &net::Socket::can_read);
	e->link_class_func("Socket.can_write", &net::Socket::can_write);
	e->link_class_func("Socket.is_connected", &net::Socket::is_connected);

	e->declare_class_size("BinaryBuffer", sizeof(BinaryBuffer));
	e->declare_class_element("BinaryBuffer.data", &BinaryBuffer::data);
	e->link_class_func("BinaryBuffer.__init__", &kaba::generic_init<BinaryBuffer>);
	e->link_class_func("BinaryBuffer.__delete__", &kaba::generic_delete<BinaryBuffer>);
	e->link_class_func("BinaryBuffer.__rshift__:BinaryBuffer:i32", (void(BinaryBuffer::*)(int&))&BinaryBuffer::operator>>);
	e->link_class_func("BinaryBuffer.__rshift__:BinaryBuffer:f32", (void(BinaryBuffer::*)(float&))&BinaryBuffer::operator>>);
	e->link_class_func("BinaryBuffer.__rshift__:BinaryBuffer:bool", (void(BinaryBuffer::*)(bool&))&BinaryBuffer::operator>>);
	e->link_class_func("BinaryBuffer.__rshift__:BinaryBuffer:u8", (void(BinaryBuffer::*)(char&))&BinaryBuffer::operator>>);
	e->link_class_func("BinaryBuffer.__rshift__:BinaryBuffer:string", (void(BinaryBuffer::*)(string&))&BinaryBuffer::operator>>);
	e->link_class_func("BinaryBuffer.__rshift__:BinaryBuffer:math.vec3", (void(BinaryBuffer::*)(vec3&))&BinaryBuffer::operator>>);
	e->link_class_func("BinaryBuffer.clear", &BinaryBuffer::clear);
	e->link_class_func("BinaryBuffer.start_block", &BinaryBuffer::start_block);
	e->link_class_func("BinaryBuffer.end_block", &BinaryBuffer::end_block);
	e->link_class_func("BinaryBuffer.set_pos", &BinaryBuffer::set_pos);
	e->link_class_func("BinaryBuffer.get_pos", &BinaryBuffer::get_pos);
	e->link_class_func("BinaryBuffer.__lshift__:BinaryBuffer:i32", (void(BinaryBuffer::*)(int))&BinaryBuffer::operator<<);
	e->link_class_func("BinaryBuffer.__lshift__:BinaryBuffer:f32", (void(BinaryBuffer::*)(float))&BinaryBuffer::operator<<);
	e->link_class_func("BinaryBuffer.__lshift__:BinaryBuffer:bool", (void(BinaryBuffer::*)(bool))&BinaryBuffer::operator<<);
	e->link_class_func("BinaryBuffer.__lshift__:BinaryBuffer:u8", (void(BinaryBuffer::*)(char))&BinaryBuffer::operator<<);
	e->link_class_func("BinaryBuffer.__lshift__:BinaryBuffer:string", (void(BinaryBuffer::*)(const string&))&BinaryBuffer::operator<<);
	e->link_class_func("BinaryBuffer.__lshift__:BinaryBuffer:math.vec3", (void(BinaryBuffer::*)(const vec3&))&BinaryBuffer::operator<<);

	e->link_func("listen", &__socket_listen__);
	e->link_func("connect", &__socket_connect__);
	e->link_func("create_udp", &__socket_create_udp__);
}


