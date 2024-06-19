/*
 * SampleManagerConsole.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef SAMPLEMANAGERCONSOLE_H_
#define SAMPLEMANAGERCONSOLE_H_

#include "SideBar.h"

namespace tsunami {

class Song;
class Sample;
class AudioOutput;
class BufferStreamer;
class MidiEventStreamer;
class SampleManagerItem;
class Progress;
class Session;
class SignalChain;

class SampleManagerConsole : public SideBarConsole {
public:
	SampleManagerConsole(Session *session, SideBar *bar);

	obs::sink in_song_update;

	void on_enter() override;
	void on_leave() override;

	void update_list();

	void on_list_edit();
	void on_list_right_click();
	void on_import();
	void on_export();
	void on_preview();
	void on_insert();
	void on_create_from_selection();
	void on_auto_delete();
	void on_delete();
	void on_scale();

	void on_progress_cancel();
	void on_song_update();
	void on_preview_tick();
	void on_preview_stream_end();

	void end_preview();

	owned_array<SampleManagerItem> items;
	Array<SampleManagerItem*> old_items;
	void add(SampleManagerItem *item);
	void remove(SampleManagerItem *item);
	int get_index(Sample *s);
	Array<Sample*> get_selected();

	void set_selection(const Array<Sample*> &samples);

	struct Preview {
		shared<SignalChain> chain;
		AudioOutput *stream = nullptr;
		BufferStreamer *renderer = nullptr;
		MidiEventStreamer *midi_streamer = nullptr;
		Sample *sample = nullptr;
	} preview;

	owned<Progress> progress;
	
	string id_list;
	owned<hui::Menu> menu_samples;
};

}

#endif /* SAMPLEMANAGERCONSOLE_H_ */
