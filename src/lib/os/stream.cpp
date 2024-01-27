/*
 * stream.cpp
 *
 *  Created on: 21 Jul 2022
 *      Author: michi
 */

#include "stream.h"
#include "formatter.h"

Stream::Stream(Stream::Mode mode) {
	if (mode == Mode::TEXT)
		formatter = new TextLinesFormatter(this);
	else if (mode == Mode::BINARY)
		formatter = new BinaryFormatter(this);
}

Stream::~Stream() {
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


// read the complete file into the buffer
bytes Stream::read_complete() {
	static const int CHUNK_SIZE = 2048;
	bytes buf, chunk;
	chunk.resize(CHUNK_SIZE);
	while (true) {
		int r = read(chunk);
		if (r <= 0)
			return buf;
		buf += chunk.sub_ref(0, r);
	};
	return buf;
}


unsigned char Stream::read_byte() {
	return formatter->read_byte();
}

char Stream::read_char() {
	return formatter->read_char();
}

unsigned int Stream::read_word() {
	return formatter->read_word();
}

unsigned int Stream::read_word_reversed() {
	return formatter->read_word_reversed();
}

int Stream::read_int() {
	return formatter->read_int();
}

float Stream::read_float() {
	return formatter->read_float();
}

bool Stream::read_bool() {
	return formatter->read_bool();
}

string Stream::read_str() {
	return formatter->read_str();
}

void Stream::read_vector(void *v) {
	formatter->read_vector(v);
}

void Stream::write_byte(unsigned char b) {
	formatter->write_byte(b);
}

void Stream::write_char(char c) {
	formatter->write_char(c);
}

void Stream::write_bool(bool b) {
	formatter->write_bool(b);
}

void Stream::write_word(unsigned int w) {
	formatter->write_word(w);
}

void Stream::write_int(int i) {
	formatter->write_int(i);
}

void Stream::write_float(float f) {
	formatter->write_float(f);
}

void Stream::write_str(const string& s) {
	formatter->write_str(s);
}

void Stream::write_vector(const void *v) {
	formatter->write_vector(v);
}

void Stream::read_comment() {
	formatter->read_comment();
}

void Stream::write_comment(const string &str) {
	formatter->write_comment(str);
}




