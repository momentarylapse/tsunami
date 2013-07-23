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




#include <string.h>
#include <stdlib.h>
	

#include "../base/base.h"
#include "msg.h"
#include "file_op.h"



//--------------------------------------------------------------
// time/date

struct Date
{
	int time;
	int year, month, day, hour, minute, second, milli_second;
	int day_of_week, day_of_year;
	string _cdecl format(const string &f) const;
	string _cdecl str() const;
};

Date _cdecl get_current_date();




//--------------------------------------------------------------
// file operation class

typedef bool t_file_try_again_func(const string &filename);

extern bool SilentFiles;
extern t_file_try_again_func *FileTryAgainFunc;

void file_set_archive(const string &filename);
void file_clean_up_archive();

class CFile
{
public:
	CFile();
	~CFile();

	// opening
	bool _cdecl Open(const string &filename);
	bool _cdecl Create(const string &filename);
	bool _cdecl Append(const string &filename);
	bool _cdecl Close();

	// meta
	void _cdecl SetBinaryMode(bool bm);
	void _cdecl SetPos(int pos,bool absolute);
	int _cdecl GetSize();
	int _cdecl GetPos();
	Date _cdecl GetDateCreation();
	Date _cdecl GetDateModification();
	Date _cdecl GetDateAccess();

	// file format version
	int _cdecl ReadFileFormatVersion();
	void _cdecl WriteFileFormatVersion(bool binary, int fvv);

	// really low level
	int _cdecl ReadBuffer(void *buffer, int size);
	int _cdecl WriteBuffer(const void *buffer, int size);
	string _cdecl ReadComplete();

	// medium level
	void _cdecl ReadComment();
	char _cdecl ReadChar();
	unsigned char _cdecl ReadByte();
	unsigned char _cdecl ReadByteC();
	unsigned short _cdecl ReadWord();
	unsigned short _cdecl ReadWordC();
	unsigned short _cdecl ReadReversedWord(); // for antique versions!!
	int _cdecl ReadInt();
	int _cdecl ReadIntC();
	float _cdecl ReadFloat();
	float _cdecl ReadFloatC();
	bool _cdecl ReadBool();
	bool _cdecl ReadBoolC();
	string _cdecl ReadStr();
	string _cdecl ReadStrNT();
	string _cdecl ReadStrC();
	string _cdecl ReadStrRW(); // for antique versions!!
	void _cdecl ReadVector(void *v);

	void _cdecl WriteChar(char c);
	void _cdecl WriteByte(unsigned char c);
	void _cdecl WriteWord(unsigned short);
	void _cdecl WriteInt(int in);
	void _cdecl WriteFloat(float f);
	void _cdecl WriteBool(bool b);
	void _cdecl WriteStr(const string &str);
	void _cdecl WriteComment(const string &str);
	void _cdecl WriteVector(const void *v);

	// high level
	void _cdecl Int(int &i);
	void _cdecl Float(float &f);
	void _cdecl Bool(bool &b);
	void _cdecl String(string &str);
	void _cdecl Vector(float *v);
	void _cdecl Struct(const char *format, void *data);
	void _cdecl StructN(const char *format, int &num, void *data, int shift);

	void _cdecl ShiftRight(int s);

	bool Eof, Binary, SilentFileAccess, Reading;
	int FloatDecimals;

//private:
	int handle;
	bool Error,ErrorReported,DontReportErrors;
};

extern CFile *_cdecl FileOpen(const string &filename);
extern CFile *_cdecl FileOpenSilent(const string &filename);
extern CFile *_cdecl FileCreate(const string &filename);
extern CFile *_cdecl FileCreateSilent(const string &filename);
extern CFile *_cdecl FileAppend(const string &filename);
extern void _cdecl FileClose(CFile *f);
extern string _cdecl FileRead(const string &filename);
extern void _cdecl FileWrite(const string &filename, const string &str);




#endif


