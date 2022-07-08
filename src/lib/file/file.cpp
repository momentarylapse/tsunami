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



//#define StructuredShifts
//#define FILE_COMMENTS_DEBUG

#include <chrono>
#include <ctime>

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
#ifdef OS_LINUX
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


Date time2date(time_t t) {
	Date d;
	d.time = (int64)t;
	tm *tm = localtime(&t);
	d.milli_second = 0;
	return d;
}

Date Date::now() {
	auto now = std::chrono::system_clock::now();
	auto t = std::chrono::system_clock::to_time_t(now);
	auto d = time2date(t);

	auto dtn = now.time_since_epoch();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dtn);
	d.milli_second = ms.count() % 1000;
	return d;
}

string Date::format(const string &f) const {
	char buffer [80];
	time_t rawtime = (time_t)this->time;
	tm * timeinfo = localtime (&rawtime);
	strftime(buffer, sizeof(buffer), f.c_str(), timeinfo);
	return buffer;
}

string Date::str() const {
	return this->format("%c");
}

void Date::__assign__(const Date &d) {
	*this = d;
}


FileStream::FileStream(int h) {
	handle = h;
}

FileStream::~FileStream() {
	if (handle >= 0)
		close();
}

bool FileStream::is_end() {
#ifdef OS_WINDOWS
	return _eof(handle);
#endif
#if defined(OS_LINUX) or defined(OS_MINGW)
	int pos = lseek(handle,0,SEEK_CUR);
	struct stat stat;
	fstat(handle, &stat);
	return (pos >= stat.st_size);
#endif
}

void set_mode_bin(int handle) {
#ifdef OS_WINDOWS
	_setmode(f->handle,_O_BINARY);
#endif
}

void set_mode_text(int handle) {
#ifdef OS_WINDOWS
	_setmode(f->handle,_O_TEXT);
#endif
}

// open a file stream
FileStream *file_open(const Path &filename, const string &mode) {
	int handle = -1;

	if (mode.find("r") >= 0) {
		// reading
		handle = _open(filename.c_str(), O_RDONLY);
	} else if (mode.find("w") >= 0) {
#ifdef OS_WINDOWS
		handle = _creat(filename.c_str(), _S_IREAD | _S_IWRITE);
#endif
#if defined(OS_LINUX) or defined(OS_MINGW)
		handle = ::creat(filename.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
#endif
	} else if (mode.find("a") >= 0) {
#ifdef OS_WINDOWS
		handle = _open(filename.c_str(), O_WRONLY | O_APPEND | O_CREAT, _S_IREAD | _S_IWRITE);
#endif
#if defined(OS_LINUX) or defined(OS_MINGW)
		handle = ::open(filename.c_str(), O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
#endif
	} else {
		throw FileError(format("mode unhandled: '%s'", mode));
	}

	if (handle <= 0)
		throw FileError(format("failed opening file '%s'", filename));


	// text mode?
	if (mode.find("t") >= 0)
		set_mode_text(handle);
	else
		set_mode_bin(handle);

	return new FileStream(handle);
}

bytes file_read_binary(const Path &filename) {
	auto *f = file_open(filename, "rb");
	bytes r = f->read_complete();
	delete f;
	return r;
}

string file_read_text(const Path &filename) {
	auto *f = file_open(filename, "rt");
	string r = f->read_complete();
	delete f;
	return r;
}

void file_write_binary(const Path &filename, const bytes &buf) {
	auto *f = file_open(filename, "wb");
	f->Stream::write(buf);
	delete f;
}

void file_write_text(const Path &filename, const string &str) {
	auto f = file_open(filename, "wt");
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
int FileStream::get_size32() {
	struct stat _stat;
	fstat(handle, &_stat);
	return _stat.st_size;
}

// retrieve the size of the opened(!) file
int64 FileStream::get_size() {
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
int FileStream::get_pos() {
	return _lseek(handle, 0, SEEK_CUR);
}

// read the complete file into the buffer
bytes Stream::read_complete() {
	static const int CHUNK_SIZE = 2048;
	bytes buf, chunk;
	chunk.resize(CHUNK_SIZE);
	int r = 0;
	while (true) {
		int r = read(chunk);
		if (r <= 0)
			return buf;
		buf += chunk.sub_ref(0, r);
	};
	return buf;
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

int Stream::read(void *data, int size) {
	return read_basic(data, size);
}

int Stream::write(const void *data, int size) {
	return write_basic(data, size);
}

int Stream::read(bytes &data) {
	return read_basic(data.data, data.num);
}

bytes Stream::read(int size) {
	bytes data;
	data.resize(size);
	int r = read(data);
	data.resize(max(r, 0));
	return data;
}

int Stream::write(const bytes &data) {
	return write_basic(data.data, data.num);
}
