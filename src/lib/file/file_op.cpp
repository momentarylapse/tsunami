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
	inline unsigned char to_low(unsigned char c)
	{
		if ((c>='A')and(c<='Z'))
			return c-'A'+'a';
		return c;
	}
#endif
#ifdef OS_LINUX
	int _stricmp(const char*a,const char*b)
	{
		unsigned char a_low=to_low(*a);
		unsigned char b_low=to_low(*b);
		while((*a!=0)and(*b!=0)){
			if (a_low!=b_low)	break;
			a++,++b;
			a_low=to_low(*a);
			b_low=to_low(*b);
		}
		return a_low-b_low;
	}
#endif


// just test the existence of a file
bool file_test_existence(const string &filename)
{
	struct stat s;
	if (stat(filename.sys_filename().c_str(), &s) == 0){
		//if (s.st_mode & S_IFREG)
			return true;
	}
	return false;
}

bool file_is_directory(const string &path)
{
	struct stat s;
	if (stat(path.sys_filename().c_str(), &s) == 0){
		if (s.st_mode & S_IFDIR)
			return true;
	}
	return false;
}


bool dir_create(const string &dir)
{
#if defined(OS_WINDOWS)
	return (_mkdir(dir.sys_filename().c_str()) == 0);
#elif defined(OS_MINGW)
	return (mkdir(dir.sys_filename().c_str()) == 0);
#else // defined(OS_LINUX)
	return (mkdir(dir.sys_filename().c_str(),S_IRWXU | S_IRWXG | S_IRWXO) == 0);
#endif
	return false;
}

bool dir_delete(const string &dir)
{
	return (_rmdir(dir.sys_filename().c_str())==0);
}

string get_current_dir()
{
	string str;
	char tmp[256];
#ifdef OS_WINDOWS
	char *r=_getcwd(tmp, sizeof(tmp));
	str = tmp;
	str += "\\";
#else // defined(OS_LINUX) || defined(OS_MINGW)
	char *r=getcwd(tmp, sizeof(tmp));
	str = tmp;
	str += "/";
#endif
	return str;
}

bool file_rename(const string &source,const string &target)
{
	char dir[512];
	for (int i=0;i<target.num;i++){
		dir[i]=target[i];
		dir[i+1]=0;
		if (i>3)
			if ((target[i]=='/')or(target[i]=='\\'))
				dir_create(string(dir));
	}
	return (rename(source.sys_filename().c_str(), target.sys_filename().c_str())==0);
}

bool file_copy(const string &source,const string &target)
{
	char dir[512];
	for (int i=0;i<target.num;i++){
		dir[i]=target[i];
		dir[i+1]=0;
		if (i>3)
			if ((target[i]=='/')or(target[i]=='\\'))
				dir_create(string(dir));
	}
	int hs=_open(source.sys_filename().c_str(),O_RDONLY);
	if (hs<0)
		return false;
#ifdef OS_WINDOWS
	int ht=_creat(target.sys_filename().c_str(),_S_IREAD | _S_IWRITE);
	_setmode(hs,_O_BINARY);
	_setmode(ht,_O_BINARY);
#else // defined(OS_LINUX) || defined(OS_MINGW)
	int ht=creat(target.sys_filename().c_str(),S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
#endif
	if (ht<0){
		_close(hs);
		return false;
	}
	char buf[10240];
	int r=10;
	while(r>0){
		r=_read(hs,buf,sizeof(buf));
		int rr=_write(ht,buf,r);
	}
	_close(hs);
	_close(ht);
	return true;
}

bool file_delete(const string &filename)
{
	return (_unlink(filename.sys_filename().c_str())==0);
}

string file_hash(const string &filename, const string &type)
{
	if (type == "md5"){
		return FileRead(filename).md5();
	}
	return "";
}

string shell_execute(const string &cmd)
{
#ifdef OS_LINUX
	FILE *f = popen(cmd.c_str(), "r");
	string buffer;

	while(true){
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



// search a directory for files matching a filter
Array<DirEntry> dir_search(const string &dir, const string &filter, bool show_directories)
{
	Array<DirEntry> entry_list;
	DirEntry entry;


	string filter2 = filter.substr(1, filter.num - 1);
	string dir2 = dir;
	dir2.dir_ensure_ending();
	dir2 = dir2.sys_filename();

#ifdef OS_WINDOWS
	static _finddata_t t;
	auto handle=_findfirst((dir2 + "*").c_str(), &t);
	auto e=handle;
	while(e>=0){
		string name = t.name;
		//if ((strcmp(t.name,".")!=0)and(strcmp(t.name,"..")!=0)and(strstr(t.name,"~")==NULL)){
		if ((name != ".") and (name != "..") and (name.back() != '~')){
			if (name.match(filter) or ((show_directories) and (t.attrib==_A_SUBDIR)) ){
				entry.name = name;
				entry.is_dir = (t.attrib == _A_SUBDIR);
				entry.size = t.size;
				entry_list.add(entry);
			}
		}
		e=_findnext(handle,&t);
	}
#else // defined(OS_LINUX) || defined(OS_MINGW)
	DIR *_dir;
	_dir=opendir(dir2.c_str());
	if (!_dir){
		return entry_list;
	}
	struct dirent *dn;
	dn=readdir(_dir);
	struct stat s;
	while(dn){
		//if ((strcmp(dn->d_name,".")!=0)and(strcmp(dn->d_name,"..")!=0)and(!strstr(dn->d_name,"~"))){
		string name = dn->d_name;
		if ((name != ".") and (name != "..") and (name.back() != '~')){
			string ffn = dir2 + name;
			stat(ffn.c_str(), &s);
			bool is_reg=(s.st_mode & S_IFREG)>0;
			bool is_dir=(s.st_mode & S_IFDIR)>0;
			int sss=strlen(dn->d_name) - filter2.num;
			if (sss<0)	sss=0;
			if (((is_reg) and (name.match(filter))) or ((show_directories) and (is_dir))){
				entry.name = name;
				entry.is_dir = is_dir;
				entry.size = s.st_size;
				entry_list.add(entry);
			}
		}
		dn=readdir(_dir);
	}
	closedir(_dir);
#endif

	
	// sorting...
	for (int i=0;i<entry_list.num-1;i++)
		for (int j=i+1;j<entry_list.num;j++){
			bool ok = true;
			if (entry_list[i].is_dir == entry_list[j].is_dir){
				if (entry_list[i].name.icompare(entry_list[j].name) > 0)
					ok = false;
			}else
				if ((!entry_list[i].is_dir) and (entry_list[j].is_dir))
					ok = false;
			if (!ok)
				entry_list.swap(i, j);
		}

	return entry_list;
}



