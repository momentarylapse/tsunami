/*----------------------------------------------------------------------------*\
| net                                                                          |
| -> network functions                                                         |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2008.12.13 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _NET_INCLUDED_
#define _NET_INCLUDED_


#include "../base/base.h"
#include "Socket.h"
#include "BinaryBuffer.h"


class SocketError : public Exception {
public:
	SocketError() : Exception() {}
	SocketError(const string &msg) : Exception(msg) {}
};

class SocketConnectionLostException : public SocketError {};


void NetInit();

// ...
void NetSendBugReport(const string &sender, const string &program, const string &version, const string &comment);

#endif
