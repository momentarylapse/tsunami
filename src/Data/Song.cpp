/*
 * AudioFile.cpp
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#include "Song.h"
#include "base.h"
#include "Track.h"
#include "TrackLayer.h"
#include "Sample.h"
#include "CrossFade.h"
#include "SongSelection.h"
#include "TrackMarker.h"
#include "Rhythm/Bar.h"
#include "../Action/ActionManager.h"
#include "../Action/Bar/ActionBarAdd.h"
#include "../Action/Bar/ActionBarDelete.h"
#include "../Action/Bar/ActionBarEdit.h"
#include "../Action/Sample/ActionSampleAdd.h"
#include "../Action/Sample/ActionSampleDelete.h"
#include "../Action/Sample/ActionSampleEditName.h"
#include "../Action/Sample/ActionSampleReplaceBuffer.h"
#include "../Action/Song/ActionSongDeleteSelection.h"
#include "../Action/Song/Data/ActionSongChangeAllTrackVolumes.h"
#include "../Action/Song/Data/ActionSongSetDefaultFormat.h"
#include "../Action/Song/Data/ActionSongSetSampleRate.h"
#include "../Action/Tag/ActionTagAdd.h"
#include "../Action/Tag/ActionTagDelete.h"
#include "../Action/Tag/ActionTagEdit.h"
#include "../Action/Track/ActionTrackAdd.h"
#include "../Action/Track/ActionTrackDelete.h"
#include "../Action/Track/Sample/ActionTrackInsertSelectedSamples.h"
#include "../Action/Track/Sample/ActionTrackSampleFromSelection.h"
#include "../Module/Synth/DummySynthesizer.h"
#include "../Module/Audio/AudioEffect.h"
#include "../Module/Midi/MidiEffect.h"
#include "../Stuff/Log.h"
#include "../lib/threads/Mutex.h"
#include "../lib/hui/language.h"
#include <assert.h>


int get_track_index(Track *t) {
	if (t)
		return t->get_index();
	return -1;
}

Song::Error::Error(const string &_message) : Exception(_message) {
}

Song::Song(Session *session, int _sample_rate) :
	Data(session)
{
	//msg_write("  new Song " + p2s(this));
	sample_rate = _sample_rate;
	default_format = SampleFormat::SAMPLE_FORMAT_16;
	compression = 0;
}

void Song::__init__(Session *session, int _sample_rate) {
	new(this) Song(session, _sample_rate);
}

void Song::__delete__() {
	this->Song::~Song();
}


const string Song::MESSAGE_NEW = "New";
const string Song::MESSAGE_ADD_TRACK = "AddTrack";
const string Song::MESSAGE_DELETE_TRACK = "DeleteTrack";
const string Song::MESSAGE_ADD_EFFECT = "AddEffect";
const string Song::MESSAGE_DELETE_EFFECT = "DeleteEffect";
const string Song::MESSAGE_ADD_SAMPLE = "AddSample";
const string Song::MESSAGE_DELETE_SAMPLE = "DeleteSample";
const string Song::MESSAGE_ADD_LAYER = "AddLayer";
const string Song::MESSAGE_EDIT_LAYER = "EditLayer";
const string Song::MESSAGE_DELETE_LAYER = "DeleteLayer";
const string Song::MESSAGE_CHANGE_CHANNELS = "ChangeChannels";
const string Song::MESSAGE_EDIT_BARS = "EditBars";
const string Song::MESSAGE_ENABLE_FX = "EnableFx";

bool Tag::operator ==(const Tag &o) const {
	return key == o.key and value == o.value;
}

bool Tag::operator !=(const Tag &o) const {
	return !(*this == o);
}

bool Tag::valid() const {
	return (key != "") and (value != "");
}


void Song::add_tag(const string &key, const string &value) {
	if ((key != "") and (value != ""))
		execute(new ActionTagAdd({key, value}));
}

void Song::edit_tag(int index, const string &key, const string &value) {
	execute(new ActionTagEdit(index, {key, value}));
}

void Song::delete_tag(int index) {
	execute(new ActionTagDelete(index));
}

void Song::change_all_track_volumes(Track *t, float volume) {
	execute(new ActionSongChangeAllTrackVolumes(this, t, volume));
}

void Song::set_sample_rate(int _sample_rate) {
	if (_sample_rate > 0)
		execute(new ActionSongSetSampleRate(this, _sample_rate));
}

void Song::set_default_format(SampleFormat _format) {
	execute(new ActionSongSetDefaultFormat(_format, compression));
}

void Song::set_compression(int _compression) {
	execute(new ActionSongSetDefaultFormat(default_format, _compression));
}

// delete all data
void Song::reset() {
	action_manager->reset();

	filename = "";
	tags.clear();
	bars.clear();
	default_format = SampleFormat::SAMPLE_FORMAT_16;
	compression = 0;
	sample_rate = DEFAULT_SAMPLE_RATE;

	secret_data.clear();

	tracks.clear();
	samples.clear();

	action_manager->reset();

	notify();
	notify(MESSAGE_NEW);
}

Song::~Song() {
	reset();
	//msg_write("  del Song " + p2s(this));
}

bool Song::is_empty() {
	if (!filename.is_empty())
		return false;
	return action_manager->is_save();
}

Range Song::range() {
	Range r = Range::NONE;

	for (Track *t: weak(tracks))
		r = r or t->range();

	return r;
}

Range Song::range_with_time() {
	Range r = range();

	if (bars.num > 0)
		r = r or bars.range();

	return r;
}

void disect_time(int t, int sample_rate, bool &sign, int &hours, int &min, int &sec, int &msec) {
	sign = (t < 0);
	if (sign)
		t = -t;
	hours = (t / 3600 / sample_rate);
	min = ((t / 60 / sample_rate) % 60);
	sec = ((t / sample_rate) % 60);
	msec = (((t - sample_rate * (t / sample_rate)) * 1000 / sample_rate) % 1000);
}

string Song::get_time_str(int t) {
	bool sign;
	int _hours, _min, _sec, _msec;
	disect_time(t, sample_rate, sign, _hours, _min, _sec, _msec);
	if (_hours > 0)
		return format("%s%d:%.2d:%.2d,%.3d",sign?"-":"",_hours,_min,_sec,_msec);
	if (_min > 0)
		return format("%s%d:%.2d,%.3d",sign?"-":"",_min,_sec,_msec);
	return format("%s%.2d,%.3d",sign?"-":"",_sec,_msec);
}

string Song::get_time_str_fuzzy(int t, float dt) {
	bool sign;
	int _hours, _min, _sec, _msec;
	disect_time(t, sample_rate, sign, _hours, _min, _sec, _msec);
	if (dt < 1.0) {
		if (_hours > 0)
			return format("%s%d:%.2d:%.2d,%.3d",sign?"-":"",_hours,_min,_sec,_msec);
		if (_min > 0)
			return format("%s%d:%.2d,%.3d",sign?"-":"",_min,_sec,_msec);
		return format("%s%.2d,%.3d",sign?"-":"",_sec,_msec);
	} else {
		if (_hours > 0)
			return format("%s%d:%.2d:%.2d",sign?"-":"",_hours,_min,_sec);
		if (_min > 0)
			return format("%s%d:%.2d",sign?"-":"",_min,_sec);
		return format("%s%.2d",sign?"-":"",_sec);
	}
}

string Song::get_time_str_long(int t) {
	bool sign;
	int _hours, _min, _sec, _msec;
	disect_time(t, sample_rate, sign, _hours, _min, _sec, _msec);
	if (_hours > 0)
		return format("%s%dh %.2dm %.2ds %.3dms",sign?"-":"",_hours,_min,_sec,_msec);
	if (_min > 0)
		return format("%s%dm %.2ds %.3dms",sign?"-":"",_min,_sec,_msec);
	return format("%s%ds %.3dms",sign?"-":"",_sec,_msec);
}


int Song::bar_offset(int index) {
	int pos = 0;
	for (int i=0; i<min(index, bars.num); i++)
		pos += bars[i]->length;
	return pos;
}

Bar *song_bar_at(Song *s, int pos) {
	for (Bar *b: weak(s->bars))
		if (b->range().is_inside(pos))
			return b;
	return nullptr;
}

int song_bar_divisor(Song *s, int pos) {
	Bar *b = song_bar_at(s, pos);
	if (!b)
		return 1;
	if (b->is_pause())
		return 1;
	return b->divisor;
}



Track *Song::add_track(SignalType type, int index) {
	if (type == SignalType::BEATS) {
		// force single time track
		if (time_track())
			throw Error(_("There already is one rhythm track."));
	}
	if (index < 0)
		index = tracks.num;
	//msg_write("--------new track");
	Track *t = new Track(type, CreateSynthesizer(session, ""));
	//msg_write("--------layer add");
	t->layers.add(new TrackLayer(t));
	//msg_write("--------execute");
	return (Track*)execute(new ActionTrackAdd(t, index));
}

Track *Song::add_track_after(SignalType type, Track *ref) {
	int index = ref->get_index();
	return add_track(type, index + 1);
}

void Song::insert_selected_samples(const SongSelection &sel) {
	if (sel.num_samples() > 0)
		execute(new ActionTrackInsertSelectedSamples(sel));
}

void Song::delete_selected_samples(const SongSelection &sel) {
	action_manager->group_begin();
	for (Track *t: weak(tracks))
		for (TrackLayer *l: weak(t->layers)) {
			for (int j=l->samples.num-1; j>=0; j--)
				if (sel.has(l->samples[j].get()))
					l->delete_sample_ref(l->samples[j].get());
		}
	action_manager->group_end();
}

/*Song::Layer *Song::addLayer(const string &name, int index) {
	return (Song::Layer*)execute(new ActionLayerAdd(name, index));
}

void Song::deleteLayer(int index) {
	if (layers.num < 2)
		throw Error(_("At least one layer has to exist."));
	execute(new ActionLayerDelete(index));
}

void Song::mergeLayers(int source, int target) {
	if (layers.num < 2)
		throw Error(_("At least one layer has to exist."));
	if (source == target)
		throw Error(_("Can't merge a layer with itself."));
	execute(new ActionLayerMerge(source, target));
}

void Song::moveLayer(int source, int target) {
	execute(new ActionLayerMove(source, target));
}

void Song::renameLayer(int index, const string &name) {
	execute(new ActionLayerRename(index, name));
}*/

void Song::delete_track(Track *track) {
	if (tracks.num < 2)
		throw Error(_("At least one layer has to exist."));
	execute(new ActionTrackDelete(track));
}

Sample *Song::create_sample_audio(const string &name, const AudioBuffer &buf) {
	auto *sample = new Sample(name, buf);
	add_sample(sample);
	return sample;
}

Sample *Song::create_sample_midi(const string &name, const MidiNoteBuffer &buf) {
	auto *sample = new Sample(name, buf);
	add_sample(sample);
	return sample;
}

void Song::add_sample(Sample *sample) {
	execute(new ActionSampleAdd(sample));
}

void Song::delete_sample(Sample *s) {
	if (s->ref_count > 0)
		throw Error(_("Can not delete a sample that is in use."));
	execute(new ActionSampleDelete(s));
}

void Song::edit_sample_name(Sample *s, const string &name) {
	execute(new ActionSampleEditName(s, name));
}

void Song::sample_replace_buffer(Sample *s, AudioBuffer *buf) {
	execute(new ActionSampleReplaceBuffer(s, buf));
}

void Song::delete_selection(const SongSelection &sel) {
	execute(new ActionSongDeleteSelection(sel));
}

void Song::create_samples_from_selection(const SongSelection &sel, bool auto_delete) {
	if (!sel.range().is_empty())
		execute(new ActionTrackSampleFromSelection(sel, auto_delete));
}

void Song::add_bar(int index, const BarPattern &b, int mode) {
	if (index >= 0)
		execute(new ActionBarAdd(index, b, mode));
	else
		execute(new ActionBarAdd(bars.num, b, mode));
}

void Song::add_pause(int index, int length, int mode) {
	if (index >= 0)
		execute(new ActionBarAdd(index, BarPattern(length, 0, 0), mode));
	else
		execute(new ActionBarAdd(bars.num, BarPattern(length, 0, 0), mode));
}

void Song::edit_bar(int index, const BarPattern &b, int mode) {
	execute(new ActionBarEdit(index, b, mode));
}

void Song::delete_bar(int index, bool affect_midi) {
	execute(new ActionBarDelete(index, affect_midi));
}

void Song::delete_time_interval(int index, const Range &range) {
	//execute(new ActionBarDelete(index, affect_midi));
}

void Song::invalidate_all_peaks() {
	for (auto t: tracks)
		t->invalidate_all_peaks();
	for (auto s: samples)
		if (s->buf)
			s->buf->peaks.clear();
}


Sample* Song::get_sample_by_uid(int uid) {
	for (Sample *s: weak(samples))
		if (s->uid == uid)
			return s;
	return nullptr;
}

Track *Song::time_track() {
	for (Track *t: weak(tracks))
		if (t->type == SignalType::BEATS)
			return t;
	return nullptr;
}

Array<TrackMarker*> Song::get_parts() {
	Track *t = time_track();
	if (!t)
		return {};
	return t->layers[0]->markers_sorted();
}

string Song::get_tag(const string &key) {
	for (Tag &t: tags)
		if (t.key == key)
			return t.value;
	return "";
}

Array<TrackLayer*> Song::layers() const {
	Array<TrackLayer*> layers;
	for (Track *t: weak(tracks))
		layers.append(weak(t->layers));
	return layers;
}



Song *copy_song_from_selection(Song *song, const SongSelection &sel) {
	Song *ss = new Song(song->session, song->sample_rate);
	ss->tags = song->tags;
	for (Bar *b: weak(song->bars))
		if (sel.range().covers(b->range())) {
			int before = b->range().offset - sel.range().offset;
			if (ss->bars.num == 0 and before > 0)
				ss->bars.add(new Bar(before, 0, 0)); // pause
			ss->bars.add(b->copy());
		}
	for (Track *t: weak(song->tracks)) {
		if (!sel.has(t))
			continue;
		Track *tt = new Track(t->type, (Synthesizer*)t->synth->copy());
		ss->tracks.add(tt);
		tt->name = t->name;
		tt->volume = t->volume;
		tt->panning = t->panning;
		tt->muted = t->muted;
		for (auto *f: weak(t->fx))
			tt->fx.add((AudioEffect*)f->copy());
		for (auto *f: weak(t->midi_fx))
			tt->midi_fx.add((MidiEffect*)f->copy());
		tt->synth = (Synthesizer*)t->synth->copy();
		tt->instrument = t->instrument;
		tt->channels = t->channels;
		for (TrackLayer *l: weak(t->layers)) {
			if (!sel.has(l))
				continue;
			auto *ll = new TrackLayer(tt);
			tt->layers.add(ll);
			for (auto *n: weak(l->midi))
				if (sel.has(n))
					ll->midi.add(n->copy(-sel.range().offset));
			for (auto &b: l->buffers) {
				if (b.range().overlaps(sel.range())) {
					Range ri = b.range() and sel.range();
					AudioBuffer bb;
					l->read_buffers(bb, ri, true);
					ll->buffers.add(bb);
					ll->buffers.back().offset = ri.offset - sel.range().offset;
				}
			}
			for (auto *m: weak(l->markers))
				if (sel.has(m))
					ll->markers.add(m->copy(- sel.range().offset));
			ll->fades = l->fades; // TODO...
		}
	}
	return ss;
}
