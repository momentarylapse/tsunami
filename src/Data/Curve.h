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
namespace Kaba
{
	class Class;
};

class Curve : public Observable<VirtualBase>
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
		string temp_name;
		string temp_name_nice;
		Target();
		Target(float *p);
		Target(float *p, const string &name, const string &name_nice);
		void fromString(const string &str, Song *a);
		string str(Song *a);
		string niceStr(Song *a);

		static Array<Target> enumerate(Song *a);
		static void enumerateTrack(Track *t, Array<Target> &list, const string &prefix, const string &prefix_nice);
		static void enumerateModule(Module *c, Array<Target> &list, const string &prefix, const string &prefix_nice);
		static void enumerateType(char *p, Kaba::Class *t, Array<Target> &list, const string &prefix, const string &prefix_nice);
	};

	string name;
	Array<Target> targets;
	Array<float> temp_values;
	int type;

	float min, max;

	struct Point
	{
		int pos;
		float value;
	};

	Array<Point> points;

	float get(int pos);

	void apply(int pos);
	void unapply();

	string getTargets(Song *a);
};

#endif /* SRC_DATA_CURVE_H_ */
