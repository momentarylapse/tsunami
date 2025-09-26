/*
 * BinaryBuffer.cpp
 *
 *  Created on: Jan 8, 2021
 *      Author: michi
 */


#include "BinaryBuffer.h"
#include "../math/vec3.h"


BinaryBuffer::BinaryBuffer() {
	pos = 0;
	block_pos = 0;
}

void BinaryBuffer::read(void *p, int size) {
	memcpy(p, &data[pos], size);
	pos += size;
}

void BinaryBuffer::clear() {
	pos = 0;
	data.clear();
}

void BinaryBuffer::start_block() {
	data.resize(data.num + 4);
	//buffer.pos += 4;
	block_pos = data.num;//buffer.pos;
}

void BinaryBuffer::end_block() {
	*(int*)&data[block_pos-4] = data.num - block_pos; // pure content
}


void BinaryBuffer::operator>>(int &i) {
	i = *(int*)&data[pos];
	pos += sizeof(i);
}

void BinaryBuffer::operator>>(float &f) {
	read(&f, sizeof(f));
}

void BinaryBuffer::operator>>(bool &b) {
	read(&b, sizeof(b));
}

void BinaryBuffer::operator>>(char &c) {
	read(&c, sizeof(c));
}

void BinaryBuffer::operator>>(string &s) {
	int n;
	read(&n, sizeof(n));
	s.resize(n);
	read(s.data, n);
}

void BinaryBuffer::operator>>(vec3 &v) {
	read(&v, sizeof(v));
}

int BinaryBuffer::get_pos() {
	return pos;
}

void BinaryBuffer::set_pos(int _pos) {
	pos = _pos;
}



void BinaryBuffer::operator<<(int i) {
	data += bytes((char*)&i, sizeof(i));
}
void BinaryBuffer::operator<<(float f) {
	data += bytes((char*)&f, sizeof(f));
}
void BinaryBuffer::operator<<(bool b) {
	data += bytes((char*)&b, sizeof(b));
}
void BinaryBuffer::operator<<(char c) {
	data += bytes((char*)&c, sizeof(c));
}
void BinaryBuffer::operator<<(const string &s) {
	data += bytes((char*)&s.num, sizeof(int));
	data += s;
}
void BinaryBuffer::operator<<(const vec3 &v) {
	data += bytes((char*)&v, sizeof(v));
}

