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
class OutputStream;
class BufferStreamer;
class SampleManagerItem;
class Progress;
class Session;

class SampleManagerConsole : public SideBarConsole
{
public:
	SampleManagerConsole(Session *session);
	virtual ~SampleManagerConsole();

	void update_list();

	void on_list_select();
	void on_list_edit();
	void on_import();
	void on_export();
	void on_preview();
	void on_insert();
	void on_create_from_selection();
	void on_delete();
	void on_scale();

	void on_edit_song();

	void on_progress_cancel();
	void on_song_update();
	void on_preview_stream_update();
	void on_preview_stream_end();

	void end_preview();

	Array<SampleManagerItem*> items;
	Array<SampleManagerItem*> old_items;
	void add(SampleManagerItem *item);
	void remove(SampleManagerItem *item);
	int get_index(Sample *s);
	Array<Sample*> get_selected();

	void set_selection(const Array<Sample*> &samples);

	OutputStream *preview_stream;
	BufferStreamer *preview_renderer;
	Sample *preview_sample;

	Progress *progress;

	static Sample *_cdecl select(Session *session, hui::Panel *parent, Sample *old);
};

#endif /* SAMPLEMANAGERCONSOLE_H_ */
