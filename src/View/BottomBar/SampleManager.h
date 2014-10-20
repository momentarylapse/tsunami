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

	void FillList();

	void OnListSelect();
	void OnListEdit();
	void OnImportFromFile();
	void OnInsert();
	void OnCreateFromSelection();
	void OnDelete();

	virtual void OnUpdate(Observable *o, const string &message);

	AudioFile *audio;
	Array<string> icon_names;
	int selected_uid;

	static Sample *_cdecl Select(HuiPanel *root, AudioFile *a, Sample *old);
};

#endif /* SAMPLEMANAGER_H_ */
