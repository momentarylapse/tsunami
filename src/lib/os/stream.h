/*
 * stream.h
 *
 *  Created on: 21 Jul 2022
 *      Author: michi
 */

#ifndef SRC_LIB_OS_STREAM_H_
#define SRC_LIB_OS_STREAM_H_


#include "../base/base.h"
#include "../base/pointer.h"

class Formatter;


class Stream : public Sharable<base::Empty> {
public:
	enum class Mode {
		NONE,
		BINARY,
		TEXT
	};
	Stream(Mode mode);
	virtual ~Stream();

	// meta
	virtual void set_pos(int pos) = 0;
	virtual void seek(int delta) = 0;
	virtual int64 size() const = 0;
	int size32() const { return (int)size(); };
	virtual int pos() const = 0;

	virtual int read_basic(void *buffer, int size) = 0;
	virtual int write_basic(const void *buffer, int size) = 0;
	int read(void *buffer, int size);
	int write(const void *buffer, int size);
	int read(bytes&);
	bytes read(int size);
	int write(const bytes&);

	virtual bool is_end() const = 0;

	bytes read_complete();



	unsigned char read_byte();
	char read_char();
	unsigned int read_word();
	unsigned int read_word_reversed(); // for antique versions!!
	int read_int();
	float read_float();
	bool read_bool();
	string read_str();
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

	owned<Formatter> formatter;
};



#endif /* SRC_LIB_OS_STREAM_H_ */
