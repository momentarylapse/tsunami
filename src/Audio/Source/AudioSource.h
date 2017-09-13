/*
 * AudioSource.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SRC_AUDIO_SOURCE_AUDIOSOURCE_H_
#define SRC_AUDIO_SOURCE_AUDIOSOURCE_H_

#include "../../Data/Song.h"

class AudioSource : public VirtualBase
{
public:
	AudioSource();
	virtual ~AudioSource(){}

	void _cdecl __init__();
	virtual void _cdecl __delete__();

	virtual int _cdecl read(BufferBox &buf){ return 0; }
	int _cdecl readResize(BufferBox &buf);
	virtual void _cdecl reset(){}
	virtual Range _cdecl range(){ return Range(0, 0); }
	virtual int _cdecl getPos(){ return 0; }
	virtual void _cdecl seek(int pos){}
	virtual int _cdecl getSampleRate(){ return DEFAULT_SAMPLE_RATE; }
	virtual int _cdecl getNumSamples(){ return 0; }
	virtual Array<Tag> _cdecl getTags();
	virtual bool _cdecl needsReset(){ return false; }
};

#endif /* SRC_AUDIO_SOURCE_AUDIOSOURCE_H_ */
