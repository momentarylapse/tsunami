/*----------------------------------------------------------------------------*\
| CFile                                                                        |
| -> acces to files (high & low level)                                         |
| -> text mode / binary mode                                                   |
|    -> textmode: numbers as decimal numbers, 1 line per value saved,          |
|                 carriage-return/linefeed 2 characters (windows),...          |
|    -> binary mode: numbers as 4 byte binary coded, carriage-return 1         |
|                    character,...                                             |
| -> opening a missing file can call a callback function (x: used for          |
|    automatically downloading the file)                                       |
| -> files can be stored in an archive file                                    |
|                                                                              |
| vital properties:                                                            |
|  - a single instance per file                                                |
|                                                                              |
| last update: 2010.06.01 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/
#include "file.h"
#include "date.h"



//#define StructuredShifts
//#define FILE_COMMENTS_DEBUG

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>

#ifdef OS_WINDOWS
	#include <stdio.h>
	#include <io.h>
	#include <direct.h>
	#include <stdarg.h>
	#include <windows.h>
	#include <winbase.h>
	#include <winnt.h>
#endif
#if defined(OS_LINUX) || defined(OS_MAC)
	#include <unistd.h>
	#include <dirent.h>
	#include <stdarg.h>
	#include <sys/stat.h>

	#define _open	::open
	#define _read	::read
	#define _write	::write
	#define _lseek	::lseek
	bool _eof(int handle)
	{
		int pos = lseek(handle,0,SEEK_CUR);
		struct stat stat;
		fstat(handle, &stat);
		return (pos >= stat.st_size);
	}
	#define _close	::close
	#define _rmdir	::rmdir
	#define _unlink	::unlink
#endif

Date time2date(time_t t);


namespace os::fs {


FileStream::FileStream(int h, Mode mode) : Stream(mode) {
	handle = h;
}

FileStream::~FileStream() {
	if (handle >= 0)
		close();
}

bool FileStream::is_end() const {
#ifdef OS_WINDOWS
	return _eof(handle);
#endif
#if defined(OS_LINUX) || defined(OS_MAC) || defined(OS_MINGW)
	int pos = lseek(handle,0,SEEK_CUR);
	struct stat stat;
	fstat(handle, &stat);
	return (pos >= stat.st_size);
#endif
}

void set_mode_bin(int handle) {
#ifdef OS_WINDOWS
	_setmode(handle,_O_BINARY);
#endif
}

void set_mode_text(int handle) {
#ifdef OS_WINDOWS
	_setmode(handle,_O_TEXT);
#endif
}

// open a file stream
FileStream *open(const Path &filename, const string &mode) {
	int handle = -1;

	if (mode.find("r") >= 0) {
		// reading
		handle = _open(filename.c_str(), O_RDONLY);
	} else if (mode.find("w") >= 0) {
#ifdef OS_WINDOWS
		handle = _creat(filename.c_str(), _S_IREAD | _S_IWRITE);
#endif
#if defined(OS_LINUX) || defined(OS_MAC) || defined(OS_MINGW)
		handle = ::creat(filename.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
#endif
	} else if (mode.find("a") >= 0) {
#ifdef OS_WINDOWS
		handle = _open(filename.c_str(), O_WRONLY | O_APPEND | O_CREAT, _S_IREAD | _S_IWRITE);
#endif
#if defined(OS_LINUX) || defined(OS_MAC) || defined(OS_MINGW)
		handle = ::open(filename.c_str(), O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
#endif
	} else {
		throw FileError(format("mode unhandled: '%s'", mode));
	}

	if (handle < 0)
		throw FileError(format("failed opening file '%s'", filename));


	// text mode?
	if (mode.find("t") >= 0)
		set_mode_text(handle);
	else
		set_mode_bin(handle);

	return new FileStream(handle, (mode.find("t") >= 0) ? Stream::Mode::TEXT : Stream::Mode::BINARY);
}



bytes read_binary(const Path &filename) {
	auto *f = open(filename, "rb");
	bytes r = f->read_complete();
	delete f;
	return r;
}

string read_text(const Path &filename) {
	auto *f = open(filename, "rt");
	string r = f->read_complete();
	delete f;
	return r;
}

void write_binary(const Path &filename, const bytes &buf) {
	auto *f = open(filename, "wb");
	f->Stream::write(buf);
	delete f;
}

void write_text(const Path &filename, const string &str) {
	auto f = open(filename, "wt");
	f->write(str);
	delete f;
}


// close the file
void FileStream::close() {
	//flush(handle);
	if (handle >= 0)
		_close(handle);
	handle = -1;
}

// jump to an position in the file or to a position relative to the current
void FileStream::set_pos(int pos) {
	_lseek(handle, pos, SEEK_SET);
}

void FileStream::seek(int delta) {
	_lseek(handle, delta, SEEK_CUR);
}

// retrieve the size of the opened(!) file
int64 FileStream::size() const {
	struct stat s;
	fstat(handle, &s);
	return s.st_size;
}

Date FileStream::ctime() {
	struct stat s;
	fstat(handle, &s);
	return time2date(s.st_ctime);
}

Date FileStream::mtime() {
	struct stat s;
	fstat(handle, &s);
	return time2date(s.st_mtime);
}

Date FileStream::atime() {
	struct stat s;
	fstat(handle, &s);
	return time2date(s.st_atime);
}

// where is the current reading position in the file?
int FileStream::pos() const {
	return _lseek(handle, 0, SEEK_CUR);
}

// read a part of the file into the buffer
int FileStream::read_basic(void *buffer, int size) {
	int r = _read(handle, buffer, size);
	if (r < 0)
		throw FileError(format("failed reading file '%s'", filename));
	return r;
}

// insert the buffer into the file
int FileStream::write_basic(const void *buffer, int size) {
	if (size == 0)
		return 0;
	int r = _write(handle,buffer,size);
	if (r < 0)
		throw FileError(format("failed writing file '%s'", filename));
	return r;
}

}


#if defined(OS_LINUX) || defined(OS_MAC)
	#undef _open
	#undef _read
	#undef _write
	#undef _lseek
	#undef _close
	#undef _rmdir
	#undef _unlink
#endif
