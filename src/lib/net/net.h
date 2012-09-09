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

#include "../types/types.h"


//--------------------------------------------------------------------//
//                  all about networking                              //
//--------------------------------------------------------------------//


// general
void _cdecl NetInit();
void _cdecl NetClose(int &s);
void _cdecl NetSetBlocking(int s, bool blocking);
int _cdecl NetCreate(int port, bool blocking);
int _cdecl NetAccept(int sh);
int _cdecl NetConnect(const string &addr, int port);
bool _cdecl NetConnectionLost(int s);

// send / recieve directly
string _cdecl NetRead(int s);
int _cdecl NetWrite(int s, const string &buf);
bool _cdecl NetReadyToWrite(int s);
bool _cdecl NetReadyToRead(int s);

// buffered data
void _cdecl NetResetBuffer();
bool _cdecl NetReadBuffer(int s);
bool _cdecl NetWriteBuffer(int s);
// read from buffer
int _cdecl NetReadInt();
bool _cdecl NetReadBool();
float _cdecl NetReadFloat();
vector _cdecl NetReadVector();
string _cdecl NetReadStr();
char _cdecl NetReadChar();
int _cdecl NetReadBlockStart();
void _cdecl NetReadBlockEnd();
// write to buffer
void _cdecl NetWriteInt(int i);
void _cdecl NetWriteBool(bool b);
void _cdecl NetWriteFloat(float f);
void _cdecl NetWriteVector(const vector &v);
void _cdecl NetWriteChar(char c);
void _cdecl NetWriteStr(const string &str);
void _cdecl NetWriteBlockStart(int id);
void _cdecl NetWriteBlockEnd();

int NetGetBufferPos();
void NetSetBufferPos(int pos);

// ...
bool _cdecl NetSendBugReport(const string &sender, const string &program, const string &version, const string &comment, string &return_msg);

extern float NetConnectTimeout;

#endif
