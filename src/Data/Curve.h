/*
 * Curve.h
 *
 *  Created on: 19.04.2014
 *      Author: michi
 */

#ifndef SRC_DATA_CURVE_H_
#define SRC_DATA_CURVE_H_

#include "../lib/base/pointer.h"
#include "../Stuff/Observable.h"

class Song;
class Track;
class AudioEffect;
class Module;
namespace kaba {
	class Class;
};


struct CurveTarget {
	float *p;
	string temp_name;
	string temp_name_nice;
	CurveTarget();
	explicit CurveTarget(float *p);
	CurveTarget(float *p, const string &name, const string &name_nice);
	void from_string(const string &str, Track *t);
	string str(Track *t) const;
	string nice_str(Track *t) const;
	//Track *track(Song *s) const;

	static Array<CurveTarget> enumerate_track(Track *t);
	static Array<CurveTarget> enumerate_module(Module *c, const string &prefix, const string &prefix_nice);
	static Array<CurveTarget> enumerate_type(char *p, const kaba::Class *t, const string &prefix, const string &prefix_nice);
};

class Curve : public Sharable<Observable<VirtualBase>> {
public:
	Curve();
	virtual ~Curve();

	enum {
		TYPE_LINEAR,
		TYPE_LOG,
	};

	string name;
	CurveTarget target;
	float temp_value;
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

	string get_target(Track *t);
};

#endif /* SRC_DATA_CURVE_H_ */
