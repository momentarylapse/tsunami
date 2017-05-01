/*
 * InputStreamAny.h
 *
 *  Created on: 16.08.2015
 *      Author: michi
 */

#ifndef SRC_AUDIO_AUDIOINPUTANY_H_
#define SRC_AUDIO_AUDIOINPUTANY_H_

#include "../View/Helper/PeakMeter.h"

class Device;

class InputStreamAny : public PeakMeterSource
{
public:

	InputStreamAny(const string &name, int sample_rate);
	virtual ~InputStreamAny();

	static const string MESSAGE_CAPTURE;

	virtual bool _cdecl start() = 0;
	virtual void _cdecl stop() = 0;

	virtual bool _cdecl isCapturing() = 0;
	virtual int _cdecl getDelay() = 0;
	virtual void _cdecl resetSync() = 0;

	virtual void _cdecl accumulate(bool enable) = 0;
	virtual void _cdecl resetAccumulation() = 0;
	virtual int _cdecl getSampleCount() = 0;

	// PeakMeterSource
	virtual float _cdecl getSampleRate(){ return sample_rate; }
	virtual void _cdecl getSomeSamples(BufferBox &buf, int num_samples) = 0;
	virtual int _cdecl getState() = 0;

	virtual void _cdecl setDevice(Device *d) = 0;
	virtual Device *_cdecl getDevice() = 0;

	virtual void _cdecl setBackupMode(int mode);
	int backup_mode;

	virtual int _cdecl getType() = 0;

	virtual void _cdecl setChunkSize(int size);
	virtual void _cdecl setUpdateDt(float dt);
	int chunk_size;
	float update_dt;

	int sample_rate;
};

#endif /* SRC_AUDIO_AUDIOINPUTANY_H_ */
