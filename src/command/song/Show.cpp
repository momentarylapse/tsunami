#include "Show.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../module/synthesizer/Synthesizer.h"
#include "../../module/audio/AudioEffect.h"
#include "../../module/midi/MidiEffect.h"
#include "../../lib/os/msg.h"

void show_song(Song *song) {
	msg_write(format("  sample-rate: %d", song->sample_rate));
	msg_write(format("  samples: %d", song->range().length));
	msg_write("  length: " + song->get_time_str(song->range().length));
	msg_write(format("  bars: %d", song->bars.num));
	msg_write(format("  tracks: %d", song->tracks.num));
	int n = 0;
	for (Track *t: weak(song->tracks)) {
		msg_write(format("  track '%s'", t->nice_name()));
		msg_write("    type: " + signal_type_name(t->type));
		if (t->type == SignalType::Midi) {
			msg_write(format("    synth: %s v%d", t->synth->module_class, t->synth->version()));
		}
		for (TrackLayer *l: weak(t->layers)) {
			msg_write("    layer");
			if (l->buffers.num > 0)
				msg_write(format("      buffers: %d", l->buffers.num));
			if (l->midi.num > 0)
				msg_write(format("      notes: %d", l->midi.num));
			if (l->samples.num > 0)
				msg_write(format("      sample-refs: %d", l->samples.num));
			if (l->markers.num > 0)
				msg_write(format("      markers: %d", l->markers.num));
			n += l->samples.num;
		}
		for (auto *fx: weak(t->fx))
			msg_write(format("    fx: %s v%d", fx->module_class, fx->version()));
		for (auto *fx: weak(t->midi_fx))
			msg_write(format("    midifx: %s v%d", fx->module_class, fx->version()));
	}
	msg_write(format("  refs: %d / %d", n, song->samples.num));
	for (Tag &t: song->tags)
		msg_write(format("  tag: %s = '%s'", t.key, t.value));
}
