#include "filesystem.h"
#include "../base/sort.h"
#include "date.h"
#include "file.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cerrno>
#include <filesystem>

namespace os::fs {


// just test the existence of a file
bool exists(const Path& path) {
	std::error_code e;
	return std::filesystem::exists(path.c_str(), e);
}

base::result<int64> size(const Path& path) {
	std::error_code e;
	const auto size = std::filesystem::file_size(path.c_str(), e);
	if (e)
		return base::Error(format("file size '%s': %s", path, e.message().c_str()));
	return (int64)size;
}

base::result<Date> mtime(const Path &path) {
	/* fails, because the "file clock" has an undefined epoch... :(
	std::error_code e;
	const auto ftime = std::filesystem::last_write_time(path.c_str(), e);
	if (e)
		return base::Error(format("file size '%s': %s", path, e.message().c_str()));
	return Date::from_unix(ftime.time_since_epoch().count());*/

	struct stat s;
	if (stat(path.str().c_str(), &s) != 0)
		return base::Error(format("mtime '%s': failed", path));
	return Date::from_unix(s.st_mtime);
}

bool is_directory(const Path& path) {
	std::error_code e;
	return std::filesystem::is_directory(path.c_str(), e);
}


base::result_void create_directory(const Path& dir) {
	if (dir.is_empty())
		return base::Error("create directory: empty path");
	std::error_code e;
	std::filesystem::create_directories(dir.c_str(), e);
	if (e)
		return base::Error(format("create directory '%s': %s", dir, e.message().c_str()));
	return base::result_success();
}

base::result_void remove(const Path& path) {
	std::error_code e;
	std::filesystem::remove_all(path.c_str(), e);
	if (e)
		return base::Error(format("remove '%s': %s", path, e.message().c_str()));
	return base::result_success();
}

Path current_directory() {
	std::error_code e;
	return std::filesystem::current_path(e).c_str();
}


void set_current_directory(const Path &dir) {
	std::error_code e;
	std::filesystem::current_path(dir.c_str(), e);
}

base::result_void move(const Path& source, const Path& target) {
	if (!target.parent().is_empty())
		RESULT_PROPAGATE_ERROR_VOID(create_directory(target.parent()));

	// linux automatically overwrites, windows will fail rename()
	if (exists(target))
		RESULT_PROPAGATE_ERROR_VOID(remove(target));

	std::error_code e;
	std::filesystem::rename(source.c_str(), target.c_str(), e);
	if (e)
		return base::Error(format("move '%s' -> '%s': %s", source, target, e.message().c_str()));
	return base::result_success();
}

base::result_void copy(const Path& source, const Path& target) {
	if (!target.parent().is_empty())
		RESULT_PROPAGATE_ERROR_VOID(create_directory(target.parent()));

	// linux automatically overwrites, windows will fail rename()
	if (exists(target))
		RESULT_PROPAGATE_ERROR_VOID(remove(target));

	std::error_code e;
	std::filesystem::copy(source.c_str(), target.c_str(), e);
	if (e)
		return base::Error(format("copy '%s' -> '%s': %s", source, target, e.message().c_str()));
	return base::result_success();
}

base::result<string> hash(const Path &filename, const string &type) {
	if (type == "md5") {
		try {
			return read_binary(filename).md5();
		} catch (const Exception& e) {
			return base::Error(e.message());
		}
	}
	return base::Error("file hash: only supporting 'md5'");
}



// search a directory for files matching a filter
void search_single(const Path& dir, const string& filter, Array<Path>& dir_list, Array<Path>& file_list) {
	std::error_code e;
	for (const auto& entry: std::filesystem::directory_iterator(dir.c_str(), e)) {
		const string name = entry.path().filename().c_str();
		if (entry.is_directory()) {
			dir_list.add(name);
		} else if (name.match(filter)) {
			file_list.add(name);
		}
	}

	// sorting...
	base::inplace_sort(dir_list, [] (const Path& a, const Path& b) { return a <= b; });
	base::inplace_sort(file_list, [] (const Path& a, const Path& b) { return a <= b; });
}

void search_single_rec(const Path& dir0, const Path& subdir, const string& filter, Array<Path>& dir_list, Array<Path>& file_list) {
	Array<Path> sub_dir_list, sub_file_list;
	search_single(dir0 | subdir, filter, sub_dir_list, sub_file_list);
	for (const auto &x: sub_dir_list) {
		dir_list.add(subdir | x);
		search_single_rec(dir0, subdir | x, filter, dir_list, file_list);
	}
	for (const auto &x: sub_file_list)
		file_list.add(subdir | x);
}

// search a directory for files matching a filter
Array<Path> search(const Path& dir, const string& filter, const string& options) {
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

