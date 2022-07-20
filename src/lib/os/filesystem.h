#if !defined(FILESYSTEM_H__INCLUDED_)
#define FILESYSTEM_H__INCLUDED_

#include "../base/base.h"

class Date;
class Path;
#include <functional>


//--------------------------------------------------------------
// file/directory operations

void dir_create(const Path &dir);
void dir_delete(const Path &dir);
Path get_current_dir();
void file_rename(const Path &source, const Path &target);
void file_copy(const Path &source, const Path &target);
void file_delete(const Path &filename);
bool file_exists(const Path &filename);
bool file_is_directory(const Path &path);
int64 file_size(const Path &path);
Date file_mtime(const Path &path);
string file_hash(const Path &filename, const string &type);

bool func_did_not_throw(std::function<void()> f);
#define FILE_OP_OK(OP) \
	func_did_not_throw([=]{ OP; })



string shell_execute(const string &cmd);


Array<Path> dir_search(const Path &dir, const string &filter, const string &options);

#endif
