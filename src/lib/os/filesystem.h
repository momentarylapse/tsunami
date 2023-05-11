#if !defined(FILESYSTEM_H__INCLUDED_)
#define FILESYSTEM_H__INCLUDED_

#include "../base/base.h"

class Date;
class Path;
#include <functional>

namespace os::fs {

void create_directory(const Path &dir);
void delete_directory(const Path &dir);
Path current_directory();
void set_current_directory(const Path &dir);
void rename(const Path &source, const Path &target);
void move(const Path &source, const Path &target);
void copy(const Path &source, const Path &target);
void _delete(const Path &filename);
bool exists(const Path &filename);
bool is_directory(const Path &path);
int64 size(const Path &path);
Date mtime(const Path &path);
string hash(const Path &filename, const string &type);

bool func_did_not_throw(std::function<void()> f);
#define FILE_OP_OK(OP) \
	os::fs::func_did_not_throw([=]{ OP; })



Array<Path> search(const Path &dir, const string &filter, const string &options);

}

#endif
