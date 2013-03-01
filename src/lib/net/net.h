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
	Socket operator=(const Socket &other);

	bool Create(int port);
	bool Accept(Socket &con);
	bool Connect(const string &addr, int port);
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
	void __assign__(const Socket &other);
private:
	int uid;
	int s;
	string *buffer;
	int buffer_pos;
	bool last_op_reading;
};

void NetInit();


// buffered data
/*void _cdecl NetResetBuffer();
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
void NetSetBufferPos(int pos);*/

// ...
bool NetSendBugReport(const string &sender, const string &program, const string &version, const string &comment, string &return_msg);

#endif
