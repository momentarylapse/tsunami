/*
 * MidiPreview.h
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_HELPER_MIDIPREVIEW_H_
#define SRC_VIEW_HELPER_MIDIPREVIEW_H_

#include "../../lib/base/base.h"

class OutputStream;
class Synthesizer;
class MidiPreviewSource;
class SignalChain;
class Session;


class MidiPreview : public VirtualBase
{
public:
	MidiPreview(Session *s);
	virtual ~MidiPreview();

	SignalChain *chain;
	Synthesizer *synth;
	OutputStream *stream;
	MidiPreviewSource *source;
	Session *session;


	void start(Synthesizer *s, const Array<int> &pitch, float volume, float ttl);
	void end();

	void on_end_of_stream();
};

#endif /* SRC_VIEW_HELPER_MIDIPREVIEW_H_ */
