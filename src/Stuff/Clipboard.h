/*
 * Clipboard.h
 *
 *  Created on: 21.12.2012
 *      Author: michi
 */

#ifndef CLIPBOARD_H_
#define CLIPBOARD_H_

#include "Observable.h"
class AudioFile;
class BufferBox;

class Clipboard : public Observable
{
public:
	Clipboard();
	virtual ~Clipboard();

	void Copy(AudioFile *a);
	void Paste(AudioFile *a);
	void Clear();
	bool HasData();
	bool CanCopy(AudioFile *a);

private:
	BufferBox *buf;
	int sample_rate;
};

#endif /* CLIPBOARD_H_ */
