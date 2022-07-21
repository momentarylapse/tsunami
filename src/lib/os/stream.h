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



class Stream : public Sharable<Empty> {
public:
	virtual ~Stream() {}

	// meta
	virtual void set_pos(int pos) = 0;
	virtual void seek(int delta) = 0;
	virtual int get_size32() = 0;
	virtual int64 get_size() = 0;
	virtual int get_pos() = 0;

	virtual int read_basic(void *buffer, int size) = 0;
	virtual int write_basic(const void *buffer, int size) = 0;
	int read(void *buffer, int size);
	int write(const void *buffer, int size);
	int read(bytes&);
	bytes read(int size);
	int write(const bytes&);

	virtual bool is_end() = 0;

	bytes read_complete();
};



#endif /* SRC_LIB_OS_STREAM_H_ */
