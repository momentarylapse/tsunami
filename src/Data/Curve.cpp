/*
 * Curve.cpp
 *
 *  Created on: 19.04.2014
 *      Author: michi
 */

#include "Curve.h"
#include "../Plugins/Effect.h"
#include "../Audio/Synth/Synthesizer.h"
#include "../lib/script/script.h"
#include "Song.h"

Curve::Target::Target()
{
	p = NULL;
}

Curve::Target::Target(float *_p)
{
	p = _p;
}

Curve::Target::Target(float *_p, const string &name, const string &name_nice)
{
	p = _p;
	temp_name = name;
	temp_name_nice = name_nice;
}

string Curve::Target::str(Song *a)
{
	Array<Target> list = enumerate(a);
	for (Target &t : list)
		if (t.p == p)
			return t.temp_name;
	return "";
}

string Curve::Target::niceStr(Song *a)
{
	Array<Target> list = enumerate(a);
	for (Target &t : list)
		if (t.p == p)
			return t.temp_name_nice;
	return "";
}

Array<Curve::Target> Curve::Target::enumerate(Song *a)
{
	Array<Target> list;
	foreachi(Track *t, a->tracks, i)
		enumerateTrack(t, list, format("t:%d", i), format("track[%d]", i));
	return list;
}
void Curve::Target::enumerateTrack(Track *t, Array<Target> &list, const string &prefix, const string &prefix_nice)
{
	list.add(Target(&t->volume, prefix + ":volume", prefix_nice + ".volume"));
	list.add(Target(&t->panning, prefix + ":panning", prefix_nice + ".panning"));
	foreachi(Effect *fx, t->fx, i)
		enumerateConfigurable(fx, list, prefix + format(":fx:%d", i), prefix_nice + format(".fx[%d]", i));
	enumerateConfigurable(t->synth, list, prefix + ":s", prefix_nice + ".synth");
}
void Curve::Target::enumerateConfigurable(Configurable *c, Array<Target> &list, const string &prefix, const string &prefix_nice)
{
	PluginData *pd = c->get_config();
	if (pd)
		enumerateType((char*)pd, pd->type, list, prefix, prefix_nice);
}

void Curve::Target::enumerateType(char *pp, Script::Type *t, Array<Target> &list, const string &prefix, const string &prefix_nice)
{
	if (t->name == "float"){
		list.add(Target((float*)pp, prefix, prefix_nice));
	}else if (t->is_array){
		for (int i=0; i<t->array_length; i++){
			enumerateType(pp + t->parent->size * i, t->parent, list, prefix + format(":%d", i), prefix_nice + format("[%d]", i));
		}
	}else if (t->is_super_array){
		DynamicArray *da = (DynamicArray*)pp;
		for (int i=0; i<da->num; i++){
			enumerateType(pp + da->element_size * i, t->parent, list, prefix + format(":%d", i), prefix_nice + format("[%d]", i));
		}
	}else{
		for (auto &e : t->element)
			if (!e.hidden)
				enumerateType(pp + e.offset, e.type, list, prefix + ":" + e.name, prefix_nice + "." + e.name);
	}
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
	temp_values.resize(targets.num);
	for (int i=0; i<targets.num; i++){
		temp_values[i] = *targets[i].p;
		*targets[i].p = get(pos);
	}
}

void Curve::unapply()
{
	for (int i=0; i<targets.num; i++){
		*targets[i].p = temp_values[i];
	}
}

string Curve::getTargets(Song *a)
{
	string tt;
	foreachi(Target &t, targets, i){
		if (i > 0)
			tt += ", ";
		tt += t.niceStr(a);
	}
	return tt;
}

