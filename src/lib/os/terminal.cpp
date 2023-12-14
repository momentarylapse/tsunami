#include "terminal.h"
#include <stdio.h>

namespace os::terminal {


const string RED = "\x1b[0;31m";
const string GREEN = "\x1b[0;32m";
const string YELLOW = "\x1b[0;33m";
const string BLUE = "\x1b[0;34m";
const string MAGENTA = "\x1b[0;35m";
const string CYAN = "\x1b[0;36m";
const string GRAY = "\x1b[38;5;245m";
const string DARK_GRAY = "\x1b[0;90m";
const string ORANGE = "\x1b[38;5;209m";
const string BOLD = "\x1b[1m";
const string END = "\x1b[0m";


string _print_postfix_ = "\n";
void print(const string &s) {
	printf("%s%s", s.c_str(), _print_postfix_.c_str()); fflush(stdout);
}

string shell_execute(const string &cmd) {
#if defined(OS_LINUX) || defined(OS_MAC)
	// thread safe...
	char *s = new char[cmd.num + 1];
	memcpy(s, cmd.data, cmd.num);
	s[cmd.num] = 0;
	FILE *f = popen(s, "r");
	delete[] s;
	//FILE *f = popen(cmd.c_str(), "r");
	string buffer;

	while (true) {
		int c = fgetc(f);
		if (c == EOF)
			break;
		buffer.add(c);
	}

	int r = pclose(f);
//	int r = system(cmd.c_str());
	if (r != 0)
		throw Exception("failed to run shell command");
	return buffer;
#else
	return "";
#endif
}

}

