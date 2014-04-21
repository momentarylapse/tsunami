/*
 * Curve.cpp
 *
 *  Created on: 19.04.2014
 *      Author: michi
 */

#include "Curve.h"
#include "AudioFile.h"
#include "../Plugins/Effect.h"
#include "../lib/script/script.h"

Curve::Target::Target()
{
	p = NULL;
}

Curve::Target::Target(float *_p)
{
	p = _p;
}

string Curve::Target::str(AudioFile *a)
{
	if (!p)
		return "";
	foreachi(Track *t, a->track, i){
		if (p == &t->volume)
			return format("t:%d:volume", i);
		if (p == &t->panning)
			return format("t:%d:panning", i);
		foreachi(Effect *fx, t->fx, j){
			PluginData *pd = fx->get_config();
			if (pd){
				foreach(Script::ClassElement &e, pd->type->element)
					if (p == (float*)((char*)pd + e.offset))
						return format("t:%d:fx:%d:", i, j) + e.name;
			}
		}
	}
	return "";
}

string Curve::Target::niceStr(AudioFile *a)
{
	if (!p)
		return "none";
	foreachi(Track *t, a->track, i){
		if (p == &t->volume)
			return format("Track[%d].volume", i);
		if (p == &t->panning)
			return format("Track[%d].panning", i);
		foreachi(Effect *fx, t->fx, j){
			PluginData *pd = fx->get_config();
			if (pd){
				foreach(Script::ClassElement &e, pd->type->element)
					if (p == (float*)((char*)pd + e.offset))
						return format("Track[%d].fx[%d].", i, j) + e.name;
			}
		}
	}
	return "???";
}

Array<Curve::Target> Curve::Target::enumerate(AudioFile *a)
{
	Array<Target> r;
	foreach(Track *t, a->track)
		r.append(enumerateTrack(a, t));
	return r;
}
Array<Curve::Target> Curve::Target::enumerateTrack(AudioFile *a, Track *t)
{
	Array<Target> r;
	r.add(Target(&t->volume));
	r.add(Target(&t->panning));
	foreach(Effect *fx, t->fx)
		r.append(enumerateEffect(a, t, fx));
	r.append(enumerateSynth(a, t, t->synth));
	return r;
}
Array<Curve::Target> Curve::Target::enumerateEffect(AudioFile *a, Track *t, Effect *fx)
{
	Array<Target> r;
	PluginData *pd = fx->get_config();
	if (!pd)
		return r;
	foreach(Script::ClassElement &e, pd->type->element)
		if (e.type->name == "float")
			r.add(Target((float*)((char*)pd + e.offset)));
	return r;
}
Array<Curve::Target> Curve::Target::enumerateSynth(AudioFile *a, Track *t, Synthesizer *s)
{
	Array<Target> r;
	return r;
}


Curve::Curve() :
	Observable("Curve")
{
	min = 0;
	max = 1;
	type = TYPE_LINEAR;
}

Curve::~Curve()
{
}

void Curve::add(int pos, float value)
{
	Point p;
	p.pos = pos;
	p.value = value;
	for (int i=0; i<points.num; i++)
		if (pos < points[i].pos){
			points.insert(p, i);
			return;
		}
	points.add(p);
}

float Curve::get(int pos)
{
	if (points.num == 0)
		return min;
	if (pos < points[0].pos)
		return points[0].value;
	for (int i=1; i<points.num; i++)
		if (pos < points[i].pos){
			float dv = points[i].value - points[i-1].value;
			float dp = points[i].pos - points[i-1].pos;
			return points[i-1].value + dv * (pos - points[i-1].pos) / dp;
		}
	return points.back().value;
}



void Curve::apply(int pos)
{
	temp_value.resize(target.num);
	for (int i=0; i<target.num; i++){
		temp_value[i] = *target[i].p;
		*target[i].p = get(pos);
	}
}

void Curve::unapply()
{
	for (int i=0; i<target.num; i++){
		*target[i].p = temp_value[i];
	}
}

string Curve::getTargets(AudioFile *a)
{
	string tt;
	foreachi(Target &t, target, i){
		if (i > 0)
			tt += ", ";
		tt += t.niceStr(a);
	}
	return tt;
}

