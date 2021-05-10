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


CurveTarget::CurveTarget() {
	p = nullptr;
}

CurveTarget::CurveTarget(float *_p) {
	p = _p;
}

CurveTarget::CurveTarget(float *_p, const string &name, const string &name_nice) {
	p = _p;
	temp_name = name;
	temp_name_nice = name_nice;
}

string CurveTarget::str(Track *t) const {
	auto list = enumerate_track(t);
	for (auto &ct : list)
		if (ct.p == p)
			return ct.temp_name;
	return "";
}

string CurveTarget::nice_str(Track *t) const {
	auto list = enumerate_track(t);
	for (auto &ct : list)
		if (ct.p == p)
			return ct.temp_name_nice;
	return "";
}

void CurveTarget::from_string(const string &str, Track *t) {
	auto targets = enumerate_track(t);
	p = nullptr;
	for (auto &ct: targets)
		if (ct.str(t) == str)
			*this = ct;
	if (!p)
		msg_error("can't find curve target " + str);
}

Array<CurveTarget> CurveTarget::enumerate_track(Track *t) {
	Array<CurveTarget> list;
	list.add(CurveTarget(&t->volume, "volume", "volume"));
	list.add(CurveTarget(&t->panning, "panning", "panning"));

	foreachi(auto *fx, weak(t->fx), i)
		list.append(enumerate_module(fx, format("fx:%d", i), "fx" + i2s_small(i)));
	foreachi(auto *fx, weak(t->midi_fx), i)
		list.append(enumerate_module(fx, format("mfx:%d", i), "mfx" + i2s_small(i)));
	list.append(enumerate_module(t->synth.get(), "s", "synth"));
	return list;
}
Array<CurveTarget> CurveTarget::enumerate_module(Module *c, const string &prefix, const string &prefix_nice) {
	Array<CurveTarget> list;
	auto *pd = c->get_config();
	if (pd) {
		list.append(enumerate_type((char*)pd, pd->kaba_class, prefix, prefix_nice));
	}
	return list;
}

Array<CurveTarget> CurveTarget::enumerate_type(char *pp, const kaba::Class *t, const string &prefix, const string &prefix_nice) {
	Array<CurveTarget> list;
	if (t->name == "float") {
		list.add(CurveTarget((float*)pp, prefix, prefix_nice));
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
	temp_value = 0;
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
	temp_value = *target.p;
	*target.p = get(pos);
}

void Curve::unapply() {
	*target.p = temp_value;
}

string Curve::get_target(Track *t) {
	return target.nice_str(t);
}

