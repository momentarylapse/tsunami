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

class SampleManager : public BottomBarConsole, public Observer
{
public:
	SampleManager(AudioFile *a);
	virtual ~SampleManager();

	void fillList();

	void onListSelect();
	void onListEdit();
	void onImportFromFile();
	void onInsert();
	void onCreateFromSelection();
	void onDelete();

	virtual void onUpdate(Observable *o, const string &message);

	AudioFile *audio;
	Array<string> icon_names;
	int selected_uid;

	static Sample *_cdecl select(HuiPanel *root, AudioFile *a, Sample *old);
};

#endif /* SAMPLEMANAGER_H_ */
