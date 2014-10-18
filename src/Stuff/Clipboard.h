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
class AudioView;
class MidiData;

class Clipboard : public Observable
{
public:
	Clipboard();
	virtual ~Clipboard();

	void Copy(AudioView *view);
	void Paste(AudioView *view);
	void Clear();
	bool HasData();
	bool CanCopy(AudioView *view);

private:
	int type;
	BufferBox *buf;
	MidiData *midi;
	int ref_uid;
	int sample_rate;
};

#endif /* CLIPBOARD_H_ */
