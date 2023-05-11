/*
 * Track.cpp
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#include "Track.h"
#include "TrackLayer.h"
#include "TrackMarker.h"
#include "CrossFade.h"
#include "Curve.h"
#include "base.h"
#include "Song.h"
#include "audio/AudioBuffer.h"
#include "../action/track/buffer/ActionTrackSetChannels.h"
#include "../action/track/curve/ActionTrackAddCurve.h"
#include "../action/track/curve/ActionTrackCurveAddPoint.h"
#include "../action/track/curve/ActionTrackCurveDeletePoint.h"
#include "../action/track/curve/ActionTrackCurveEditPoint.h"
#include "../action/track/curve/ActionTrackDeleteCurve.h"
#include "../action/track/curve/ActionTrackEditCurve.h"
#include "../action/track/data/ActionTrackEditName.h"
#include "../action/track/data/ActionTrackEditMuted.h"
#include "../action/track/data/ActionTrackEditVolume.h"
#include "../action/track/data/ActionTrackEditPanning.h"
#include "../action/track/data/ActionTrackSetInstrument.h"
#include "../action/track/data/ActionTrackSetTarget.h"
#include "../action/track/layer/ActionTrackLayerAdd.h"
#include "../action/track/layer/ActionTrackLayerDelete.h"
#include "../action/track/layer/ActionTrackLayerMerge.h"
//#include "../Action/Track/Layer/ActionTrackLayerMove.h"
#include "../action/track/layer/ActionTrackLayerMarkDominant.h"
#include "../action/track/midi/ActionTrackAddMidiEffect.h"
#include "../action/track/midi/ActionTrackDeleteMidiEffect.h"
#include "../action/track/midi/ActionTrackEditMidiEffect.h"
#include "../action/track/midi/ActionTrackMoveMidiEffect.h"
#include "../action/track/midi/ActionTrackToggleMidiEffectEnabled.h"
#include "../action/track/ActionTrackMove.h"
#include "../action/track/synthesizer/ActionTrackSetSynthesizer.h"
#include "../action/track/synthesizer/ActionTrackEditSynthesizer.h"
#include "../action/track/synthesizer/ActionTrackDetuneSynthesizer.h"
#include "../action/track/effect/ActionTrackAddEffect.h"
#include "../action/track/effect/ActionTrackDeleteAudioEffect.h"
#include "../action/track/effect/ActionTrackEditEffect.h"
#include "../action/track/effect/ActionTrackMoveAudioEffect.h"
#include "../action/track/effect/ActionTrackToggleEffectEnabled.h"
#include "../module/synthesizer/Synthesizer.h"
#include "../module/audio/AudioEffect.h"
#include "../plugins/PluginManager.h"
//#include "../Tsunami.h"
#include "../lib/base/iter.h"
#include "../lib/base/algo.h"
#include "../lib/hui/language.h"
#include "../lib/threads/Mutex.h"


Track::Track(Song *_song, SignalType _type, Synthesizer *_synth) {
	//msg_write("  new Track " + p2s(this));
	type = _type;
	song = _song;
	channels = 1;
	if (type == SignalType::AUDIO_MONO) {
		type = SignalType::AUDIO;
		channels = 1;
	} else if (type == SignalType::AUDIO_STEREO) {
		type = SignalType::AUDIO;
		channels = 2;
	} else if (type == SignalType::AUDIO) {
		channels = 1;
	}
	muted = false;
	volume = 1;
	panning = 0;
	send_target = nullptr;

	volume = 1;
	muted = false;

	_register_synth(_synth);
	synth = _synth;
}


Track::~Track() {
	out_death.notify();
	//msg_write("  del Track " + p2s(this));
}

Range Track::range() const {
	Range r = Range::NONE;

	for (auto *l: weak(layers))
		r = r or l->range(synth.get()->keep_notes);

	return r;
}

int get_same_type_index(const Track *t) {
	if (!t->song)
		return -1;
	int n = 0;
	for (auto&& [i,tt]: enumerate(weak(t->song->tracks)))
		if (tt->type == t->type) {
			if (tt == t)
				return n;
			n ++;
		}
	return -1;
}

string track_base_name(SignalType type) {
	if (type == SignalType::BEATS)
		return _("Metronome");
	if (type == SignalType::GROUP)
		return _("Master");
	if (type == SignalType::AUDIO)
		return _("Audio");
	if (type == SignalType::MIDI)
		return _("Midi");
	return _("Track");
}

string Track::nice_name() const {
	if (name.num > 0)
		return name;
	int n = get_same_type_index(this);
	string base = track_base_name(type);
	if ((n == 0) and ((type == SignalType::BEATS) or (type == SignalType::GROUP)))
		return base;
	return base + format(" %d", n+1);
	//int n = get_track_index(this);
	//return format(_("Track %d"), n+1);
}

int Track::get_index() const {
	//assert(song);
	return weak(song->tracks).find(const_cast<Track*>(this));
}

void Track::invalidate_all_peaks() {
	for (TrackLayer *l: weak(layers))
		for (AudioBuffer &b: l->buffers)
			b.peaks.clear();
}



bool Track::has_version_selection() const {
	for (auto *l: weak(layers))
		if (l->fades.num > 0)
			return true;
	return false;
}

void Track::set_name(const string& name) {
	song->execute(new ActionTrackEditName(this, name));
}

void Track::set_instrument(const Instrument& instrument) {
	song->execute(new ActionTrackSetInstrument(this, instrument));
}

void Track::set_muted(bool muted) {
	song->execute(new ActionTrackEditMuted(this, muted));
}

void Track::set_volume(float volume) {
	song->execute(new ActionTrackEditVolume(this, volume));
}

void Track::set_panning(float panning) {
	song->execute(new ActionTrackEditPanning(this, panning));
}

void Track::set_send_target(Track *target) {
	song->execute(new ActionTrackSetTarget(this, target));
}

void Track::move(int target) {
	if (target != get_index())
		song->execute(new ActionTrackMove(this, target));
}
void Track::set_channels(int _channels) {
	if (channels != _channels)
		song->execute(new ActionTrackSetChannels(this, _channels));
}

void Track::add_effect(shared<AudioEffect> effect) {
	song->execute(new ActionTrackAddEffect(this, effect));
	_register_fx(effect.get());
}


void Track::_register_fx(AudioEffect *fx) {
	fx->set_func_edit([this, fx] { edit_effect(fx); });
}

void Track::_register_midi_fx(MidiEffect *fx) {
	fx->set_func_edit([this, fx] { edit_midi_effect(fx); });
}

void Track::_register_synth(Synthesizer *s) {
	s->set_func_edit([this, s] { edit_synthesizer(); });
}

// execute after editing...
void Track::edit_effect(AudioEffect *effect) {
	song->execute(new ActionTrackEditEffect(effect));
}

void Track::enable_effect(AudioEffect *effect, bool enabled) {
	if (effect->enabled != enabled)
		song->execute(new ActionTrackToggleEffectEnabled(effect));
}

void Track::delete_effect(AudioEffect *effect) {
	int index = base::find_index(weak(fx), effect);
	if (index >= 0)
		song->execute(new ActionTrackDeleteEffect(this, index));
}

void Track::move_effect(int source, int target) {
	if (source != target)
		song->execute(new ActionTrackMoveAudioEffect(this, source, target));
}

void Track::add_midi_effect(shared<MidiEffect> effect) {
	song->execute(new ActionTrackAddMidiEffect(this, effect));
	_register_midi_fx(effect.get());
}

// execute after editing...
void Track::edit_midi_effect(MidiEffect *effect) {
	song->execute(new ActionTrackEditMidiEffect(effect));
}

void Track::enable_midi_effect(MidiEffect *effect, bool enabled) {
	if (effect->enabled != enabled)
		song->execute(new ActionTrackToggleMidiEffectEnabled(effect));
}

void Track::delete_midi_effect(MidiEffect *effect) {
	int index = base::find_index(weak(midi_fx), effect);
	if (index >= 0)
		song->execute(new ActionTrackDeleteMidiEffect(this, index));
}

void Track::move_midi_effect(int source, int target) {
	if (source != target)
		song->execute(new ActionTrackMoveMidiEffect(this, source, target));
}

void Track::set_synthesizer(shared<Synthesizer> _synth) {
	song->execute(new ActionTrackSetSynthesizer(this, _synth));
	_register_synth(_synth.get());
}

// execute after editing...
void Track::edit_synthesizer() {
	song->execute(new ActionTrackEditSynthesizer(this));
}

void Track::detune_synthesizer(const Temperament &temperament) {
	song->execute(new ActionTrackDetuneSynthesizer(this, temperament));
}

TrackLayer *Track::add_layer() {
	auto *layer = new TrackLayer(this);
	song->execute(new ActionTrackLayerAdd(this, layer));
	return layer;
}

void Track::delete_layer(TrackLayer *layer) {
	song->execute(new ActionTrackLayerDelete(this, layer->version_number()));
}

void Track::merge_layers() {
	song->execute(new ActionTrackLayerMerge(this));
}

void Track::mark_dominant(const Array<const TrackLayer*> &layers, const Range &range) {
	song->execute(new ActionTrackLayerMarkDominant(this, layers, range));
}

Curve *Track::add_curve(const string &name, CurveTarget &target) {
	auto c = new Curve;
	c->name = name;
	c->target = target;
	song->execute(new ActionTrackAddCurve(this, c, curves.num));
	return c;
}
void Track::delete_curve(Curve *curve) {
	int index = base::find_index(weak(curves), curve);
	if (index >= 0)
		song->execute(new ActionTrackDeleteCurve(this, index));
}

void Track::edit_curve(Curve *curve, const string &name, float min, float max, CurveType type) {
	song->execute(new ActionTrackEditCurve(this, curve, name, min, max, type));
}

void Track::curve_add_point(Curve *curve, int pos, float value) {
	song->execute(new ActionTrackCurveAddPoint(curve, pos, value));
}

void Track::curve_delete_point(Curve *curve, int index) {
	song->execute(new ActionTrackCurveDeletePoint(curve, index));
}

void Track::curve_edit_point(Curve *curve, int index, int pos, float value) {
	song->execute(new ActionTrackCurveEditPoint(curve, index, pos, value));
}


