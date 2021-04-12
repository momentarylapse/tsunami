/*
 * BeatSource.h
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#ifndef SRC_MODULE_BEATS_BEATSOURCE_H_
#define SRC_MODULE_BEATS_BEATSOURCE_H_

#include "../Port/Port.h"
#include "../Module.h"

class DummyBeatSource;

class BeatSource : public Module {
public:
	BeatSource();

	void _cdecl __init__();
	void _cdecl __delete__() override;

	class Output : public Port {
	public:
		Output(BeatSource *s);
		int read_beats(Array<Beat> &beats, int samples) override;
		BeatSource *source;
	};

	virtual int _cdecl read(Array<Beat> &beats, int samples){ return samples; }
	virtual void _cdecl reset(){}

	virtual void _cdecl set_pos(int pos){}
	virtual int _cdecl get_pos(){ return 0; }

	virtual int _cdecl beats_per_bar(){ return 1; }
	virtual int _cdecl cur_bar(){ return 0; }
	virtual int _cdecl cur_beat(){ return 0; }
	virtual float _cdecl beat_fraction(){ return 0; }
};

class DummyBeatSource : public BeatSource {
public:
	int _cdecl read(Array<Beat> &beats, int samples) override { return samples; }
};

BeatSource *CreateBeatSource(Session *session, const string &name);


#endif /* SRC_MODULE_BEATS_BEATSOURCE_H_ */
