/*
 * SampleManagerConsole.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef SAMPLEMANAGERCONSOLE_H_
#define SAMPLEMANAGERCONSOLE_H_

#include "SideBar.h"

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
	SampleManagerConsole(Session *session);
	virtual ~SampleManagerConsole();

	void update_list();

	void on_list_select();
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

	void on_edit_song();

	void on_progress_cancel();
	void on_song_update();
	void on_preview_tick();
	void on_preview_stream_end();

	void end_preview();

	Array<SampleManagerItem*> items;
	Array<SampleManagerItem*> old_items;
	void add(SampleManagerItem *item);
	void remove(SampleManagerItem *item);
	int get_index(Sample *s);
	Array<Sample*> get_selected();

	void set_selection(const Array<Sample*> &samples);

	struct Preview {
		SignalChain *chain = nullptr;
		AudioOutput *stream = nullptr;
		BufferStreamer *renderer = nullptr;
		MidiEventStreamer *midi_streamer = nullptr;
		Sample *sample = nullptr;
	} preview;

	Progress *progress;
	
	string id_list;
	hui::Menu *menu_samples;

	static Sample *_cdecl select(Session *session, hui::Panel *parent, Sample *old);
};

#endif /* SAMPLEMANAGERCONSOLE_H_ */
