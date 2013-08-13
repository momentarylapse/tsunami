/*
 * SampleManager.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef SAMPLEMANAGER_H_
#define SAMPLEMANAGER_H_

#include "../../lib/hui/hui.h"
#include "../../Stuff/Observer.h"

class AudioFile;
class Image;

class SampleManager : public HuiDialog, public Observer
{
public:
	SampleManager(AudioFile *a, HuiWindow *_parent, bool _allow_parent);
	virtual ~SampleManager();

	void FillList();

	void OnListSelect();
	void OnImportFromFile();
	void OnInsert();
	void OnCreateFromSelection();
	void OnDelete();
	void OnClose();

	virtual void OnUpdate(Observable *o);

	AudioFile *audio;
	Array<string> icon_names;
};

#endif /* SAMPLEMANAGER_H_ */
