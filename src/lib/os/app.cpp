//
// Created by michi on 7/23/25.
//

#include "app.h"
#include "filesystem.h"
#ifdef OS_WINDOWS
	#include <Windows.h>
#endif


namespace os::app {

	Path filename;
	Path directory_dynamic;
	Path directory_static;
	Path initial_working_directory;
	Path install_prefix;
	Path source_root;
	bool installed = false;

#ifdef OS_WINDOWS
	void* win_instance = nullptr;
#endif

	//   filename -> executable file
	//   directory_dynamic ->
	//      NONINSTALLED:  binary dir
	//      INSTALLED:     ~/.MY_APP/      <<< now always this
	//   directory_static ->
	//      NONINSTALLED:  binary dir/static/
	//      INSTALLED:     /usr/local/share/MY_APP/
	//   initial_working_directory -> working dir before running this program
	void detect(const Array<string> &arg, const string &app_name) {

		initial_working_directory = os::fs::current_directory();
		installed = false;

		// our build system should define these:
#ifdef SOURCE_ROOT
		source_root = SOURCE_ROOT;
#endif
#ifdef INSTALL_PREFIX
		install_prefix = INSTALL_PREFIX;
#else
		// oh no... fall-back
		install_prefix = "/usr/local";
#endif


		// executable file
#if defined(OS_LINUX) || defined(OS_MAC) || defined(OS_MINGW) //defined(__GNUC__) || defined(OS_LINUX)
		if (arg.num > 0)
			filename = arg[0];
#else // OS_WINDOWS
		char *ttt = nullptr;
		int r = _get_pgmptr(&ttt);
		filename = ttt;
		win_instance = (void*)GetModuleHandle(nullptr);
#endif


		// first, assume a local/non-installed version
		directory_dynamic = initial_working_directory; //strip_dev_dirs(filename.parent());
		if (source_root)
			directory_static = source_root | "static";
		else
			directory_static = directory_dynamic | "static";


		directory_static = source_root | "static";

#if defined(OS_LINUX) || defined(OS_MAC) || defined(OS_MINGW) //defined(__GNUC__) || defined(OS_LINUX)
		// installed version?
		if (filename.is_in(install_prefix) or (filename.str().find("/") < 0)) {
			installed = true;
			directory_static = install_prefix | "share" | app_name;
		}

		// inside an AppImage?
		if (getenv("APPIMAGE")) {
			installed = true;
			directory_static = Path(getenv("APPDIR")) | "usr" | "share" | app_name;
		}

		// inside MacOS bundle?
		if (str(filename).find(".app/Contents/MacOS/") >= 0) {
			installed = true;
			directory_static = filename.parent().parent() | "Resources";
		}

		directory_dynamic = format("%s/.%s/", getenv("HOME"), app_name);
		os::fs::create_directory(directory_dynamic);
#endif
	}




	Array<string> make_args(int num_args, char *args[]) {
		Array<string> a;
		for (int i=0; i<num_args; i++)
			a.add(args[i]);
		return a;
	}


	//----------------------------------------------------------------------------------
	// system independence of main() function

	int main(const Array<string> &);

}

// for a system independent usage of this library

#ifdef OS_WINDOWS

int main(int num_args, char* args[]) {
	return os::app::main(os::app::make_args(num_args, args));
}

#ifdef _CONSOLE

int _tmain(int num_args, _TCHAR *args[]) {
	return os::app::main(hui::make_args(num_args, args));
}

#else

// split by space... but parts might be in quotes "a b"
Array<string> parse_command_line(const string& s) {
	Array<string> a;
	a.add("-dummy-");

	for (int i=0; i<s.num; i++) {
		if (s[i] == '\"') {
			string t;
			bool escape = false;
			i ++;
			for (int j = i; j<s.num; j++) {
				i = j;
				if (escape) {
					escape = false;
				} else {
					if (s[j] == '\\')
						escape = true;
					else if (s[j] == '\"')
						break;
				}
				t.add(s[j]);
			}
			a.add(t.unescape());
			i ++;
		} else if (s[i] == ' ') {
			continue;
		} else {
			string t;
			for (int j=i; j<s.num; j++) {
				i = j;
				if (s[j] == ' ')
					break;
				t.add(s[j]);
			}
			a.add(t);
		}
	}
	return a;
}

int APIENTRY WinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPTSTR    lpCmdLine,
					 int       nCmdShow)
{
	return os::app::main(parse_command_line(lpCmdLine));
}

#endif

#endif
#if defined(OS_LINUX) || defined(OS_MAC) || defined(OS_MINGW)

int main(int num_args, char *args[]) {
	return os::app::main(os::app::make_args(num_args, args));
}

#endif




// usage:
// namespace os::app {
// int main(const Array<string> &arg) {
//     ....
//     return 0;
// }
// }



