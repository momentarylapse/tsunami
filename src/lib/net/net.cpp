/*----------------------------------------------------------------------------*\
|  net                                                                      |
| -> network functions                                                         |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2008.12.13 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#include "net.h"
#include "../file/file.h"


#ifdef OS_WINDOWS
	#include <winsock.h>
	#pragma comment(lib,"wsock32.lib")

	static WSADATA wsaData;
#endif
#ifdef OS_LINUX
	#include <stdio.h>
	//#include <stdio.h>
	#include <string.h>
	#include <sys/fcntl.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <unistd.h>

	#include <netdb.h>

#endif

/*------------------------------
 protocol:
 char[4] "mxth" / "mxtc"
 int     package length (overall)
 int     continous package index
------------------------------*/


#define NET_MAX_BUFFER		(1048576*4)
#define NET_MAX_SEND			4096
#define NET_MAX_CONNECTIONS	64
#define NET_DEBUG			0

static string Buffer;
static int BufferUsed;
float NetConnectTimeout=5.0f;
static int BlockStartPos=-1,BlockSize;

struct NetConnection
{
	bool used;
	int s;
	bool host;
	int rest_size;
	char rest_buffer[NET_MAX_SEND];
	int recieve_no,send_no;
	bool connection_lost;
};

static NetConnection con[NET_MAX_CONNECTIONS];




void so(int dbg,const string &str)
{
	if (dbg<=NET_DEBUG)
		msg_write(str);
}

void so(int dbg,int i)
{
	if (dbg<=NET_DEBUG)
		msg_write(i);
}


int create_con(bool host, int s)
{
	for (int i=0;i<NET_MAX_CONNECTIONS;i++)
		if (!con[i].used){
			con[i].used=true;
			con[i].s=s;
			con[i].host=host;
			con[i].rest_size=0;
			con[i].recieve_no=0;
			con[i].send_no=0;
			con[i].connection_lost=false;
			return i;
		}
	msg_error("Net: too many connections!");
	return -1;
}




void NetInit()
{
	if (!msg_inited)
		msg_init();
#ifdef OS_WINDOWS
	if (WSAStartup(MAKEWORD(1,1),&wsaData)!=0){
		msg_error("WSAStartup  (Network....)");
	}
#endif
	for (int i=0;i<NET_MAX_CONNECTIONS;i++)
		con[i].used=false;;
}




void NetClose(int &s)
{
	if (s < 0)
		return;
	//so("close");
#ifdef OS_WINDOWS
	if (con[s].s >= 0)
		closesocket(con[s].s);
#endif
#ifdef OS_LINUX
	if (con[s].s >= 0)
		close(con[s].s);
#endif
	con[s].used = false;
	s = -1;
}

void NetSetBlocking(int s, bool blocking)
{
#ifdef OS_WINDOWS
	unsigned long l = blocking ? 0 : 1;
	ioctlsocket(con[s].s, FIONBIO, &l);
#endif
#ifdef OS_LINUX
	fcntl(con[s].s, F_SETFL, blocking ? 0 : O_NONBLOCK);
#endif
}

int _NetCreateSocket_()
{
	so(1,"socket...");
	int s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0){
		so(0,"  -ERROR (CreateSocket)");
		return -1;
	}else
		so(1,"  -ok");
	return s;
}

// host
int NetCreate(int port, bool blocking)
{
	so(1,"socket...");
	int s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0){
		so(0,"  -ERROR (socket creation)");
		return -1;
	}else
		so(1,"  -ok");

	int c = create_con(true, s);

	NetSetBlocking(c, blocking);

	struct sockaddr_in my_addr;
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = INADDR_ANY; /// An jedem Device warten

	so(1,"bind...");
	if (bind(s, (struct sockaddr *)&my_addr, sizeof(my_addr))==-1){
		so(0,"  -ERROR (bind)");
		NetClose(c);
		return -1;
	}else
		so(1,"  -ok");

	so(1,"listen...");
	if (listen(s, 1) == -1){
		so(0,"  -ERROR (listen)");
		return -1;
	}else
		so(1,"  -ok");

	return c;
}

// host
int NetAccept(int sh)
{
//	so(1,"accept...");
	struct sockaddr_in remote_addr;
	int size = sizeof(remote_addr);
	int sc;
#ifdef OS_WINDOWS
	sc = accept(con[sh].s, (struct sockaddr *)&remote_addr, &size);
#endif
#ifdef OS_LINUX
	socklen_t len = *(socklen_t*)&size;
	sc = accept(con[sh].s, (struct sockaddr *)&remote_addr, &len);
#endif

	int c = create_con(true, sc);

	if (sc < 0){
		//so("  -FEHLER");
		NetClose(c);
		return -1;
	}else{
		so(1,"accept...");
		so(1,"  -ok");
	}

	so(1,"  -client found");
	#ifdef OS_WINDOWS
		so(1, inet_ntoa(remote_addr.sin_addr));//.s_addr));
	#endif
	NetSetBlocking(c, true);

	return c;
}

// client
int NetConnect(const string &addr,int port)
{
	int s=-1;
	struct sockaddr_in host_addr;
	struct hostent *host;


	so(1,"GetHostByName...");
		so(1,addr);
	host = gethostbyname(addr.c_str());
	if (host == NULL){
		so(1,"  -ERROR (GetHostByName)");
		return -1;
	}else
		so(1,"  -ok");

	s = _NetCreateSocket_();
	so(2,format("s: %d\n",s));

	host_addr.sin_family = AF_INET;
	host_addr.sin_addr = *((struct in_addr *)host->h_addr);
	host_addr.sin_port = htons(port);

	int c = create_con(false, s);
	so(2,format("c: %d\n",c));


	NetSetBlocking(c, false);

	so(1,"connect...");
	int status=connect(s, (struct sockaddr *)&host_addr, sizeof(host_addr));
	so(2,status);
	//so(-1,errno);
	//so(-1,EINPROGRESS);
	float ttt = 0;
	while(ttt < NetConnectTimeout){
		//printf("%f\n",ttt);
		fd_set r,w;
		FD_ZERO(&r);
		FD_SET(((unsigned)s),&r);
		FD_ZERO(&w);
		FD_SET(((unsigned)s),&w);
		struct timeval t;
		t.tv_sec=0;
		t.tv_usec=10000;
		status=select(s+1,&r,&w,NULL,&t);
		so(2,format("select: %d  r=%d  w=%d",status,FD_ISSET(s,&r),FD_ISSET(s,&w)));
		if ((FD_ISSET(s,&w)>0)&&(status>0)){
			so(2,"test");
			so(2,status);
			struct sockaddr address;
			#ifdef OS_WINDOWS
				int address_len=sizeof(address);
			#else
				socklen_t address_len=sizeof(address);
			#endif
			if (getpeername(s,&address,&address_len)<0){
				so(1,"peer name :(");
				ttt=NetConnectTimeout;
				break;
			}else{
				//so(2,string2("peer: %d    %s",address_len,address.sa_data));
				//so(-1,"ok");
				//so(-1,s);
				ttt=-1;
				so(1,"  -ok");
				break;
			}
		}
		ttt+=0.01f;
	}
	if (ttt>0){
		so(1,"  -ERROR (connect)");
		#ifdef OS_WINDOWS
			so(0,WSAGetLastError());
		#endif
		NetClose(c);
		return -1;
	}

	/*if (connect(s, (struct sockaddr *)&host_addr, sizeof(host_addr))==-1){
		so(0,"  -ERROR (connect)");
		#ifdef OS_WINDOWS
			so(0,WSAGetLastError());
		#endif
		NetClose(s);
		return -1;
	}else
		so(1,"  -ok");*/
	NetSetBlocking(c,true);

	return c;
}

bool NetConnectionLost(int s)
{
	if (s>=0)
		return con[s].connection_lost;
	return true;
}

char _net_temp_buf_[65536];

string NetRead(int s)
{
	int r = recv(con[s].s, _net_temp_buf_, sizeof(_net_temp_buf_), 0);
	string buf;
	if (r > 0){
		buf.resize(r);
		memcpy(buf.data, _net_temp_buf_, r);
	}
	//msg_write(string("recv: ",i2s(r)));
	return buf;
}

int NetWrite(int s, const string &buf)
{
	int r = send(con[s].s, (char*)buf.data, buf.num, 0);
	return r;
}

void NetResetBuffer()
{
	// 12bytes for the buffer length information and other protocol data!
	Buffer.resize(12);
	BufferUsed=12;
	BlockStartPos=-1;
}

bool NetReadBuffer(int s)
{
	/*ZeroMemory(Buffer,NET_MAX_BUFFER);
	so(1,"empfange BUFFER");
	bool ok=true;
	int r=recv(s,Buffer,NET_MAX_BUFFER,0);
	if (r<=0){
		msg_error("beim Empfangen von Daten");
		msg_write(r);
		ok=false;
	}
	if ((NetReadyToRead(s))&&(ok)){
		so(1,"##############################################################################");
		so(1,"restliche Daten:");
		so(1,"##############################################################################");
		char B[5000];
		so(1,recv(s,B,sizeof(B),0));
	}
#if NET_DEBUG>2
	msg_write(Buffer,200);
#endif
	BufferUsed=0;
	return ok;*/

	so(2,"\n_________________________");
	so(1,"RECIEVING!...");
	so(2,s);
	so(2,con[s].s);
	int recieved=0;
	Buffer.resize(12);
	BufferUsed=12;
	BlockStartPos=-1;

	// already recieved last time....?
	if (con[s].rest_size>0){
		so(2,"WARNING (recv)   using data already read!");
		so(2,format("rest=%d",con[s].rest_size));
		Buffer.resize(con[s].rest_size);
		memcpy(Buffer.data, con[s].rest_buffer, con[s].rest_size);
		BufferUsed = *(int*)&Buffer[4];
		recieved = con[s].rest_size;
		so(2,format("BufferUsed=%d", BufferUsed));
		so(2,Buffer);

		con[s].rest_size -= BufferUsed;
		if (con[s].rest_size>0){
			so(2,"WARNING (recv)   still having data!");
			so(2,format("rest=%d",con[s].rest_size));
			memcpy(con[s].rest_buffer,&Buffer[BufferUsed],con[s].rest_size);
	//		so(2,con[s].rest_buffer,con[s].rest_size);
			so(2,d2h(con[s].rest_buffer,12,false));
			so(2,"waere das ein echtes Packet, waere die Laengenangabe:");
			so(2,*(int*)&con[s].rest_buffer[4]);
		}
	}

	// recieve now
	//con[s].rest_size=0;
	while(recieved<BufferUsed){
		so(1,"--recv");
		int current=recv(con[s].s,&Buffer[recieved],NET_MAX_SEND,0);
		so(2,format("gelesen=%d",current));
		if (current<=0){
			so(1,"ERROR (recv)");
			con[s].connection_lost=true; // test me!!!
			return false;
		}
		so(2,Buffer.substr(recieved, recieved + current));
		recieved+=current;
		BufferUsed=*(int*)&Buffer[4];
		so(1,format("%d/%d",recieved,BufferUsed));
		if (recieved>BufferUsed){
			so(2,"WARNING (recv)   read too much!");
			con[s].rest_size=recieved-BufferUsed;
			so(2,format("rest=%d",con[s].rest_size));
			memcpy(con[s].rest_buffer,&Buffer[BufferUsed],con[s].rest_size);
			so(2,string(con[s].rest_buffer,con[s].rest_size));
			so(2,d2h(con[s].rest_buffer,12,false));
			so(2,"waere das ein echtes Packet, waere die Laengenangabe:");
			so(2,*(int*)&con[s].rest_buffer[4]);
		}
		if (BufferUsed>NET_MAX_BUFFER){
			msg_error("ERROR (recv) expected buffer size too large!");
			return false;
		}
	}
	so(2,"<<<");
	so(2,Buffer);
	so(2,d2h(Buffer.data,12,false));
	char bbb=Buffer[3];
	if ( (con[s].host) && (bbb!='c') ){
		msg_error("ERROR (recv): signature missmatch, 'c' expected!");
		return false;
	}
	if ( (!con[s].host) && (bbb!='h') ){
		msg_error("ERROR (recv): signature missmatch, 'h' expected");
		return false;
	}
	BufferUsed=8;
	int rn=NetReadInt();
	so(2,format("letztes=%d   aktuelles=%d",con[s].recieve_no,rn));
	if ( rn<=con[s].recieve_no ){
		msg_error("ERROR (recv): signature missmatch, send_no too small");
		return false;
	}
	if ( rn>con[s].recieve_no+1 ){
		msg_error("ERROR (recv): signature missmatch, send_no too large");
	}
	con[s].recieve_no=rn;

	return true;
}

int NetReadInt()
{
	int i=*(int*)&Buffer[BufferUsed];
	BufferUsed+=sizeof(i);
	so(2,"read int");
	so(2,i);
	return i;
}

bool NetReadBool()
{
	bool b=*(bool*)&Buffer[BufferUsed];
	BufferUsed+=sizeof(b);
	so(2,"read bool");
	if (b)	so(2,"true");
	else	so(2,"false");
	return b;
}

float NetReadFloat()
{
	float f=*(float*)&Buffer[BufferUsed];
	BufferUsed+=sizeof(f);
	so(2,"read float");
	so(2,f2s(f,3));
	return f;
}

vector NetReadVector()
{
	vector v;
	v.x=NetReadFloat();
	v.y=NetReadFloat();
	v.z=NetReadFloat();
	return v;
}

char NetReadChar()
{
	char c=Buffer[BufferUsed];
	BufferUsed+=sizeof(c);
	so(2,"read char");
	so(2,c);
	return c;
}

string NetReadStr()
{
	so(2, "read string");
	int l = NetReadInt();
	BufferUsed += l;
	so(2, Buffer.substr(BufferUsed - l, BufferUsed));
	return Buffer.substr(BufferUsed - l, BufferUsed);
}

int NetReadBlockStart()
{
	if (BlockStartPos>=0)
		NetReadBlockEnd();
	BlockStartPos=BufferUsed;
	BlockSize=NetReadInt();
	int id=NetReadInt();
	so(2,"read block");
	return id;
}

void NetReadBlockEnd()
{
	if (BlockStartPos<0)
		return;
	if (BufferUsed!=BlockStartPos+BlockSize)
		so(1,"Net: block size mismatch");
	BufferUsed=BlockStartPos+BlockSize;
}

bool NetWriteBuffer(int s)
{
	so(2,"\n_________________________");
	so(1,"SENDING!...");
	so(2,s);
	so(2,con[s].s);
	*(char*)&Buffer[0]='m';
	*(char*)&Buffer[1]='x';
	*(char*)&Buffer[2]='t';
	*(char*)&Buffer[3]= con[s].host?'h':'c';
	*(int*)&Buffer[4]=BufferUsed;
	*(int*)&Buffer[8]=(++con[s].send_no);
	so(2,">>>");
	so(2,Buffer);
	so(2,d2h(Buffer.data,12));
	int sent=0;
	while(sent<BufferUsed){
		int to_send=BufferUsed-sent;
		int current=send(con[s].s,&Buffer[sent],(to_send>NET_MAX_SEND)?NET_MAX_SEND:to_send,0);
		so(2,current);
		if (current<=0){
			so(1,"ERROR (send)");
			con[s].connection_lost=true; // test me!!!
			return false;
		}
		so(2,Buffer.substr(sent, sent + current));
		sent+=current;
		so(1,format("%d/%d",sent,BufferUsed));
	}
	NetResetBuffer();
	return true;
}

int NetGetBufferPos()
{
	return BufferUsed;
}

void NetSetBufferPos(int pos)
{
	BufferUsed = pos;
}

void NetWriteInt(int i)
{
	so(2,"write int");
	so(2,i);
	*(int*)&Buffer[BufferUsed]=i;
	BufferUsed+=sizeof(i);
}

void NetWriteBool(bool b)
{
	so(2,"write bool");
	if (b)	so(2,"true");
	else	so(2,"false");
	*(bool*)&Buffer[BufferUsed]=b;
	BufferUsed+=sizeof(b);
}

void NetWriteFloat(float f)
{
	so(2,"write float");
	so(2,f2s(f,3));
	*(float*)&Buffer[BufferUsed]=f;
	BufferUsed+=sizeof(f);
}

void NetWriteVector(const vector &v)
{
	NetWriteFloat(v.x);
	NetWriteFloat(v.y);
	NetWriteFloat(v.z);
}

void NetWriteChar(char c)
{
	so(2,"write char");
	so(2,c);
	Buffer[BufferUsed]=c;
	BufferUsed+=sizeof(c);
}

void NetWriteStr(const string &str)
{
	NetWriteInt(str.num);
	memcpy(&Buffer[BufferUsed], str.data, str.num);
	BufferUsed += str.num;
	so(2,"write string");
	so(2,str);
}

void NetWriteBlockStart(int id)
{
	if (BlockStartPos>=0)
		NetWriteBlockEnd();
	so(2,"write block");
	BlockStartPos=BufferUsed;
	BlockSize=sizeof(int)*2;
	NetWriteInt(0); // dummy size
	NetWriteInt(id);
}

void NetWriteBlockEnd()
{
	BlockSize=BufferUsed-BlockStartPos;
	*(int*)&Buffer[BlockStartPos]=BlockSize; // insert the block size
	// no data?
	if (BlockSize==sizeof(int)*2)
		BufferUsed=BlockStartPos;
	BlockStartPos=-1;
}

bool NetReadyToWrite(int s)
{
	//so("teste socket...");
	fd_set r,w;
	FD_ZERO(&r);
	FD_SET(((unsigned)con[s].s),&r);
	FD_ZERO(&w);
	FD_SET(((unsigned)con[s].s),&w);
	struct timeval t;
	t.tv_sec=0;
	t.tv_usec=10;
	select(con[s].s+1,&r,&w,(fd_set*)0,&t);
	/*if (FD_ISSET(con[s].s,&w)>0)
		so("w=1");
	else
		so("w=0");*/
	return (FD_ISSET(con[s].s,&w)>0);
}

bool NetReadyToRead(int s)
{
	//so("teste socket...");
	fd_set r,w;
	FD_ZERO(&r);
	FD_SET(((unsigned)con[s].s),&r);
	FD_ZERO(&w);
	FD_SET(((unsigned)con[s].s),&w);
	struct timeval t;
	t.tv_sec=0;
	t.tv_usec=10;
	select(con[s].s+1,&r,&w,(fd_set*)0,&t);
	/*if (FD_ISSET(con[s].s,&r)>0)
		so("r=1");
	else
		so("r=0");*/
	return (FD_ISSET(con[s].s,&r)>0);
}

#define MAX_REPORT	16384

bool _cdecl NetSendBugReport(const string &sender, const string &program, const string &version, const string &comment, string &return_msg)
{
	NetInit();

	int s = NetConnect("michisoft.michi.is-a-geek.org", 80);
	if (s >= 0){
		string temp, report;

		// actual data to send
		report = "program=" + program + "&";
		report += "version=" + version + "&";
		report += "sender=" + sender + "&";//str_m2ascii(sender)));
		report += "comment=" + comment + "&";//str_m2ascii(comment)));
		report += "report=";
		report += msg_get_buffer(MAX_REPORT);


		
		// http header to wrap around
		temp += "POST /report.php HTTP/1.1\r\n";
		temp += "Host: michisoft.michi.is-a-geek.org\r\n";
		temp += "Content-Type: application/x-www-form-urlencoded\r\n";
		temp += format("Content-Length: %d\r\n", report.num);
		temp += "\r\n";
		temp += report;

		// try and send
		int sent = 0, r;
		for (int n=0;n<100;n++){
			r = NetWrite(s, temp.substr(sent, temp.num - sent));
			if (r > 0)
				sent += r;
		}
		if (sent < temp.num){
			return_msg = "Server accepted a connection but could not send any data";
			NetClose(s);
			return false;
		}

		// get response
		temp = NetRead(s);
		NetClose(s);
		if (temp.num == 0){
			return_msg = "Server does not respond";
			return false;
		}
		if (temp.find(" 200 OK") >= 0){
			if (temp.find("report: ok") >= 0){
				return_msg = "report successfully sent";
				return true;
			}else
				return_msg = "server could not save report to database :----(";
		}else
			return_msg = "bad server http response: " + temp;
	}else
		return_msg = "Could not connect to server";
	return false;
}

