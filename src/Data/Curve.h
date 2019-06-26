/*
 * Curve.h
 *
 *  Created on: 19.04.2014
 *      Author: michi
 */

#ifndef SRC_DATA_CURVE_H_
#define SRC_DATA_CURVE_H_

#include "../Stuff/Observable.h"

class Song;
class Track;
class AudioEffect;
class Module;
namespace Kaba {
	class Class;
};

class Curve : public Observable<VirtualBase> {
public:
	Curve();
	virtual ~Curve();

	enum {
		TYPE_LINEAR,
		TYPE_LOG,
	};

	struct Target {
		float *p;
		string temp_name;
		string temp_name_nice;
		Target();
		explicit Target(float *p);
		Target(float *p, const string &name, const string &name_nice);
		void from_string(const string &str, Song *a);
		string str(Song *s) const;
		string nice_str(Song *s) const;
		Track *track(Song *s) const;

		static Array<Target> enumerate(Song *s);
		static Array<Target> enumerate_track(Track *t, const string &prefix, const string &prefix_nice);
		static Array<Target> enumerate_module(Module *c, const string &prefix, const string &prefix_nice);
		static Array<Target> enumerate_type(char *p, const Kaba::Class *t, const string &prefix, const string &prefix_nice);
	};

	string name;
	Array<Target> targets;
	Array<float> temp_values;
	int type;

	float min, max;

	struct Point {
		int pos;
		float value;
	};

	Array<Point> points;

	float get(int pos);

	void apply(int pos);
	void unapply();

	string get_targets(Song *s);
};

#endif /* SRC_DATA_CURVE_H_ */
