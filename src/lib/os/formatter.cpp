/*
 * formatter.cpp
 *
 *  Created on: 1 Jul 2022
 *      Author: michi
 */

#include "file.h"
#include "formatter.h"
#include "msg.h"



static void read_buffer_asserted(Stream *s, void *buf, int size) {
	if (auto f = dynamic_cast<os::fs::FileStream*>(s)) {
		int r = f->read_basic(buf, size);
		if (r < size)
			throw os::fs::FileError(format("end of file '%s'", f->filename));
	}
}

Formatter::Formatter(shared<Stream> s) {
	stream = s;
}

BinaryFormatter::BinaryFormatter(shared<Stream> s) : Formatter(s) {
}

TextLinesFormatter::TextLinesFormatter(shared<Stream> s) : Formatter(s) {}

// read a single character (1 byte)
char BinaryFormatter::read_char() {
	char c;
	read_buffer_asserted(stream.get(), &c, 1);
	return c;
}
char TextLinesFormatter::read_char() {
	char c = 0x0d;
	while (c == 0x0d)
		read_buffer_asserted(stream.get(), &c, 1);
	return c;
}

unsigned char TextLinesFormatter::read_byte() {
	return read_str()._int();
}

// read a single character (1 byte)
unsigned char BinaryFormatter::read_byte() {
	return read_char();
}

// read the rest of the line (only text mode)
void BinaryFormatter::read_comment() {
}

void TextLinesFormatter::read_comment() {
#ifdef FILE_COMMENTS_DEBUG
	msg_write("comment: " + read_str());
#else
	read_str();
#endif

}

// read a word (2 bytes in binary mode)
unsigned int BinaryFormatter::read_word() {
	unsigned int i = 0;
	read_buffer_asserted(stream.get(), &i, 2);
	return i;
}

unsigned int TextLinesFormatter::read_word() {
	return read_str()._int();
}

// read a word (2 bytes in binary mode)
unsigned int BinaryFormatter::read_word_reversed() {
	unsigned int a = read_byte();
	unsigned int b = read_byte();
	return (a << 8) + b;
}

// read an integer (4 bytes in binary mode)
int BinaryFormatter::read_int() {
	int i;
	read_buffer_asserted(stream.get(), &i, 4);
	return i;
}

int TextLinesFormatter::read_int() {
	return read_str()._int();
}

// read a float (4 bytes in binary mode)
float BinaryFormatter::read_float() {
	float f;
	read_buffer_asserted(stream.get(), &f, 4);
	return f;
}

float TextLinesFormatter::read_float() {
	return read_str()._float();
}

// read a boolean (1 byte in binary mode)
bool BinaryFormatter::read_bool() {
	char bb = 0;
	read_buffer_asserted(stream.get(), &bb, 1);
	return (bb == '1') or (bb == 0x01); // sigh, old style booleans
}

bool TextLinesFormatter::read_bool() {
	string s = read_str();
	return (s == "1");
}

// read a string
//   text mode:   complete rest of this line
//   binary mode: length word, then string
string BinaryFormatter::read_str() {
	// binary: read length as a word then read so many bytes
	int l = read_word();
	if (l >= 0xc000) {
		l = (read_word() << 14) + (l & 0x3fff);
	}

	string str;
	str.resize(l + 16); // prevents "uninitialized" bytes in syscall parameter... (valgrind)
	read_buffer_asserted(stream.get(), str.data, l);
	str.resize(l);
	return str;
}

string TextLinesFormatter::read_str() {
	// read one byte at a time until we reach a \n character
	string str;
	while(true) {
		char c = read_char();

		#if defined(OS_LINUX) || defined(OS_MAC)
			// windows read-function does this on its own... (O_O)
			if (c == '\r')
				continue;
		#endif

		if (c == '\n')
			break;
		str.add(c);
	}
	return str;
}

// read a null-terminated string
string BinaryFormatter::read_str_nt() {
	string str;
	while (true) {
		char c = read_char();
		if (c == 0)
			break;
		str.add(c);
	}
	return str;
}

// read a string having reversed byte as length in binary mode
string BinaryFormatter::read_str_rw() {
	int l = read_word_reversed();
	string str;
	str.resize(l);
	read_buffer_asserted(stream.get(), str.data, l);
	return str;
}

void BinaryFormatter::read_vector(void *v) {
	((float*)v)[0] = read_float();
	((float*)v)[1] = read_float();
	((float*)v)[2] = read_float();
}

void TextLinesFormatter::read_vector(void *v) {
	((float*)v)[0] = read_float();
	((float*)v)[1] = read_float();
	((float*)v)[2] = read_float();
}

// write a single character (1 byte)
void BinaryFormatter::write_char(char c) {
	stream->write_basic(&c, 1);
}

void TextLinesFormatter::write_char(char c) {
	write_str(string(&c, 1));
	//write_str(i2s(c));
}

// write a single character (1 byte)
void BinaryFormatter::write_byte(unsigned char c) {
	stream->write_basic(&c, 1);
}

void TextLinesFormatter::write_byte(unsigned char c) {
	write_str(i2s(c));
}

// write a word (2 bytes)
void BinaryFormatter::write_word(unsigned int w) {
	stream->write_basic(&w, 2);
}

void TextLinesFormatter::write_word(unsigned int w) {
	write_str(i2s(w));
}

// write an integer (4 bytes)
void BinaryFormatter::write_int(int i) {
	stream->write_basic(&i, 4);
}

void TextLinesFormatter::write_int(int i) {
	write_str(i2s(i));
}

// write a float (4 bytes)
void BinaryFormatter::write_float(float f) {
	stream->write_basic(&f, 4);
}

void TextLinesFormatter::write_float(float f) {
	write_str(f2s(f, decimals));
}

// write a boolean (1 byte)
void BinaryFormatter::write_bool(bool b) {
	stream->write_basic(&b, 1);
}

void TextLinesFormatter::write_bool(bool b) {
	write_str(b ? "1" : "0");
}

// write a string
//   text mode:   complete rest of this line
//   binary mode: length word, then string
void BinaryFormatter::write_str(const string &str) {
	int num = min(str.num, 0x1fffffff); // ~ 1gb...
	if (num >= 0xc000) {
		write_word((num & 0x3fff) | 0xc000);
		write_word(num >> 14);
		stream->write_basic(str.data, num);
	} else {
		write_word(num);
		stream->write_basic(str.data, num);
	}
}
void TextLinesFormatter::write_str(const string &str) {
	stream->write(str + "\n");
}

// write a comment line
void BinaryFormatter::write_comment(const string &str) {
}

void TextLinesFormatter::write_comment(const string &str) {
#ifdef FILE_COMMENTS_DEBUG
	msg_write("comment: " + str);
#endif
	write_str(str);
}

void BinaryFormatter::write_vector(const void *v) {
	write_float(((float*)v)[0]);
	write_float(((float*)v)[1]);
	write_float(((float*)v)[2]);
}

void TextLinesFormatter::write_vector(const void *v) {
	write_float(((float*)v)[0]);
	write_float(((float*)v)[1]);
	write_float(((float*)v)[2]);
}

void Formatter::set_pos(int pos) {
	stream->set_pos(pos);
}

void Formatter::seek(int delta) {
	stream->seek(delta);
}

int Formatter::get_size32() {
	return stream->get_size32();
}

int64 Formatter::get_size() {
	return stream->get_size();
}

int Formatter::get_pos() {
	return stream->get_pos();
}

int Formatter::read_basic(void *buffer, int size) {
	return stream->read_basic(buffer, size);
}

int Formatter::write_basic(const void *buffer, int size) {
	return stream->write_basic(buffer, size);
}

bool Formatter::is_end() {
	return stream->is_end();
}
// read a single character followed by the file-format-version-number
/*int File::ReadFileFormatVersion()
{
	unsigned char a=0;
	if (_read(handle,&a,1)<=0){
		return -1;
	}
	if (a=='t'){
		//SetBinaryMode(false);
	}else if (a=='b'){
		//SetBinaryMode(true);
	}else{
		throw FileError("File Format Version must begin ether with 't' or 'b'!!!");
	}
	return read_word();
}

// write a single character followed by the file-format-version-number
void File::WriteFileFormatVersion(bool binary,int fvv)
{
	char a = binary ? 'b' : 't';
	int r = _write(handle, &a, 1);
	//SetBinaryMode(binary);
	write_word(fvv);
}*/
