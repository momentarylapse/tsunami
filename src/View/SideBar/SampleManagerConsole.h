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

	void updateList();

	void onListSelect();
	void onListEdit();
	void onImport();
	void onExport();
	void onPreview();
	void onInsert();
	void onCreateFromSelection();
	void onDelete();
	void onScale();

	void onEditSong();

	void onProgressCancel();
	void onSongUpdate();
	void onPreviewStreamUpdate();
	void onPreviewStreamEnd();

	void endPreview();

	Array<SampleManagerItem*> items;
	Array<SampleManagerItem*> old_items;
	void add(SampleManagerItem *item);
	void remove(SampleManagerItem *item);
	int getIndex(Sample *s);
	Array<Sample*> getSelected();

	void setSelection(const Array<Sample*> &samples);

	OutputStream *preview_stream;
	BufferStreamer *preview_renderer;
	Sample *preview_sample;

	Progress *progress;

	static Sample *_cdecl select(Session *session, hui::Panel *parent, Sample *old);
};

#endif /* SAMPLEMANAGERCONSOLE_H_ */
