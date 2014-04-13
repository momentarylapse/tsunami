/*
 * SynthConsole.h
 *
 *  Created on: 13.04.2014
 *      Author: michi
 */

#ifndef SYNTHCONSOLE_H_
#define SYNTHCONSOLE_H_

#include "BottomBar.h"
#include "../../Stuff/Observer.h"

class Track;
class AudioFile;
class AudioView;

class SynthConsole : public BottomBarConsole, public Observer
{
public:
	SynthConsole(AudioView *view, AudioFile *audio);
	virtual ~SynthConsole();

	void Clear();
	void SetTrack(Track *t);

	virtual void OnUpdate(Observable *o, const string &message);

	string id_inner;

	HuiPanel *panel;

	AudioView *view;
	Track *track;
	AudioFile *audio;
};

#endif /* SYNTHCONSOLE_H_ */
