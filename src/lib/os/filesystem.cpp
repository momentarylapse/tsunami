#include "filesystem.h"
#include "../base/sort.h"
#include "date.h"
#include "file.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <cerrno>

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

namespace os::fs {

bool func_did_not_throw(std::function<void()> f) {
	try {
		f();
		return true;
	} catch (...) {
		return false;
	}
}


// just test the existence of a file
bool exists(const Path &filename) {
	struct stat s;
	if (stat(filename.str().c_str(), &s) == 0) {
		//if (s.st_mode & S_IFREG)
			return true;
	}
	return false;
}

int64 size(const Path &path) {
	struct stat s;
	if (stat(path.str().c_str(), &s) != 0)
		throw FileError(format("unable to stat '%s'", path));
	return s.st_size;
}

Date mtime(const Path &path) {
	struct stat s;
	if (stat(path.str().c_str(), &s) != 0)
		throw FileError(format("unable to stat '%s'", path));
	return time2date(s.st_mtime);
}

bool is_directory(const Path &path) {
	struct stat s;
	if (stat(path.str().c_str(), &s) != 0)
		return false; //throw FileError("unable to stat '" + path + "'");
	return (s.st_mode & S_IFDIR);
}


void create_directory(const Path &dir) {
	if (dir.is_empty())
		return;
	if (is_directory(dir))
		return;
#if defined(OS_WINDOWS)
	if (_mkdir(dir.str().c_str()) != 0)
#elif defined(OS_MINGW)
	if (mkdir(dir.str().c_str()) != 0)
#else // defined(OS_LINUX)
	if (mkdir(dir.str().c_str(),S_IRWXU | S_IRWXG | S_IRWXO) != 0)
#endif
		throw FileError(format("can not create directory '%s'", dir));
}

void delete_directory(const Path &dir) {
	if (_rmdir(dir.str().c_str()) != 0)
		throw FileError(format("can not delete directory '%s'", dir));
}

Path current_directory() {
	string str;
	char tmp[256];
#ifdef OS_WINDOWS
	static_cast<void>(_getcwd(tmp, sizeof(tmp)));
	str = tmp;
	str += "\\";
#else // defined(OS_LINUX) || defined(OS_MINGW)
	static_cast<void>(getcwd(tmp, sizeof(tmp)));
	str = tmp;
	str += "/";
#endif
	return Path(str);
}

void rename(const Path &source, const Path &target) {
	for (auto &p: target.all_parents())
		create_directory(p);

	// linux automatically overwrites, windows will fail rename()
	if (exists(target))
		_delete(target);

	if (::rename(source.str().c_str(), target.str().c_str()) != 0)
		throw FileError(format("can not rename file '%s' -> '%s'", source, target));
}

void move(const Path &source, const Path &target) {
	for (auto &p: target.all_parents())
		create_directory(p);

	// linux automatically overwrites, windows will fail rename()
	if (exists(target))
		_delete(target);

	int r = ::rename(source.str().c_str(), target.str().c_str());
	if ((r != 0) and (errno == EXDEV)) {
		copy(source, target);
		_delete(source);
	} else if (r != 0) {
		throw FileError(format("can not move file '%s' -> '%s' (%s)", source, target, strerror(r)));
	}
}

void copy(const Path &source, const Path &target) {
	for (auto &p: target.all_parents())
		create_directory(p);

	int hs = ::_open(source.str().c_str(),O_RDONLY);
	if (hs < 0)
		throw FileError(format("copy: can not open source file '%s'", source));
#ifdef OS_WINDOWS
	int ht = ::_creat(target.str().c_str(),_S_IREAD | _S_IWRITE);
	_setmode(hs,_O_BINARY);
	_setmode(ht,_O_BINARY);
#else // defined(OS_LINUX) || defined(OS_MINGW)
	int ht = ::creat(target.str().c_str(),S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
#endif
	if (ht < 0){
		_close(hs);
		throw FileError(format("copy: can not create target file '%s'", target));
	}
	char buf[10240];
	int r = 10;
	while(r > 0){
		r = ::_read(hs,buf,sizeof(buf));
		int rr = ::_write(ht,buf,r);
		if (rr < r)
			throw FileError(format("can not copy file '%s' -> '%s' (write failed)", source, target));
	}
	_close(hs);
	_close(ht);
}

void _delete(const Path &filename) {
	if (_unlink(filename.str().c_str()) != 0)
		throw FileError(format("can not delete file '%s'", filename));
}

string hash(const Path &filename, const string &type) {
	if (type == "md5") {
		return read_binary(filename).md5();
	}
	return "";
}



// search a directory for files matching a filter
void search_single(const Path &dir, const string &filter, Array<Path> &dir_list, Array<Path> &file_list) {
	string filter2 = filter.sub(1);

#ifdef OS_WINDOWS
	static _finddata_t t;
	auto handle = _findfirst((dir.as_dir().str() + "*").c_str(), &t);
	auto e = handle;
	while (e >= 0) {
		string name = t.name;
		//if ((strcmp(t.name,".")!=0)and(strcmp(t.name,"..")!=0)and(strstr(t.name,"~")==NULL)){
		if ((name != ".") and (name != "..") and (name.back() != '~')) {
			if (name.match(filter) or (t.attrib == _A_SUBDIR)) {
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
	_dir = opendir(dir.str().c_str());
	if (!_dir)
		return;
	struct dirent *dn;
	dn = readdir(_dir);
	struct stat s;
	while (dn) {
		//if ((strcmp(dn->d_name,".")!=0)and(strcmp(dn->d_name,"..")!=0)and(!strstr(dn->d_name,"~"))){
		string name = dn->d_name;
		if ((name != ".") and (name != "..") and (name.back() != '~')) {
			Path ffn = dir | name;
			stat(ffn.str().c_str(), &s);
			bool is_reg = (s.st_mode & S_IFREG) > 0;
			bool is_dir = (s.st_mode & S_IFDIR) > 0;
			if ((is_reg and name.match(filter)) or is_dir) {
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
	base::inplace_sort(dir_list, [](const Path &a, const Path &b) { return a <= b; });
	base::inplace_sort(file_list, [](const Path &a, const Path &b) { return a <= b; });
}

void search_single_rec(const Path &dir0, const Path &subdir, const string &filter, Array<Path> &dir_list, Array<Path> &file_list) {
	Array<Path> sub_dir_list, sub_file_list;
	search_single(dir0 | subdir, filter, sub_dir_list, sub_file_list);
	for (auto &x: sub_dir_list) {
		dir_list.add(subdir | x);
		search_single_rec(dir0, subdir | x, filter, dir_list, file_list);
	}
	for (auto &x: sub_file_list)
		file_list.add(subdir | x);
}

// search a directory for files matching a filter
Array<Path> search(const Path &dir, const string &filter, const string &options) {
	Array<Path> dir_list, file_list;

	bool show_files = options.find("f") >= 0;
	bool show_dirs = options.find("d") >= 0;
	bool show_recursive = options.find("r") >= 0;
	bool show_self = options.find("0") >= 0;

	if (show_recursive) {
		search_single_rec(dir, "", filter, dir_list, file_list);
	} else {
		search_single(dir, filter, dir_list, file_list);
	}
	if (show_self)
		dir_list.insert("", 0);
	
	Array<Path> r;
	if (show_dirs)
		r.append(dir_list);
	if (show_files)
		r.append(file_list);
	return r;
}

}


