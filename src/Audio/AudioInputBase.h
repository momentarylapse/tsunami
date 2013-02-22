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
	virtual bool Start(int sample_rate) = 0;
	virtual void Stop() = 0;

	virtual bool IsCapturing() = 0;
	virtual int GetDelay() = 0;
	virtual void ResetSync() = 0;

	virtual float GetSampleRate() = 0;
	virtual BufferBox GetSomeSamples(int num_samples) = 0;

	virtual int DoCapturing() = 0;
};

#endif /* AUDIOINPUTBASE_H_ */
