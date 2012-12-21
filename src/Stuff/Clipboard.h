/*
 * Clipboard.h
 *
 *  Created on: 21.12.2012
 *      Author: michi
 */

#ifndef CLIPBOARD_H_
#define CLIPBOARD_H_

#include "Observable.h"

class Clipboard : public Observable
{
public:
	Clipboard();
	virtual ~Clipboard();
};

#endif /* CLIPBOARD_H_ */
