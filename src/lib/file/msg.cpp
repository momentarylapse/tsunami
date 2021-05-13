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

#if HAS_LIB_UNWIND
#define UNW_LOCAL_ONLY
#include <cxxabi.h>
#include <libunwind.h>
#endif



// choose the level of debugging here
//    0 = very few messages
//  >10 = cruel amount of messages
#define MSG_DEBUG_OUTPUT_LEVEL		0

//#define MSG_LOG_TIMIGS
//#define MSG_LIKE_HTML



static string log_buffer;
static Array<int> log_pos;
static Array<int> log_length;
static Array<string> TodoStr;

bool msg_inited = true;

static File *file = nullptr;
static Path msg_file_name = "message.txt";
static int Shift;

static bool Verbose = true; // print to terminal?
static bool ErrorOccured;
static string ErrorMsg;

// call only once!
void msg_init(const Path &force_filename, bool verbose) {
	Verbose = verbose;
	Shift = 0;
	ErrorOccured = false;
	msg_inited = true;
	if (!force_filename.is_empty())
		msg_file_name = force_filename;
	if (!verbose)
		return;
	try {
		file = FileCreateText(msg_file_name);
	} catch(...) {}
#ifdef MSG_LOG_TIMIGS
	file->write_str("[hh:mm:ss, ms]");
#endif
}

void msg_init(bool verbose) {
	msg_init("", verbose);
}

void msg_set_verbose(bool verbose) {
	if (Verbose == verbose)
		return;
	if (verbose){
		try {
			file = FileCreateText(msg_file_name);
		} catch(...) {}
		Shift = 0;
	}else{
		msg_end(false);
	}
	Verbose = verbose;
}

void msg_add_str(const string &str) {
	int l = str.num;
	log_pos.add(log_buffer.num);
	log_length.add(l);
	log_buffer += str;
	log_buffer.add('\n');
	printf("%s\n", str.c_str());
#ifdef OS_WINDOWS
	fflush(stdout);
#endif
}



void msg_write(int i) {
	msg_write(i2s(i));
}

void msg_write(const string &str) {
	if (!Verbose)
		return;
	string s = string("\t").repeat(Shift) + str;
	msg_add_str(s.replace("\t", "    "));
	if (file)
		file->write_str(s);
}


void msg_error(const string &str) {
	int s = Shift;
	Shift = 0;
	msg_write("");
	msg_write("----------------------------- Error! -----------------------------");
	msg_write(str);
	msg_write("------------------------------------------------------------------");
	msg_write("");
	Shift = s;
	if (!ErrorOccured)
		ErrorMsg = str;
	ErrorOccured = true;
}

void msg_right() {
	Shift ++;
}

void msg_left() {
	Shift --;
	if (Shift < 0)
		Shift = 0;
}

void msg_reset_shift() {
	Shift = 0;
}

void msg_ok() {
	msg_write("-ok");
}


string msg_get_trace()
{
	Array<string> trace;

#if HAS_LIB_UNWIND
	unw_cursor_t cursor;
	unw_context_t context;

	unw_getcontext(&context);
	unw_init_local(&cursor, &context);

	while (unw_step(&cursor) > 0) {
		unw_word_t offset, pc;
		unw_get_reg(&cursor, UNW_REG_IP, &pc);
		if (pc == 0)
			break;

		// why before...?!?
		if (unw_is_signal_frame(&cursor) > 0)
			trace.clear();

		string t = format("0x%lx:", (int64)pc);

		char sym[256];
		if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
			char* nameptr = sym;
			int status;
			char* demangled = abi::__cxa_demangle(sym, nullptr, nullptr, &status);
			if (status == 0)
				nameptr = demangled;
			t += format(" (%s + 0x%lx)", nameptr, (int64)offset);
			free(demangled);
		} else {
			t += " (\?\?\?)";
		}
		trace.add(t);
	}
#endif

	return implode(trace, "\n");
}

void msg_end(bool del_file) {
	//if (!msg_inited)	return;
	if (!file)		return;
	if (!Verbose)	return;
	file->write_str("\n\n\n\n"\
" #                       # \n"\
"###                     ###\n"\
" #     natural death     # \n"\
" #                       # \n"\
" #        _              # \n"\
"       * / b   *^| _       \n"\
"______ |/ ______ |/ * _____\n");
	Verbose=false;
	msg_inited=false;
	file->close();
	delete(file);
	file = nullptr;
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
	return log_buffer.sub(log_pos[index], log_pos[index] + log_length[index]);
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
		return log_buffer.tail(max_size);
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
