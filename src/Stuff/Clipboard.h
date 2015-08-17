/*
 * Clipboard.h
 *
 *  Created on: 21.12.2012
 *      Author: michi
 */

#ifndef CLIPBOARD_H_
#define CLIPBOARD_H_

#include "Observable.h"
class Song;
class BufferBox;
class AudioView;
class MidiData;

class Clipboard : public Observable
{
public:
	Clipboard();
	virtual ~Clipboard();

	void copy(AudioView *view);
	void paste(AudioView *view);
	void clear();
	bool hasData();
	bool canCopy(AudioView *view);

private:
	Song *temp;
	Array<int> ref_uid;
};

#endif /* CLIPBOARD_H_ */
