#if !defined(FILE_OP_H__INCLUDED_)
#define FILE_OP_H__INCLUDED_


class Date;
#include <functional>


//--------------------------------------------------------------
// file/directory operations

void _cdecl dir_create(const string &dir);
void _cdecl dir_delete(const string &dir);
string _cdecl get_current_dir();
void _cdecl file_rename(const string &source,const string &target);
void _cdecl file_copy(const string &source,const string &target);
void _cdecl file_delete(const string &filename);
bool _cdecl file_exists(const string &filename);
bool _cdecl file_is_directory(const string &path);
int64 _cdecl file_size(const string &path);
Date _cdecl file_mtime(const string &path);
string _cdecl file_hash(const string &filename, const string &type);

string _cdecl path_absolute(const string &path);
string _cdecl path_canonical(const string &path);
string _cdecl dir_canonical(const string &path);

string _cdecl sys_filename(const string &path);
string _cdecl path_dirname(const string &path);
string _cdecl path_basename(const string &path);
string _cdecl path_extension(const string &path);

bool func_did_not_throw(std::function<void()> f);
#define FILE_OP_OK(OP) \
	func_did_not_throw([=]{ OP; })



string _cdecl shell_execute(const string &cmd);


Array<string> _cdecl dir_search(const string &dir,const string &filter,bool show_directories);

#endif
