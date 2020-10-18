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
#if !defined(FILE_H)
#define FILE_H



// ANSI:
//#include <stdarg.h>
// UNIX:
//#include <varargs.h>


#include "../base/base.h"


#include <string.h>
#include <stdlib.h>
	

#include "../base/base.h"
#include "msg.h"
#include "file_op.h"
#include "path.h"



//--------------------------------------------------------------
// time/date

class Date {
public:
	int64 time;
	int milli_second;
	int dummy[7];
	string _cdecl format(const string &f) const;
	string _cdecl str() const;
	void __assign__(const Date &d);

	static Date _cdecl now();
};



class FileError : public Exception {
public:
	explicit FileError(const string &msg):Exception(msg){}
};


//--------------------------------------------------------------
// file operation class

typedef bool t_file_try_again_func(const string &filename);

extern t_file_try_again_func *FileTryAgainFunc;

class File {
public:
	File();
	virtual ~File();

	// opening
	virtual void open(const Path &filename);
	virtual void create(const Path &filename);
	virtual void append(const Path &filename);
	void close();

	bool _cdecl end();

	// meta
	void _cdecl set_pos(int pos);
	void _cdecl seek(int delta);
	int _cdecl get_size32();
	int64 _cdecl get_size();
	int _cdecl get_pos();
	Date _cdecl ctime();
	Date _cdecl mtime();
	Date _cdecl atime();

	// file format version
	int ReadFileFormatVersion();
	void WriteFileFormatVersion(bool binary, int fvv);

	// really low level
	int _cdecl read_buffer(void *buffer, int size);
	int _cdecl write_buffer(const void *buffer, int size);
	int _cdecl read_buffer(string &str);
	int _cdecl write_buffer(const string &str);
	string read_complete();

	// medium level
	virtual char read_char();
	virtual unsigned char read_byte();
	virtual unsigned int read_word();
	virtual unsigned int read_word_reversed(); // for antique versions!!
	virtual int read_int();
	virtual float read_float();
	virtual bool read_bool();
	virtual string read_str();
	virtual string read_str_nt();
	virtual string read_str_rw(); // for antique versions!!
	virtual void read_vector(void *v);

	virtual void write_char(char c);
	virtual void write_byte(unsigned char c);
	virtual void write_word(unsigned int);
	virtual void write_int(int);
	virtual void write_float(float f);
	virtual void write_bool(bool b);
	virtual void write_str(const string &str);
	virtual void write_vector(const void *v);

	virtual void read_comment();
	virtual void write_comment(const string &str);

	int float_decimals;

//private:
	Path filename;
	int handle;
};

extern File *FileOpen(const Path &filename);
extern File *FileOpenText(const Path &filename);
extern File *FileCreate(const Path &filename);
extern File *FileCreateText(const Path &filename);
extern File *FileAppend(const Path &filename);
extern void FileClose(File *f);
extern string FileRead(const Path &filename);
extern string FileReadText(const Path &filename);
extern void FileWrite(const Path &filename, const string &str);
extern void FileWriteText(const Path &filename, const string &str);




#endif


