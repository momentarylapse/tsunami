#include "file.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>

#ifdef OS_WINDOWS
	#include <io.h>
	#include <direct.h>
#endif
#if defined (OS_LINUX) || defined(OS_MINGW)
	#include <unistd.h>
	#include <dirent.h>
	//#include <sys/timeb.h>
 
	 
	#define _open	open
	#define _read	read
	#define _write	write
	#define _lseek	lseek
	#define _close	close
	#define _rmdir	rmdir
	#define _unlink	unlink
	inline unsigned char to_low(unsigned char c) {
		if ((c >= 'A') and (c <= 'Z'))
			return c - 'A' + 'a';
		return c;
	}
#endif
#ifdef OS_LINUX
	int _stricmp(const char*a,const char*b) {
		unsigned char a_low = to_low(*a);
		unsigned char b_low = to_low(*b);
		while ((*a != 0) and (*b != 0)) {
			if (a_low != b_low)
				break;
			a++,++b;
			a_low = to_low(*a);
			b_low = to_low(*b);
		}
		return a_low - b_low;
	}
#endif

Date time2date(time_t t);

bool func_did_not_throw(std::function<void()> f) {
	try {
		f();
		return true;
	} catch (...) {
		return false;
	}
}


// just test the existence of a file
bool file_exists(const string &filename) {
	struct stat s;
	if (stat(sys_filename(filename).c_str(), &s) == 0) {
		//if (s.st_mode & S_IFREG)
			return true;
	}
	return false;
}

int64 file_size(const string &path) {
	struct stat s;
	if (stat(sys_filename(path).c_str(), &s) != 0)
		throw FileError("unable to stat '" + path + "'");
	return s.st_size;
}

Date file_mtime(const string &path) {
	struct stat s;
	if (stat(sys_filename(path).c_str(), &s) != 0)
		throw FileError("unable to stat '" + path + "'");
	return time2date(s.st_mtime);
}

bool file_is_directory(const string &path) {
	struct stat s;
	if (stat(sys_filename(path).c_str(), &s) != 0)
		return false; //throw FileError("unable to stat '" + path + "'");
	return (s.st_mode & S_IFDIR);
}


void dir_create(const string &dir) {
	if (file_is_directory(dir))
		return;
#if defined(OS_WINDOWS)
	if (_mkdir(sys_filename(dir).c_str()) != 0)
#elif defined(OS_MINGW)
	if (mkdir(sys_filename(dir).c_str()) != 0)
#else // defined(OS_LINUX)
	if (mkdir(sys_filename(dir).c_str(),S_IRWXU | S_IRWXG | S_IRWXO) != 0)
#endif
		throw FileError("can not create directory '" + dir + "'");
}

void dir_delete(const string &dir) {
	if (_rmdir(sys_filename(dir).c_str()) != 0)
		throw FileError("can not delete directory '" + dir + "'");
}

string get_current_dir() {
	string str;
	char tmp[256];
#ifdef OS_WINDOWS
	char *r = _getcwd(tmp, sizeof(tmp));
	str = tmp;
	str += "\\";
#else // defined(OS_LINUX) || defined(OS_MINGW)
	char *r = getcwd(tmp, sizeof(tmp));
	str = tmp;
	str += "/";
#endif
	return str;
}

void file_rename(const string &source, const string &target) {
	char dir[512];
	for (int i=0; i<target.num; i++){
		dir[i] = target[i];
		dir[i+1] = 0;
		if (i > 3)
			if ((target[i] == '/') or (target[i] == '\\'))
				dir_create(string(dir));
	}
	if (rename(sys_filename(source).c_str(), sys_filename(target).c_str()) != 0)
		throw FileError("can not rename file '" + source + "' -> '" + target + "'");
}

void file_copy(const string &source, const string &target) {
	char dir[512];
	for (int i=0; i<target.num; i++){
		dir[i] = target[i];
		dir[i+1] = 0;
		if (i > 3)
			if ((target[i] == '/') or (target[i] == '\\'))
				dir_create(string(dir));
	}
	int hs=_open(sys_filename(source).c_str(),O_RDONLY);
	if (hs<0)
		throw FileError("copy: can not open source file '" + source + "'");
#ifdef OS_WINDOWS
	int ht=_creat(sys_filename(target).c_str(),_S_IREAD | _S_IWRITE);
	_setmode(hs,_O_BINARY);
	_setmode(ht,_O_BINARY);
#else // defined(OS_LINUX) || defined(OS_MINGW)
	int ht = creat(sys_filename(target).c_str(),S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
#endif
	if (ht < 0){
		_close(hs);
		throw FileError("copy: can not create target file '" + target + "'");
	}
	char buf[10240];
	int r = 10;
	while(r > 0){
		r = _read(hs,buf,sizeof(buf));
		int rr = _write(ht,buf,r);
		if (rr < r)
			throw FileError("can not copy file '" + source + "' -> '" + target + "' (write failed)");
	}
	_close(hs);
	_close(ht);
}

void file_delete(const string &filename) {
	if (_unlink(sys_filename(filename).c_str()) != 0)
		throw FileError("can not delete file '" + filename + "'");
}

string file_hash(const string &filename, const string &type) {
	if (type == "md5") {
		return FileRead(filename).md5();
	}
	return "";
}


// transposes path-strings to the current operating system
// accepts windows and linux paths ("/" and "\\")
string sys_filename(const string &path) {
#if defined(OS_WINDOWS) || defined(OS_MINGW)
	return path.replace("/", "\\");
#else
	return path.replace("\\", "/");
#endif
}

// ends with '/' or '\'
string path_dirname(const string &path) {
	int i = max(path.rfind("/"), path.rfind("\\"));
	if (i >= 0)
		return path.head(i + 1);
	return "";
}

string path_basename(const string &path) {
	int i = max(path.rfind("/"), path.rfind("\\"));
	if (i >= 0)
		return path.tail(path.num - i - 1);
	return path;
}

// make sure the name ends with a slash
string dir_canonical(const string &path) {
	string p = path_canonical(path);
	if (p.tail(1) != "/")
		return p + "/";
	return p;
}

// remove "/../"
string path_canonical(const string &path) {
	auto p = path.replace("\\", "/").explode("/");

	for (int i=1; i<p.num; i++)
		if ((p[i] == "..") and (p[i-1] != "..")){
			p.erase(i);
			p.erase(i - 1);
			i -= 2;
		}

	return implode(p, "/");
}

string path_extension(const string &path) {
	int pos = path.rfind(".");
	if (pos >= 0)
		return path.tail(path.num - pos - 1).lower();
	return "";
}


string path_absolute(const string &path) {
	if (path.head(1) == "/" or path.substr(1,1) == ":")
		return path;
	return path_canonical(get_current_dir() + path);
}


string shell_execute(const string &cmd) {
#ifdef OS_LINUX
	FILE *f = popen(cmd.c_str(), "r");
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


void sa_sort_i(Array<string> &a) {
	for (int i=0; i<a.num-1; i++)
		for (int j=i+1; j<a.num; j++)
			if (a[i].icompare(a[j]) > 0)
				a.swap(i, j);
}

// search a directory for files matching a filter
Array<string> dir_search(const string &dir, const string &filter, bool show_directories) {
	Array<string> dir_list, file_list;

	string filter2 = filter.substr(1, filter.num - 1);
	string dir2 = sys_filename(dir_canonical(dir));

#ifdef OS_WINDOWS
	static _finddata_t t;
	auto handle = _findfirst((dir2 + "*").c_str(), &t);
	auto e = handle;
	while (e >= 0) {
		string name = t.name;
		//if ((strcmp(t.name,".")!=0)and(strcmp(t.name,"..")!=0)and(strstr(t.name,"~")==NULL)){
		if ((name != ".") and (name != "..") and (name.back() != '~')) {
			if (name.match(filter) or (show_directories and (t.attrib == _A_SUBDIR))) {
				if (t.attrib == _A_SUBDIR)
					dir_list.add(name);
				else
					file_list.add(name);
			}
		}
		e = _findnext(handle,&t);
	}
#else // defined(OS_LINUX) || defined(OS_MINGW)
	DIR *_dir;
	_dir = opendir(dir2.c_str());
	if (!_dir)
		return {};
	struct dirent *dn;
	dn = readdir(_dir);
	struct stat s;
	while (dn) {
		//if ((strcmp(dn->d_name,".")!=0)and(strcmp(dn->d_name,"..")!=0)and(!strstr(dn->d_name,"~"))){
		string name = dn->d_name;
		if ((name != ".") and (name != "..") and (name.back() != '~')) {
			string ffn = dir2 + name;
			stat(ffn.c_str(), &s);
			bool is_reg = (s.st_mode & S_IFREG)>0;
			bool is_dir = (s.st_mode & S_IFDIR)>0;
			if ((is_reg and name.match(filter)) or (show_directories and is_dir)) {
				if (is_dir)
					dir_list.add(name);
				else
					file_list.add(name);
			}
		}
		dn = readdir(_dir);
	}
	closedir(_dir);
#endif

	
	// sorting...
	sa_sort_i(dir_list);
	sa_sort_i(file_list);
	return dir_list + file_list;
}



