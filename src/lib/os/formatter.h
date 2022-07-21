/*
 * formatter.h
 *
 *  Created on: 1 Jul 2022
 *      Author: michi
 */

#ifndef SRC_LIB_FILE_FORMATTER_H_
#define SRC_LIB_FILE_FORMATTER_H_


#include "stream.h"



class Formatter : public Stream {
public:
	Formatter(shared<Stream> s);

	void set_pos(int pos) override;
	void seek(int delta) override;
	int get_size32() override;
	int64 get_size() override;
	int get_pos() override;

	int read_basic(void *buffer, int size) override;
	int write_basic(const void *buffer, int size) override;
	bool is_end() override;

	shared<Stream> stream;
};

class BinaryFormatter : public Formatter {
public:
	BinaryFormatter(shared<Stream> s);

	unsigned char read_byte();
	char read_char();
	unsigned int read_word();
	unsigned int read_word_reversed(); // for antique versions!!
	int read_int();
	float read_float();
	bool read_bool();
	string read_str();
	string read_str_nt();
	string read_str_rw(); // for antique versions!!
	void read_vector(void *v);

	void write_byte(unsigned char);
	void write_char(char);
	void write_bool(bool);
	void write_word(unsigned int);
	void write_int(int);
	void write_float(float);
	void write_str(const string&);
	void write_vector(const void *v);

	void read_comment();
	void write_comment(const string &str);

	//int read_file_format_version();
	//void write_file_format_version(int v);
};

class TextLinesFormatter : public Formatter {
public:
	TextLinesFormatter(shared<Stream> s);

	unsigned char read_byte();
	char read_char();
	unsigned int read_word();
	unsigned int read_word_reversed(); // for antique versions!!
	int read_int();
	float read_float();
	bool read_bool();
	string read_str();
	string read_str_nt();
	string read_str_rw(); // for antique versions!!
	void read_vector(void *v);

	void write_byte(unsigned char);
	void write_char(char);
	void write_bool(bool);
	void write_word(unsigned int);
	void write_int(int);
	void write_float(float);
	void write_str(const string&);
	void write_vector(const void *v);

	void read_comment();
	void write_comment(const string &str);

	//int read_file_format_version();
	//void write_file_format_version(int v);

	int decimals = 6;
};


#endif /* SRC_LIB_FILE_FORMATTER_H_ */
