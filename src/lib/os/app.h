#ifndef LIB_OS_APP_H
#define LIB_OS_APP_H

#include "path.h"

namespace os::app {

	void detect(const Array<string> &arg, const string &app_name);

	extern Path filename;          // executable
	extern Path directory_dynamic; // dir of changeable files (i.e. ~/.app/)
	extern Path directory_static;  // dir of static files (e.g. /usr/local/share/app)
	extern Path initial_working_directory;
	extern Path install_prefix;    // (e.g. /usr/local)
	extern Path source_root;       // root of project's source (repository)
	extern bool installed;         // installed into system folders?

}


#endif
