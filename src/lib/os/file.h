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
| last update: 2010.07.01 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/
#ifndef SRC_LIB_OS_FILE_H_
#define SRC_LIB_OS_FILE_H_

#include "../base/base.h"
#include "../base/error.h"
#include "stream.h"
#include "path.h"


class Date;

namespace os::fs {


class FileError : public Exception {
public:
	explicit FileError(const string &msg) : Exception(msg) {}
};


//--------------------------------------------------------------
// file operation class

class FileStream : public Stream {
public:
	FileStream(int handle, Mode mode);
	~FileStream() override;

	// meta
	void set_pos(int pos) override;
	void seek(int delta) override;
	int64 size() const override;
	int pos() const override;
	Date ctime();
	Date mtime();
	Date atime();

	bool is_end() const override;
	void close();

	int read_basic(void *buffer, int size) override;
	int write_basic(const void *buffer, int size) override;

//private:
	Path filename;
	int handle = -1;
};

FileStream *open(const Path &filename, const string &mode);

bytes read_binary(const Path &filename);
string read_text(const Path &filename);
base::result<bytes> read_binary2(const Path &filename);
base::result<string> read_text2(const Path &filename);
void write_binary(const Path &filename, const bytes &data);
void write_text(const Path &filename, const string &str);

}



#endif


