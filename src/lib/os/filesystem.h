#if !defined(FILESYSTEM_H__INCLUDED_)
#define FILESYSTEM_H__INCLUDED_

#include <lib/base/base.h>
#include <lib/base/error.h>

class Date;
class Path;

namespace os::fs {

Path current_directory();
void set_current_directory(const Path& dir);

bool exists(const Path& path);
bool is_directory(const Path& path);
base::result<int64> size(const Path& path);
base::result<Date> mtime(const Path& path);
base::result<string> hash(const Path& filename, const string& type);

base::result_void create_directory(const Path& dir);
base::result_void remove(const Path& path);
base::result_void move(const Path& source, const Path& target);
base::result_void copy(const Path& source, const Path& target);


Array<Path> search(const Path& dir, const string& filter, const string& options);

}

#endif
