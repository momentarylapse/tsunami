/*
 * formatter.h
 *
 *  Created on: 1 Jul 2022
 *      Author: michi
 */

#ifndef SRC_LIB_FILE_FORMATTER_H_
#define SRC_LIB_FILE_FORMATTER_H_


#include "stream.h"



class Formatter : Sharable<VirtualBase> {
public:
	Formatter(Stream* s);
	~Formatter();

	virtual unsigned char read_byte() = 0;
	virtual char read_char() = 0;
	virtual unsigned int read_word() = 0;
	virtual unsigned int read_word_reversed() { return read_word(); }; // for antique versions!!
	virtual int read_int() = 0;
	virtual float read_float() = 0;
	virtual bool read_bool() = 0;
	virtual string read_str() = 0;
	virtual void read_vector(void *v) = 0;

	virtual void write_byte(unsigned char) = 0;
	virtual void write_char(char) = 0;
	virtual void write_bool(bool) = 0;
	virtual void write_word(unsigned int) = 0;
	virtual void write_int(int) = 0;
	virtual void write_float(float) = 0;
	virtual void write_str(const string&) = 0;
	virtual void write_vector(const void *v) = 0;

	virtual void read_comment() {}
	virtual void write_comment(const string &str) {}

	Stream* stream;
};

class BinaryFormatter : public Formatter {
public:
	BinaryFormatter(Stream* s);

	unsigned char read_byte() override;
	char read_char() override;
	unsigned int read_word() override;
	unsigned int read_word_reversed() override; // for antique versions!!
	int read_int() override;
	float read_float() override;
	bool read_bool() override;
	string read_str() override;
	void read_vector(void *v) override;

	void write_byte(unsigned char) override;
	void write_char(char) override;
	void write_bool(bool) override;
	void write_word(unsigned int) override;
	void write_int(int) override;
	void write_float(float) override;
	void write_str(const string&) override;
	void write_vector(const void *v) override;

	void read_comment() override;
	void write_comment(const string &str) override;

	//int read_file_format_version();
	//void write_file_format_version(int v);
};

class TextLinesFormatter : public Formatter {
public:
	TextLinesFormatter(Stream* s);

	unsigned char read_byte() override;
	char read_char() override;
	unsigned int read_word() override;
	int read_int() override;
	float read_float() override;
	bool read_bool() override;
	string read_str() override;
	void read_vector(void *v) override;

	void write_byte(unsigned char) override;
	void write_char(char) override;
	void write_bool(bool) override;
	void write_word(unsigned int) override;
	void write_int(int) override;
	void write_float(float) override;
	void write_str(const string&) override;
	void write_vector(const void *v) override;

	void read_comment() override;
	void write_comment(const string &str) override;

	//int read_file_format_version();
	//void write_file_format_version(int v);

	int decimals = 6;
};


#endif /* SRC_LIB_FILE_FORMATTER_H_ */
