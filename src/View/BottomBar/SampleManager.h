/*
 * SampleManager.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef SAMPLEMANAGER_H_
#define SAMPLEMANAGER_H_

#include "BottomBar.h"

class AudioFile;
class Sample;
class AudioStream;
class AudioRenderer;
class SampleManagerItem;

class SampleManager : public BottomBarConsole, public Observer
{
public:
	SampleManager(AudioFile *a);
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

	virtual void onUpdate(Observable *o, const string &message);

	void endPreview();

	AudioFile *audio;
	Array<SampleManagerItem*> items;
	Array<SampleManagerItem*> old_items;
	void add(SampleManagerItem *item);
	void remove(SampleManagerItem *item);
	int getIndex(Sample *s);
	int selected_uid;

	AudioStream *preview_stream;
	AudioRenderer *preview_renderer;
	AudioFile *preview_audio;
	Sample *preview_sample;

	static Sample *_cdecl select(HuiPanel *root, AudioFile *a, Sample *old);
};

#endif /* SAMPLEMANAGER_H_ */
