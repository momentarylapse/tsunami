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


class vector;
class string;


class Socket
{
public:
	Socket();
	~Socket();

	bool _Create(int port, bool block);
	bool _Connect(const string &addr,int port);

	Socket *Accept();
	void Close();
	void SetBlocking(bool blocking);
	bool IsConnected();

	// send / receive directly
	string Read();
	bool Write(const string &buf);
	bool CanWrite();
	bool CanRead();

	// buffered read
	int ReadInt();
	float ReadFloat();
	bool ReadBool();
	char ReadChar();
	string ReadString();
	vector ReadVector();
	void operator>>(int &i);
	void operator>>(float &f);
	void operator>>(bool &b);
	void operator>>(char &c);
	void operator>>(string &s);
	void operator>>(vector &v);
	void _read_buffered_(void *p, int size);
	void SetBufferPos(int pos);
	int GetBufferPos();

	// buffered write
	void WriteInt(int i);
	void WriteFloat(float f);
	void WriteBool(bool b);
	void WriteChar(char c);
	void WriteString(const string &s);
	void WriteVector(const vector &v);
	void operator<<(int i){	WriteInt(i);	}
	void operator<<(float f){	WriteFloat(f);	}
	void operator<<(bool b){	WriteBool(b);	}
	void operator<<(char c){	WriteChar(c);	}
	void operator<<(const string &s){		WriteString(s);	}
	void operator<<(const vector &v){		WriteVector(v);	}
	bool WriteBuffer();

	// kaba interface
	void __init__();
	void __delete__();
private:
	int uid;
	int s;
	string *buffer;
	int buffer_pos;
	bool last_op_reading;
};

void NetInit();

Socket *NetListen(int port, bool block);
Socket *NetConnect(const string &host, int port);

// ...
bool NetSendBugReport(const string &sender, const string &program, const string &version, const string &comment, string &return_msg);

#endif
