/*----------------------------------------------------------------------------*\
| msg                                                                          |
| -> debug logfile system                                                      |
| -> "message.txt" used for output                                             |
|                                                                              |
| vital properties:                                                            |
|  - stores the latest messages in memory for direct access                    |
|                                                                              |
| last updated: 2010.07.14 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#include "file.h"
#include <stdio.h>
#include <stdarg.h>

#ifdef OS_WINDOWS
	#include <windows.h>
#endif



// choose the level of debugging here
//    0 = very few messages
//  >10 = cruel amount of messages
#define MSG_DEBUG_OUTPUT_LEVEL		0

//#define MSG_LOG_TIMIGS
//#define MSG_LIKE_HTML
#define MSG_TRACE_REF



static string log_buffer;
static Array<int> log_pos;
static Array<int> log_length;
static Array<string> TodoStr;

bool msg_inited = false;

static CFile *file = NULL;
static string msg_file_name = "message.txt";
static int Shift;

static bool Verbose=false;
static bool ErrorOccured;
static string ErrorMsg;

// tracing system
#define MSG_NUM_TRACES_SAVED		256
#define MSG_MAX_TRACE_LENGTH		96

#ifdef MSG_TRACE_REF
	static const char *TraceStr[MSG_NUM_TRACES_SAVED];
#else
	dont
	static char TraceStr[MSG_NUM_TRACES_SAVED][MSG_MAX_TRACE_LENGTH];
#endif
static int CurrentTraceLevel=0;



// call only once!
void msg_init(const string &force_filename, bool verbose)
{
	Verbose = false;
	file = new CFile();
	Shift = 0;
	ErrorOccured = false;
	msg_inited = true;
	if (force_filename != "")
		msg_file_name = force_filename;
	if (!verbose)
		return;
	file->Create(msg_file_name);
	Verbose = verbose;
#ifdef MSG_LOG_TIMIGS
	file->WriteStr("[hh:mm:ss, ms]");
#endif
}

void msg_init(bool verbose)
{
	msg_init("", verbose);
}

void msg_set_verbose(bool verbose)
{
	if (Verbose == verbose)
		return;
	if (verbose){
		file->Create(msg_file_name);
		Shift = 0;
	}else{
		msg_end(false);
	}
	Verbose = verbose;
}

static void _strcpy_save_(char *a, const char *b,int max_length)
{
	int l=strlen(b);
	if (l>max_length-1)
		l=max_length-1;
	memcpy(a,b,l);
	a[l]=0;
}

void msg_add_str(const string &str)
{
	if (!Verbose)	return;
	int l = str.num;
	log_pos.add(log_buffer.num);
	log_length.add(Shift * 4 + l);
	for (int i=0;i<Shift;i++)
		log_buffer += "    ";
	log_buffer += str;
	log_buffer.add('\n');
	for (int i=0;i<Shift;i++)
		printf("    ");
	printf("%s\n", str.c_str());
}

void write_date()
{
#ifdef MSG_LOG_TIMIGS
	sDate t=get_current_date();
	char tstr[128];
	sprintf(tstr,"[%d%d:%d%d:%d%d,%d%d%d]\t",	t.hour/10,t.hour%10,
												t.minute/10,t.minute%10,
												t.second/10,t.second%10,
												t.milli_second/100,(t.milli_second/10)%10,t.milli_second%10);
	file->WriteBuffer(tstr,strlen(tstr));
	printf(tstr);
#endif
}

void msg_write(int i)
{
	if (!Verbose)	return;
	write_date();
	file->ShiftRight(Shift);
	file->WriteInt(i);

	string s = i2s(i);
	msg_add_str(s);
}

void msg_write(const string &str)
{
	if (!Verbose)	return;
	write_date();
	file->ShiftRight(Shift);
	file->WriteStr(str);

	msg_add_str(str);
}

void msg_write(const char *str)
{
	if (!Verbose)	return;
	write_date();
	file->ShiftRight(Shift);
	string s = string(str);
	file->WriteStr(s);

	msg_add_str(s);
}

#if 0
void msg_write2(const char *str,...)
{
#if 1
	va_list arg;
	va_start(arg,str);
	msg_write(string2(str,arg));
#else
#ifdef OS_WINDOWS
	char tmp[1024];
	tmp[0]=0;

	va_list marker;
	va_start(marker,str);

	int l=0,s=strlen(str);
	for (int i=0;i<s;i++){
		if ((str[i]=='%')&&(str[i+1]=='s')){
			strcat(tmp,va_arg(marker,char*));
			i++;
			l=strlen(tmp);
		}else if ((str[i]=='%')&&(str[i+1]=='d')){
			strcat(tmp,i2s(va_arg(marker,int)));
			i++;
			l=strlen(tmp);
		}else if ((str[i]=='%')&&(str[i+1]=='f')){
			int fl=3;
			if (str[i+2]==':'){
				fl=str[i+3]-'0';
				i+=3;
			}else
				i++;
			strcat(tmp,f2s((float)va_arg(marker,double),fl));
			l=strlen(tmp);
		}else if ((str[i]=='%')&&(str[i+1]=='v')){
			int fl=3;
			if (str[i+2]==':'){
				fl=str[i+3]-'0';
				i+=3;
			}else
				i++;
			/*float *v=(float*)&va_arg(marker,double);
			va_arg(marker,float);
			va_arg(marker,float);
			strcat(tmp,"( ");
			strcat(tmp,f2s(v[0],fl));
			strcat(tmp," , ");
			strcat(tmp,f2s(v[1],fl));
			strcat(tmp," , ");
			strcat(tmp,f2s(v[2],fl));
			strcat(tmp," )");
			l=strlen(tmp);*/
msg_write>Error("Todo:  %v");
		}else{
			tmp[l]=str[i];
			tmp[l+1]=0;
			l++;
		}
	}
	va_end(marker);
	write(tmp);
#endif
#endif
}
#endif

void msg_error(const string &str)
{
	if (!Verbose)	return;
	int s=Shift;
	Shift=0;
	msg_write("");
	msg_write("----------------------------- Error! -----------------------------");
	msg_write(str);
	msg_write("------------------------------------------------------------------");
	msg_write("");
	Shift=s;
	if (!ErrorOccured)
		ErrorMsg = str;
	ErrorOccured=true;
}

void msg_error(const char *str)
{
	msg_error(string(str));
}

void msg_right()
{
	if (!Verbose)	return;
	Shift++;
}

void msg_left()
{
	if (!Verbose)	return;
	Shift--;
	if (Shift<0)
		Shift=0;
}

void msg_reset_shift()
{
	if (!Verbose)	return;
	Shift=0;
}

void msg_ok()
{
	msg_write("-ok");
}

void msg_trace_r(const char *str,int level)
{
	if (CurrentTraceLevel >= MSG_NUM_TRACES_SAVED)
		return;
#ifdef MSG_TRACE_REF
	TraceStr[CurrentTraceLevel ++] = str;
#else
	char *mstr=TraceStr[CurrentTraceLevel++];
	_strcpy_save_(mstr,str,MSG_MAX_TRACE_LENGTH);
	strcpy(TraceStr[CurrentTraceLevel],"");
#endif
	if (MSG_DEBUG_OUTPUT_LEVEL>=level){//CurrentTraceLevel){
#ifdef MSG_LIKE_HTML
		msg_write(string("<",str,">"));
#else
		msg_write(str);
#endif
		msg_right();
	}
}

void msg_trace_m(const char *str,int level)
{
	if (CurrentTraceLevel >= MSG_NUM_TRACES_SAVED)
		return;
#ifdef MSG_TRACE_REF
	TraceStr[CurrentTraceLevel] = str;
#else
	_strcpy_save_(TraceStr[CurrentTraceLevel],str,MSG_MAX_TRACE_LENGTH);
#endif
	if (MSG_DEBUG_OUTPUT_LEVEL >= level)//CurrentTraceLevel)
		msg_write(str);
}

void msg_trace_l(int level)
{
#ifdef MSG_TRACE_REF
	TraceStr[CurrentTraceLevel--] = NULL;
#else
	strcpy(TraceStr[CurrentTraceLevel--],"");
#endif
	if (CurrentTraceLevel<0){
		msg_error("msg_trace_l(): level below 0!");
		CurrentTraceLevel=0;
	}
	if (MSG_DEBUG_OUTPUT_LEVEL>=level){//CurrentTraceLevel){
#ifdef MSG_LIKE_HTML
		msg_left();
		msg_write(string("</",TraceStr[CurrentTraceLevel],">"));
#else
		msg_ok();
		msg_left();
#endif
	}
}

string msg_get_trace()
{
	string str;
	for (int i=0;i<CurrentTraceLevel;i++){
		str += string(TraceStr[i]);
		if (i<CurrentTraceLevel-1)
			str += "  ->  ";
	}
#ifdef MSG_TRACE_REF
	if (TraceStr[CurrentTraceLevel])
		str += string(" ( ->  ") + TraceStr[CurrentTraceLevel] + ")";
#else
	if (strlen(TraceStr[CurrentTraceLevel])>0)
		str += string(" ( ->  ") + TraceStr[CurrentTraceLevel] + ")";
#endif
	return str;
}

int msg_get_trace_depth()
{
	return CurrentTraceLevel;
}

void msg_set_trace_depth(int depth)
{
	if (depth < CurrentTraceLevel)
		depth = CurrentTraceLevel;
}

void msg_end(bool del_file)
{
	//if (!msg_inited)	return;
	if (!file)		return;
	if (!Verbose)	return;
	file->WriteStr("\n\n\n\n"\
" #                       # \n"\
"###                     ###\n"\
" #     natural death     # \n"\
" #                       # \n"\
" #        _              # \n"\
"       * / b   *^| _       \n"\
"______ |/ ______ |/ * _____\n");
	Verbose=false;
	msg_inited=false;
	file->Close();
	if (del_file){
		delete(file);
		file=NULL;
	}
}

void msg_db_out(int dl,const char *str)
{
	if (!Verbose)	return;
	if (dl<=MSG_DEBUG_OUTPUT_LEVEL)
		msg_write(str);
}

// index = 0   -> latest log
string msg_get_str(int index)
{
	if (!Verbose)
		return "";
	index = (log_pos.num - 1 - index);
	if (index < 0)
		return "";
	return log_buffer.substr(log_pos[index], log_length[index]);
}

int msg_get_buffer_size()
{
	return log_buffer.num;
}

string msg_get_buffer(int max_size)
{
	if (log_buffer.num < max_size)
		return log_buffer;
	else
		// not all -> use only the latest log
		return log_buffer.substr(log_buffer.num - max_size, max_size);
}

void msg_todo(const string &str)
{
	for (int i=0;i<TodoStr.num;i++)
		if (TodoStr[i] == str)
			return;
	TodoStr.add(str);
	string s = "TODO (Engine): " + str;
	msg_error(s);
}


MsgBlockTracer::MsgBlockTracer(const char *str, int _level)
{
	msg_trace_r(str, _level);
	level = _level;
};

MsgBlockTracer::~MsgBlockTracer()
{
	msg_trace_l(level);
}
