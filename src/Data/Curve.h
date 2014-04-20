/*
 * Curve.h
 *
 *  Created on: 19.04.2014
 *      Author: michi
 */

#ifndef CURVE_H_
#define CURVE_H_

#include "../Stuff/Observable.h"

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

	string name;
	string target;
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
