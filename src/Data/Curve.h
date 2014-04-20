/*
 * Curve.h
 *
 *  Created on: 19.04.2014
 *      Author: michi
 */

#ifndef CURVE_H_
#define CURVE_H_

#include "../Stuff/Observable.h"

class AudioFile;
class Track;
class Effect;
class Synthesizer;

class Curve : public Observable
{
public:
	Curve();
	virtual ~Curve();

	enum
	{
		TYPE_LINEAR,
		TYPE_LOG,
	};

	struct Target
	{
		float *p;
		Target();
		Target(float *p);
		void fromString(const string &str, AudioFile *a);
		string str(AudioFile *a);

		static Array<Target> enumerate(AudioFile *a);
		static Array<Target> enumerateTrack(AudioFile *a, Track *t);
		static Array<Target> enumerateEffect(AudioFile *a, Track *t, Effect *fx);
		static Array<Target> enumerateSynth(AudioFile *a, Track *t, Synthesizer *s);
	};

	string name;
	Target target;
	int type;

	float min, max;

	struct Point
	{
		int pos;
		float value;
	};

	Array<Point> points;

	void add(int pos, float value);
	float get(int pos);
};

#endif /* CURVE_H_ */
