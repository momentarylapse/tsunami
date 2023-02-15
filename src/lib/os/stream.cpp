/*
 * stream.cpp
 *
 *  Created on: 21 Jul 2022
 *      Author: michi
 */

#include "stream.h"

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



