#include "Export.h"
#include "../../data/Song.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../data/TrackMarker.h"
#include "../../data/SongSelection.h"
#include "../../data/CrossFade.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../module/audio/AudioEffect.h"
#include "../../module/midi/MidiEffect.h"
#include "../../module/synthesizer/Synthesizer.h"
#include "../../storage/Storage.h"
#include "../../Session.h"

namespace tsunami {

shared<Song> copy_song_from_selection(Song *song, const SongSelection &sel) {
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
		Track *tt = new Track(ss, t->type, (Synthesizer*)t->synth->copy());
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
			ll->muted = l->muted;
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
			for (auto f: l->fades)
				ll->fades.add({f.position - sel.range().offset, f.mode, f.samples});
		}
	}
	return ss;
}

void unmute_all(Song *s) {
	for (auto t: weak(s->tracks)) {
		t->set_muted(false);
		for (auto l: weak(t->layers))
			l->set_muted(false);
	}
}

base::future<void> export_selection(Song *song, const SongSelection& sel, const Path& filename, bool force_unmute) {
	auto s = copy_song_from_selection(song, sel);
	if (force_unmute)
		unmute_all(s.get());
	return song->session->storage->save(s.get(), filename);
}

}

