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



//--------------------------------------------------------------
// time/date

struct Date {
	int time;
	int year, month, day, hour, minute, second, milli_second;
	int day_of_week, day_of_year;
	string _cdecl format(const string &f) const;
	string _cdecl str() const;
};

Date _cdecl get_current_date();



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
	virtual void open(const string &filename);
	virtual void create(const string &filename);
	virtual void append(const string &filename);
	void close();

	bool _cdecl eof();

	// meta
	void _cdecl set_pos(int pos);
	void _cdecl seek(int delta);
	int _cdecl get_size();
	long long _cdecl get_size64();
	int _cdecl get_pos();
	Date _cdecl GetDateCreation();
	Date _cdecl GetDateModification();
	Date _cdecl GetDateAccess();

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
	string filename;
	int handle;
};

extern File *FileOpen(const string &filename);
extern File *FileOpenText(const string &filename);
extern File *FileCreate(const string &filename);
extern File *FileCreateText(const string &filename);
extern File *FileAppend(const string &filename);
extern void FileClose(File *f);
extern string FileRead(const string &filename);
extern string FileReadText(const string &filename);
extern void FileWrite(const string &filename, const string &str);
extern void FileWriteText(const string &filename, const string &str);




#endif


