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
#include "base.h"
#include "Song.h"
#include "Audio/AudioBuffer.h"
#include "../Action/Track/Buffer/ActionTrackSetChannels.h"
#include "../Action/Track/Data/ActionTrackEditName.h"
#include "../Action/Track/Data/ActionTrackEditMuted.h"
#include "../Action/Track/Data/ActionTrackEditVolume.h"
#include "../Action/Track/Data/ActionTrackEditPanning.h"
#include "../Action/Track/Data/ActionTrackSetInstrument.h"
#include "../Action/Track/Layer/ActionTrackLayerAdd.h"
#include "../Action/Track/Layer/ActionTrackLayerDelete.h"
#include "../Action/Track/Layer/ActionTrackLayerMerge.h"
//#include "../Action/Track/Layer/ActionTrackLayerMove.h"
#include "../Action/Track/Midi/ActionTrackAddMidiEffect.h"
#include "../Action/Track/Midi/ActionTrackDeleteMidiEffect.h"
#include "../Action/Track/Midi/ActionTrackEditMidiEffect.h"
#include "../Action/Track/Midi/ActionTrackToggleMidiEffectEnabled.h"
#include "../Action/Track/ActionTrackMove.h"
#include "../Action/Track/Synthesizer/ActionTrackSetSynthesizer.h"
#include "../Action/Track/Synthesizer/ActionTrackEditSynthesizer.h"
#include "../Action/Track/Synthesizer/ActionTrackDetuneSynthesizer.h"
#include "../Action/Track/Effect/ActionTrackAddEffect.h"
#include "../Action/Track/Effect/ActionTrackDeleteAudioEffect.h"
#include "../Action/Track/Effect/ActionTrackEditEffect.h"
#include "../Action/Track/Effect/ActionTrackMoveAudioEffect.h"
#include "../Action/Track/Effect/ActionTrackToggleEffectEnabled.h"
#include "../Action/Track/Marker/ActionTrackAddMarker.h"
#include "../Action/Track/Marker/ActionTrackDeleteMarker.h"
#include "../Action/Track/Marker/ActionTrackEditMarker.h"
#include "../Module/Synth/Synthesizer.h"
#include "../Module/Audio/AudioEffect.h"
#include "../Plugins/PluginManager.h"
//#include "../Tsunami.h"
#include "../lib/hui/hui.h"
#include "../lib/threads/Mutex.h"


const string Track::MESSAGE_ADD_EFFECT = "AddEffect";
const string Track::MESSAGE_DELETE_EFFECT = "DeleteEffect";
const string Track::MESSAGE_ADD_MIDI_EFFECT = "AddMidiEffect";
const string Track::MESSAGE_DELETE_MIDI_EFFECT = "DeleteMidiEffect";
const string Track::MESSAGE_REPLACE_SYNTHESIZER = "ReplaceSynthesizer";

Track::Track(SignalType _type, Synthesizer *_synth)
{
	type = _type;
	if (type == SignalType::AUDIO_MONO){
		type = SignalType::AUDIO;
		channels = 1;
	}else if (type == SignalType::AUDIO_STEREO){
		type = SignalType::AUDIO;
		channels = 2;
	}else if (type == SignalType::AUDIO){
		channels = 1;
	}
	muted = false;
	volume = 1;
	panning = 0;
	song = nullptr;

	volume = 1;
	muted = false;

	synth = _synth;

	layers.add(new TrackLayer(this));
}


Track::~Track()
{
	for (TrackLayer *l: layers)
		delete(l);
	layers.clear();

	for (AudioEffect *f: fx)
		delete(f);
	fx.clear();

	for (MidiEffect *f: midi_fx)
		delete(f);
	midi_fx.clear();

	if (synth)
		delete(synth);
}

Range Track::range() const
{
	Range r = Range::EMPTY;

	for (TrackLayer *l: layers)
		r = r or l->range(synth->keep_notes);

	for (TrackMarker *m: markers)
		r = r or m->range;

	return r;
}

string Track::nice_name()
{
	if (name.num > 0)
		return name;
	if (type == SignalType::BEATS)
		return _("Metronome");
	int n = get_track_index(this);
	return format(_("Track %d"), n+1);
}

int Track::get_index()
{
	//assert(song);
	return song->tracks.find(this);
}

void Track::invalidate_all_peaks()
{
	for (TrackLayer *l: layers)
		for (AudioBuffer &b: l->buffers)
			b.peaks.clear();
}

bool Track::has_version_selection()
{
	return fades.num > 0;
}

void Track::set_name(const string& name)
{
	song->execute(new ActionTrackEditName(this, name));
}

void Track::set_instrument(const Instrument& instrument)
{
	song->execute(new ActionTrackSetInstrument(this, instrument));
}

void Track::set_muted(bool muted)
{
	song->execute(new ActionTrackEditMuted(this, muted));
}

void Track::set_volume(float volume)
{
	song->execute(new ActionTrackEditVolume(this, volume));
}

void Track::set_panning(float panning)
{
	song->execute(new ActionTrackEditPanning(this, panning));
}

void Track::move(int target)
{
	if (target != get_index())
		song->execute(new ActionTrackMove(this, target));
}
void Track::set_channels(int _channels)
{
	if (channels != _channels)
		song->execute(new ActionTrackSetChannels(this, _channels));
}

void Track::add_effect(AudioEffect *effect)
{
	song->execute(new ActionTrackAddEffect(this, effect));
}

// execute after editing...
void Track::edit_effect(AudioEffect *effect, const string &param_old)
{
	song->execute(new ActionTrackEditEffect(effect, param_old));
}

void Track::enable_effect(AudioEffect *effect, bool enabled)
{
	if (effect->enabled != enabled)
		song->execute(new ActionTrackToggleEffectEnabled(effect));
}

void Track::delete_effect(AudioEffect *effect)
{
	foreachi(AudioEffect *f, fx, index){
		if (f == effect)
			song->execute(new ActionTrackDeleteEffect(this, index));
	}
}

void Track::move_effect(int source, int target)
{
	if (source != target)
		song->execute(new ActionTrackMoveAudioEffect(this, source, target));
}

void Track::add_midi_effect(MidiEffect *effect)
{
	song->execute(new ActionTrackAddMidiEffect(this, effect));
}

// execute after editing...
void Track::edit_midi_effect(MidiEffect *effect, const string &param_old)
{
	song->execute(new ActionTrackEditMidiEffect(effect, param_old));
}

void Track::enable_midi_effect(MidiEffect *effect, bool enabled)
{
	if (effect->enabled != enabled)
		song->execute(new ActionTrackToggleMidiEffectEnabled(effect));
}

void Track::delete_midi_effect(MidiEffect *effect)
{
	foreachi(MidiEffect *f, midi_fx, index)
		if (f == effect)
			song->execute(new ActionTrackDeleteMidiEffect(this, index));
}

void Track::set_synthesizer(Synthesizer *_synth)
{
	song->execute(new ActionTrackSetSynthesizer(this, _synth));
}

// execute after editing...
void Track::edit_synthesizer(const string &param_old)
{
	song->execute(new ActionTrackEditSynthesizer(this, param_old));
}

void Track::detune_synthesizer(int pitch, float dpitch, bool all_octaves)
{
	song->execute(new ActionTrackDetuneSynthesizer(this, pitch, dpitch, all_octaves));
}

TrackMarker *Track::add_marker(const Range &range, const string &text)
{
	return (TrackMarker*)song->execute(new ActionTrackAddMarker(this, range, text));
}

void Track::delete_marker(const TrackMarker *marker)
{
	foreachi(const TrackMarker *m, markers, index)
		if (m == marker)
			song->execute(new ActionTrackDeleteMarker(this, index));
}

void Track::edit_marker(const TrackMarker *marker, const Range &range, const string &text)
{
	song->execute(new ActionTrackEditMarker((TrackMarker*)marker, range, text));
}

TrackLayer *Track::add_layer()
{
	return (TrackLayer*)song->execute(new ActionTrackLayerAdd(this, new TrackLayer(this)));
}

void Track::delete_layer(TrackLayer *layer)
{
	song->execute(new ActionTrackLayerDelete(this, layer->version_number()));
}

void Track::merge_layers()
{
	song->execute(new ActionTrackLayerMerge(this));
}


