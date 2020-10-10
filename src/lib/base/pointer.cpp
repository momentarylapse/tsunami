/*
 * pointer.cpp
 *
 *  Created on: Oct 8, 2020
 *      Author: michi
 */

#include "pointer.h"


#if POINTER_DEBUG

#include "../file/msg.h"

void pdb(const string &s) {
	msg_write(s);
}

void pcrash(const char *msg) {
	msg_error(msg);
	int *p = nullptr;
	*p = 0;
	exit(1);
}
#endif




