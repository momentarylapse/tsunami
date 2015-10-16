/*
 * SampleManager.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef SAMPLEMANAGER_H_
#define SAMPLEMANAGER_H_

#include "SideBar.h"

class Song;
class Sample;
class AudioStream;
class SongRenderer;
class SampleManagerItem;
class Progress;

class SampleManager : public SideBarConsole, public Observer
{
public:
	SampleManager(Song *a);
	virtual ~SampleManager();

	void updateList();

	void onListSelect();
	void onListEdit();
	void onImport();
	void onExport();
	void onPreview();
	void onInsert();
	void onCreateFromSelection();
	void onDelete();

	void onEditSong();

	virtual void onUpdate(Observable *o, const string &message);

	void endPreview();

	Song *song;
	Array<SampleManagerItem*> items;
	Array<SampleManagerItem*> old_items;
	void add(SampleManagerItem *item);
	void remove(SampleManagerItem *item);
	int getIndex(Sample *s);
	int selected_uid;

	AudioStream *preview_stream;
	SongRenderer *preview_renderer;
	Song *preview_audio;
	Sample *preview_sample;

	Progress *progress;

	static Sample *_cdecl select(HuiPanel *root, Song *a, Sample *old);
};

#endif /* SAMPLEMANAGER_H_ */
