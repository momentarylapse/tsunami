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
	string format(const string &f) const;
	string str() const;
};

Date get_current_date();




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
	bool Open(const string &filename);
	bool Create(const string &filename);
	bool Append(const string &filename);
	bool _cdecl Close();

	// meta
	void SetBinaryMode(bool bm);
	void SetPos(int pos,bool absolute);
	int GetSize();
	int GetPos();
	Date _cdecl GetDateCreation();
	Date _cdecl GetDateModification();
	Date _cdecl GetDateAccess();

	// file format version
	int ReadFileFormatVersion();
	void WriteFileFormatVersion(bool binary, int fvv);

	// really low level
	int ReadBuffer(void *buffer, int size);
	int WriteBuffer(const void *buffer, int size);
	string ReadComplete();

	// medium level
	void ReadComment();
	char ReadChar();
	unsigned char ReadByte();
	unsigned char ReadByteC();
	unsigned short ReadWord();
	unsigned short ReadWordC();
	unsigned short ReadReversedWord(); // for antique versions!!
	int _cdecl ReadInt();
	int _cdecl ReadIntC();
	float ReadFloat();
	float ReadFloatC();
	bool _cdecl ReadBool();
	bool _cdecl ReadBoolC();
	string _cdecl ReadStr();
	string ReadStrNT();
	string _cdecl ReadStrC();
	string ReadStrRW(); // for antique versions!!
	void ReadVector(void *v);

	void WriteChar(char c);
	void WriteByte(unsigned char c);
	void WriteWord(unsigned short);
	void _cdecl WriteInt(int in);
	void WriteFloat(float f);
	void _cdecl WriteBool(bool b);
	void WriteStr(const string &str);
	void WriteComment(const string &str);
	void WriteVector(const void *v);

	// high level
	void Int(int &i);
	void Float(float &f);
	void Bool(bool &b);
	void String(string &str);
	void Vector(float *v);
	void Struct(const char *format, void *data);
	void StructN(const char *format, int &num, void *data, int shift);

	void ShiftRight(int s);

	bool Eof, Binary, SilentFileAccess, Reading;
	int FloatDecimals;

//private:
	int handle;
	bool Error,ErrorReported,DontReportErrors;
};

extern CFile *OpenFile(const string &filename);
extern CFile *OpenFileSilent(const string &filename);
extern CFile *CreateFile(const string &filename);
extern CFile *CreateFileSilent(const string &filename);
extern CFile *AppendFile(const string &filename);
extern void FileClose(CFile *f);
extern string FileRead(const string &filename);
extern void FileWrite(const string &filename, const string &str);




#endif


