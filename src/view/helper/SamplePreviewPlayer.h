//
// Created by Michael Ankele on 2024-12-16.
//

#ifndef SAMPLEPREVIEWPLAYER_H
#define SAMPLEPREVIEWPLAYER_H

#include "../../data/Sample.h"
#include "../../lib/base/pointer.h"
#include "../../lib/pattern/Observable.h"

namespace hui {
	class Window;
}

namespace tsunami {

class SignalChain;
class AudioOutput;
class BufferStreamer;
class MidiEventStreamer;
class Progress;
class Sample;
class Session;

class SamplePreviewPlayer : public obs::Node<VirtualBase>{
public:
	explicit SamplePreviewPlayer(hui::Window* win, Session* session, Sample* sample);
	~SamplePreviewPlayer() override;

	shared<SignalChain> chain;
	AudioOutput *stream = nullptr;
	BufferStreamer *renderer = nullptr;
	MidiEventStreamer *midi_streamer = nullptr;
	Sample *sample = nullptr;
	owned<Progress> progress;

	void on_progress_cancel();
	void on_preview_tick();
	void on_preview_stream_end();

	void end_preview();

	static void play(hui::Window* win, Session* session, Sample* sample);
};

} // tsunami

#endif //SAMPLEPREVIEWPLAYER_H
