#include "hui.h"

#ifdef OS_WINDOWS
	#include <direct.h>
	#include <tchar.h>
	#include <signal.h>
	#include <windows.h>
#endif
#if defined(OS_LINUX) || defined(OS_MAC)
	#include <unistd.h>
#endif


#ifdef OS_WINDOWS
const TCHAR* hui_tchar_str(const string& str);
#endif

namespace hui
{

void open_document(const Path &filename) {
#ifdef OS_WINDOWS
	ShellExecute(NULL, _T(""), hui_tchar_str(filename.str()), _T(""), _T(""), SW_SHOW);
#endif
#ifdef OS_LINUX
	static_cast<void>(system(format("gnome-open '%s'", filename).c_str()));
#endif
}

};

