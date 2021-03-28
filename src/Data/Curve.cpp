/*
 * Curve.cpp
 *
 *  Created on: 19.04.2014
 *      Author: michi
 */

#include "Curve.h"
#include "Track.h"
#include "Song.h"
#include "../Module/Module.h"
#include "../Module/ModuleConfiguration.h"
#include "../Module/Audio/AudioEffect.h"
#include "../Module/Midi/MidiEffect.h"
#include "../Module/Synth/Synthesizer.h"
#include "../lib/kaba/kaba.h"

string i2s_small(int); // MidiData.cpp ?

const string NICE_SEP = u8" \u203a ";


Curve::Target::Target() {
	p = nullptr;
}

Curve::Target::Target(float *_p) {
	p = _p;
}

Curve::Target::Target(float *_p, const string &name, const string &name_nice) {
	p = _p;
	temp_name = name;
	temp_name_nice = name_nice;
}

string Curve::Target::str(Song *s) const {
	auto list = enumerate(s);
	for (Target &t : list)
		if (t.p == p)
			return t.temp_name;
	return "";
}

string Curve::Target::nice_str(Song *s) const {
	auto list = enumerate(s);
	for (Target &t : list)
		if (t.p == p)
			return t.temp_name_nice;
	return "";
}

void Curve::Target::from_string(const string &str, Song *s) {
	auto targets = enumerate(s);
	p = nullptr;
	for (auto t: targets)
		if (t.str(s) == str)
			*this = t;
	if (!p)
		msg_error("can't find curve target " + str);
}

Track* Curve::Target::track(Song *s) const {
	for (Track *track: weak(s->tracks)) {
		auto list = enumerate_track(track, "", "");
		for (Target &t : list)
			if (t.p == p)
				return track;
	}
	return nullptr;
}

Array<Curve::Target> Curve::Target::enumerate(Song *s) {
	Array<Target> list;
	foreachi(Track *t, weak(s->tracks), i)
		list.append(enumerate_track(t, format("t:%d", i), "track" + i2s_small(i)));
	return list;
}

Array<Curve::Target> Curve::Target::enumerate_track(Track *t, const string &prefix, const string &prefix_nice) {
	Array<Target> list;
	list.add(Target(&t->volume, prefix + ":volume", prefix_nice + NICE_SEP + "volume"));
	list.add(Target(&t->panning, prefix + ":panning", prefix_nice + NICE_SEP + "panning"));

	foreachi(auto *fx, weak(t->fx), i)
		list.append(enumerate_module(fx, prefix + format(":fx:%d", i), prefix_nice + NICE_SEP + "fx" + i2s_small(i)));
	foreachi(auto *fx, weak(t->midi_fx), i)
		list.append(enumerate_module(fx, prefix + format(":mfx:%d", i), prefix_nice + NICE_SEP + "mfx" + i2s_small(i)));
	list.append(enumerate_module(t->synth.get(), prefix + ":s", prefix_nice + NICE_SEP + "synth"));
	return list;
}
Array<Curve::Target> Curve::Target::enumerate_module(Module *c, const string &prefix, const string &prefix_nice) {
	Array<Target> list;
	auto *pd = c->get_config();
	if (pd) {
		list.append(enumerate_type((char*)pd, pd->kaba_class, prefix, prefix_nice));
	}
	return list;
}

Array<Curve::Target> Curve::Target::enumerate_type(char *pp, const kaba::Class *t, const string &prefix, const string &prefix_nice) {
	Array<Curve::Target> list;
	if (t->name == "float") {
		list.add(Target((float*)pp, prefix, prefix_nice));
	} else if (t->is_array()) {
		for (int i=0; i<t->array_length; i++) {
			list.append(enumerate_type(pp + t->param[0]->size * i, t->param[0], prefix + format(":%d", i), prefix_nice + i2s_small(i)));
		}
	} else if (t->is_super_array()) {
		auto *da = (DynamicArray*)pp;
		for (int i=0; i<da->num; i++) {
			list.append(enumerate_type(pp + da->element_size * i, t->param[0], prefix + format(":%d", i), prefix_nice + i2s_small(i)));
		}
	} else {
		for (auto &e : t->elements)
			if (!e.hidden())
				list.append(enumerate_type(pp + e.offset, e.type, prefix + ":" + e.name, prefix_nice + NICE_SEP + e.name));
	}
	return list;
}


Curve::Curve() {
	min = 0;
	max = 1;
	type = TYPE_LINEAR;
}

Curve::~Curve() {
}

float Curve::get(int pos) {
	if (points.num == 0)
		return min;
	if (pos < points[0].pos)
		return points[0].value;
	for (int i=1; i<points.num; i++)
		if (pos < points[i].pos) {
			float dv = points[i].value - points[i-1].value;
			float dp = (float)(points[i].pos - points[i-1].pos);
			return points[i-1].value + dv * (pos - points[i-1].pos) / dp;
		}
	return points.back().value;
}



void Curve::apply(int pos) {
	temp_values.resize(targets.num);
	for (int i=0; i<targets.num; i++) {
		temp_values[i] = *targets[i].p;
		*targets[i].p = get(pos);
	}
}

void Curve::unapply() {
	for (int i=0; i<targets.num; i++) {
		*targets[i].p = temp_values[i];
	}
}

string Curve::get_targets(Song *s) {
	string tt;
	foreachi(Target &t, targets, i) {
		if (i > 0)
			tt += ", ";
		tt += t.nice_str(s);
	}
	return tt;
}

