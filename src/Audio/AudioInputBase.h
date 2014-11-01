/*
 * AudioInputBase.h
 *
 *  Created on: 22.02.2013
 *      Author: michi
 */

#ifndef AUDIOINPUTBASE_H_
#define AUDIOINPUTBASE_H_

#include "../Data/BufferBox.h"

class AudioInputBase
{
public:
	virtual ~AudioInputBase(){}

	virtual bool start(int sample_rate) = 0;
	virtual void stop() = 0;

	virtual bool isCapturing() = 0;
	virtual int getDelay() = 0;
	virtual void resetSync() = 0;

	virtual void accumulate(bool enable) = 0;
	virtual void resetAccumulation() = 0;
	virtual int getSampleCount() = 0;

	virtual float getSampleRate() = 0;
	virtual void getSomeSamples(BufferBox &buf, int num_samples) = 0;

	virtual int doCapturing() = 0;
};

#endif /* AUDIOINPUTBASE_H_ */
