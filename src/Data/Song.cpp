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
#include "SongSelection.h"
#include "Curve.h"
#include "Rhythm/Bar.h"
#include "../Action/ActionManager.h"
#include "../Action/Bar/ActionBarAdd.h"
#include "../Action/Bar/ActionBarDelete.h"
#include "../Action/Bar/ActionBarEdit.h"
#include "../Action/Curve/ActionCurveAdd.h"
#include "../Action/Curve/ActionCurveAddPoint.h"
#include "../Action/Curve/ActionCurveDelete.h"
#include "../Action/Curve/ActionCurveDeletePoint.h"
#include "../Action/Curve/ActionCurveEdit.h"
#include "../Action/Curve/ActionCurveEditPoint.h"
#include "../Action/Sample/ActionSampleAdd.h"
#include "../Action/Sample/ActionSampleDelete.h"
#include "../Action/Sample/ActionSampleEditName.h"
#include "../Action/Sample/ActionSampleScale.h"
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
#include "../Stuff/Log.h"
#include "../lib/threads/Mutex.h"
#include <assert.h>


int get_track_index(Track *t)
{
	if (t)
		return t->get_index();
	return -1;
}

Song::Exception::Exception(const string &_message)
{
	message = _message;
}

Song::Song(Session *session, int _sample_rate) :
	Data(session)
{
	sample_rate = _sample_rate;
	default_format = SampleFormat::SAMPLE_FORMAT_16;
	compression = 0;
}

void Song::__init__(Session *session, int _sample_rate)
{
	new(this) Song(session, _sample_rate);
}

void Song::__delete__()
{
	this->Song::~Song();
}


const string Song::MESSAGE_NEW = "New";
const string Song::MESSAGE_ADD_TRACK = "AddTrack";
const string Song::MESSAGE_DELETE_TRACK = "DeleteTrack";
const string Song::MESSAGE_ADD_EFFECT = "AddEffect";
const string Song::MESSAGE_DELETE_EFFECT = "DeleteEffect";
const string Song::MESSAGE_ADD_CURVE = "AddCurve";
const string Song::MESSAGE_DELETE_CURVE = "DeleteCurve";
const string Song::MESSAGE_EDIT_CURVE = "EditCurve";
const string Song::MESSAGE_ADD_SAMPLE = "AddSample";
const string Song::MESSAGE_DELETE_SAMPLE = "DeleteSample";
const string Song::MESSAGE_ADD_LAYER = "AddLayer";
const string Song::MESSAGE_EDIT_LAYER = "EditLayer";
const string Song::MESSAGE_DELETE_LAYER = "DeleteLayer";
const string Song::MESSAGE_CHANGE_CHANNELS = "ChangeChannels";
const string Song::MESSAGE_EDIT_BARS = "EditBars";


void Song::add_tag(const string &key, const string &value)
{
	if ((key != "") and (value != ""))
		execute(new ActionTagAdd(Tag(key, value)));
}

void Song::edit_tag(int index, const string &key, const string &value)
{
	execute(new ActionTagEdit(index, Tag(key, value)));
}

void Song::delete_tag(int index)
{
	execute(new ActionTagDelete(index));
}

void Song::change_all_track_volumes(Track *t, float volume)
{
	execute(new ActionSongChangeAllTrackVolumes(this, t, volume));
}

void Song::set_sample_rate(int _sample_rate)
{
	if (_sample_rate > 0)
		execute(new ActionSongSetSampleRate(this, _sample_rate));
}

void Song::set_default_format(SampleFormat _format)
{
	execute(new ActionSongSetDefaultFormat(_format, compression));
}

void Song::set_compression(int _compression)
{
	execute(new ActionSongSetDefaultFormat(default_format, _compression));
}

// delete all data
void Song::reset()
{
	action_manager->reset();

	filename = "";
	tags.clear();
	bars.clear();
	default_format = SampleFormat::SAMPLE_FORMAT_16;
	compression = 0;
	sample_rate = DEFAULT_SAMPLE_RATE;

	for (Track *t: tracks)
		delete(t);
	tracks.clear();

	for (Sample *s: samples)
		delete(s);
	samples.clear();

	for (Curve *c: curves)
		delete(c);
	curves.clear();

	action_manager->reset();

	notify();
	notify(MESSAGE_NEW);
}

Song::~Song()
{
	reset();
}

bool Song::is_empty()
{
	if (filename.num > 0)
		return false;
	return action_manager->is_save();
}

Range Song::range()
{
	Range r = Range::EMPTY;

	for (Track *t: tracks)
		r = r or t->range();

	return r;
}

Range Song::range_with_time()
{
	Range r = range();

	if (bars.num > 0)
		r = r or bars.range();

	return r;
}

void disect_time(int t, int sample_rate, bool &sign, int &hours, int &min, int &sec, int &msec)
{
	sign = (t < 0);
	if (sign)
		t = -t;
	hours = (t / 3600 / sample_rate);
	min = ((t / 60 / sample_rate) % 60);
	sec = ((t / sample_rate) % 60);
	msec = (((t - sample_rate * (t / sample_rate)) * 1000 / sample_rate) % 1000);
}

string Song::get_time_str(int t)
{
	bool sign;
	int _hours, _min, _sec, _msec;
	disect_time(t, sample_rate, sign, _hours, _min, _sec, _msec);
	if (_hours > 0)
		return format("%s%d:%.2d:%.2d,%.3d",sign?"-":"",_hours,_min,_sec,_msec);
	if (_min > 0)
		return format("%s%d:%.2d,%.3d",sign?"-":"",_min,_sec,_msec);
	return format("%s%.2d,%.3d",sign?"-":"",_sec,_msec);
}

string Song::get_time_str_fuzzy(int t, float dt)
{
	bool sign;
	int _hours, _min, _sec, _msec;
	disect_time(t, sample_rate, sign, _hours, _min, _sec, _msec);
	if (dt < 1.0){
		if (_hours > 0)
			return format("%s%d:%.2d:%.2d,%.3d",sign?"-":"",_hours,_min,_sec,_msec);
		if (_min > 0)
			return format("%s%d:%.2d,%.3d",sign?"-":"",_min,_sec,_msec);
		return format("%s%.2d,%.3d",sign?"-":"",_sec,_msec);
	}else{
		if (_hours > 0)
			return format("%s%d:%.2d:%.2d",sign?"-":"",_hours,_min,_sec);
		if (_min > 0)
			return format("%s%d:%.2d",sign?"-":"",_min,_sec);
		return format("%s%.2d",sign?"-":"",_sec);
	}
}

string Song::get_time_str_long(int t)
{
	bool sign;
	int _hours, _min, _sec, _msec;
	disect_time(t, sample_rate, sign, _hours, _min, _sec, _msec);
	if (_hours > 0)
		return format("%s%dh %.2dm %.2ds %.3dms",sign?"-":"",_hours,_min,_sec,_msec);
	if (_min > 0)
		return format("%s%dm %.2ds %.3dms",sign?"-":"",_min,_sec,_msec);
	return format("%s%ds %.3dms",sign?"-":"",_sec,_msec);
}

int Song::bar_offset(int index)
{
	int pos = 0;
	for (int i=0; i<min(index, bars.num); i++)
		pos += bars[i]->length;
	return pos;
}



Track *Song::add_track(SignalType type, int index)
{
	if (type == SignalType::BEATS){
		// force single time track
		if (time_track())
			throw Exception(_("There already is one rhythm track."));
	}
	if (index < 0)
		index = tracks.num;
	return (Track*)execute(new ActionTrackAdd(new Track(type, CreateSynthesizer(session, "")), index));
}

Track *Song::add_track_after(SignalType type, Track *ref)
{
	int index = ref->get_index();
	return add_track(type, index + 1);
}

void Song::insert_selected_samples(const SongSelection &sel)
{
	if (sel.num_samples() > 0)
		execute(new ActionTrackInsertSelectedSamples(sel));
}

void Song::delete_selected_samples(const SongSelection &sel)
{
	action_manager->group_begin();
	for (Track *t: tracks)
	for (TrackLayer *l: t->layers){
		for (int j=l->samples.num-1; j>=0; j--)
			if (sel.has(l->samples[j]))
				l->delete_sample_ref(l->samples[j]);
	}
	action_manager->group_end();
}

/*Song::Layer *Song::addLayer(const string &name, int index)
{
	return (Song::Layer*)execute(new ActionLayerAdd(name, index));
}

void Song::deleteLayer(int index)
{
	if (layers.num < 2)
		throw Exception(_("At least one layer has to exist."));
	execute(new ActionLayerDelete(index));
}

void Song::mergeLayers(int source, int target)
{
	if (layers.num < 2)
		throw Exception(_("At least one layer has to exist."));
	if (source == target)
		throw Exception(_("Can't merge a layer with itself."));
	execute(new ActionLayerMerge(source, target));
}

void Song::moveLayer(int source, int target)
{
	execute(new ActionLayerMove(source, target));
}

void Song::renameLayer(int index, const string &name)
{
	execute(new ActionLayerRename(index, name));
}*/

void Song::delete_track(Track *track)
{
	if (tracks.num < 2)
		throw Exception(_("At least one layer has to exist."));
	execute(new ActionTrackDelete(track));
}

Sample *Song::add_sample(const string &name, AudioBuffer &buf)
{
	return (Sample*)execute(new ActionSampleAdd(name, buf, false));
}

void Song::delete_sample(Sample *s)
{
	if (s->ref_count > 0)
		throw Exception(_("Can only delete samples which are unused."));
	execute(new ActionSampleDelete(s));
}

void Song::edit_sample_name(Sample *s, const string &name)
{
	execute(new ActionSampleEditName(s, name));
}

void Song::scale_sample(Sample *s, int new_size, int method)
{
	execute(new ActionSampleScale(s, new_size, method));
}

void Song::delete_selection(const SongSelection &sel)
{
	execute(new ActionSongDeleteSelection(sel));
}

void Song::create_samples_from_selection(const SongSelection &sel, bool auto_delete)
{
	if (!sel.range.empty())
		execute(new ActionTrackSampleFromSelection(sel, auto_delete));
}

void Song::add_bar(int index, const BarPattern &b, int mode)
{
	if (index >= 0)
		execute(new ActionBarAdd(index, b, mode));
	else
		execute(new ActionBarAdd(bars.num, b, mode));
}

void Song::add_pause(int index, int length, int mode)
{
	if (index >= 0)
		execute(new ActionBarAdd(index, BarPattern(length, 0, 0), mode));
	else
		execute(new ActionBarAdd(bars.num, BarPattern(length, 0, 0), mode));
}

void Song::edit_bar(int index, const BarPattern &b, int mode)
{
	execute(new ActionBarEdit(index, b, mode));
}

void Song::delete_bar(int index, bool affect_midi)
{
	execute(new ActionBarDelete(index, affect_midi));
}

void Song::delete_time_interval(int index, const Range &range)
{
	//execute(new ActionBarDelete(index, affect_midi));
}

Curve *Song::add_curve(const string &name, Array<Curve::Target> &targets)
{
	Curve *c = new Curve;
	c->name = name;
	c->targets = targets;
	execute(new ActionCurveAdd(c, curves.num));
	return c;
}
void Song::delete_curve(Curve *curve)
{
	foreachi(auto c, curves, i)
		if (c == curve)
			execute(new ActionCurveDelete(i));
}

void Song::edit_curve(Curve *curve, const string &name, float min, float max)
{
	execute(new ActionCurveEdit(curve, name, min, max, curve->targets));
}

void Song::curve_set_targets(Curve *curve, Array<Curve::Target> &targets)
{
	execute(new ActionCurveEdit(curve, curve->name, curve->min, curve->max, targets));
}

void Song::curve_add_point(Curve *curve, int pos, float value)
{
	execute(new ActionCurveAddPoint(curve, pos, value));
}

void Song::curve_delete_point(Curve *curve, int index)
{
	execute(new ActionCurveDeletePoint(curve, index));
}

void Song::curve_edit_point(Curve *curve, int index, int pos, float value)
{
	execute(new ActionCurveEditPoint(curve, index, pos, value));
}

void Song::invalidate_all_peaks()
{
	for (Track *t: tracks)
		t->invalidate_all_peaks();
	for (Sample *s: samples)
		s->buf.peaks.clear();
}


Sample* Song::get_sample_by_uid(int uid)
{
	for (Sample *s: samples)
		if (s->uid == uid)
			return s;
	return nullptr;
}

Track *Song::time_track()
{
	for (Track *t: tracks)
		if (t->type == SignalType::BEATS)
			return t;
	return nullptr;
}

Array<TrackMarker*> Song::get_parts()
{
	Track *t = time_track();
	if (!t)
		return {};
	return t->markers_sorted();
}

string Song::get_tag(const string &key)
{
	for (Tag &t: tags)
		if (t.key == key)
			return t.value;
	return "";
}

Array<TrackLayer*> Song::layers() const
{
	Array<TrackLayer*> layers;
	for (Track *t: tracks)
		layers.append(t->layers);
	return layers;

}
