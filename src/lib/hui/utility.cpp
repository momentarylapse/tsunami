#include "hui.h"

#ifdef OS_WINDOWS
	#include <direct.h>
	#include <tchar.h>
	#include <signal.h>
#endif
#ifdef OS_LINUX
	#include <sys/time.h>
	#include <unistd.h>
#endif


namespace hui
{



// set the default directory
void SetDirectory(const string &dir)
{
#ifdef OS_WINDOWS
	_chdir(dir.sys_filename().c_str());
#endif
#ifdef OS_LINUX
	int r=chdir(sys_str_f(dir));
#endif
}

int GetCpuCount()
{
#ifdef OS_WINDOWS
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
#endif
#ifdef OS_LINUX
	return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}



void OpenDocument(const string &filename)
{
#ifdef OS_WINDOWS
	ShellExecute(NULL,_T(""),hui_tchar_str(filename),_T(""),_T(""),SW_SHOW);
#endif
#ifdef OS_LINUX
	int r=system(format("gnome-open '%s'", filename.c_str()).c_str());
#endif
}

};

