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
class BufferRenderer;
class SampleManagerItem;
class Progress;
class AudioView;

class SampleManagerConsole : public SideBarConsole, public Observer
{
public:
	SampleManagerConsole(Song *a, AudioView *v);
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

	virtual void onUpdate(Observable *o, const string &message);

	void endPreview();

	Song *song;
	AudioView *view;
	Array<SampleManagerItem*> items;
	Array<SampleManagerItem*> old_items;
	void add(SampleManagerItem *item);
	void remove(SampleManagerItem *item);
	int getIndex(Sample *s);
	Array<Sample*> getSelected();

	void setSelection(const Array<Sample*> &samples);

	OutputStream *preview_stream;
	BufferRenderer *preview_renderer;
	Sample *preview_sample;

	Progress *progress;

	static Sample *_cdecl select(hui::Panel *root, Song *a, Sample *old);
};

#endif /* SAMPLEMANAGERCONSOLE_H_ */
