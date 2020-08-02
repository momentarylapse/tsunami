#if !defined(FILE_OP_H__INCLUDED_)
#define FILE_OP_H__INCLUDED_


class Date;
class Path;
#include <functional>


//--------------------------------------------------------------
// file/directory operations

void _cdecl dir_create(const Path &dir);
void _cdecl dir_delete(const Path &dir);
Path _cdecl get_current_dir();
void _cdecl file_rename(const Path &source, const Path &target);
void _cdecl file_copy(const Path &source, const Path &target);
void _cdecl file_delete(const Path &filename);
bool _cdecl file_exists(const Path &filename);
bool _cdecl file_is_directory(const Path &path);
int64 _cdecl file_size(const Path &path);
Date _cdecl file_mtime(const Path &path);
string _cdecl file_hash(const Path &filename, const string &type);

bool func_did_not_throw(std::function<void()> f);
#define FILE_OP_OK(OP) \
	func_did_not_throw([=]{ OP; })



string _cdecl shell_execute(const string &cmd);


Array<string> _cdecl dir_search(const Path &dir, const string &filter, bool show_directories);

#endif
