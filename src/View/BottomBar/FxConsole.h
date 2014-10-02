/*
 * FxConsole.h
 *
 *  Created on: 20.03.2014
 *      Author: michi
 */

#ifndef FXCONSOLE_H_
#define FXCONSOLE_H_

#include "BottomBar.h"
#include "../../Stuff/Observer.h"

class Track;
class AudioFile;
class AudioView;

class FxConsole : public BottomBarConsole, public Observer
{
public:
	FxConsole(AudioView *view, AudioFile *audio);
	virtual ~FxConsole();

	void Clear();
	void SetTrack(Track *t);

	void OnAdd();

	virtual void OnUpdate(Observable *o, const string &message);

	string id_inner;

	AudioView *view;
	Track *track;
	AudioFile *audio;
	Array<HuiPanel*> panels;
};

#endif /* FXCONSOLE_H_ */
