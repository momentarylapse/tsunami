/*
 * FxConsole.h
 *
 *  Created on: 20.03.2014
 *      Author: michi
 */

#ifndef FXCONSOLE_H_
#define FXCONSOLE_H_

#include "../../lib/hui/hui.h"
#include "../../Stuff/Observer.h"

class Track;
class AudioFile;

class FxConsole : public HuiPanel, public Observer, public Observable
{
public:
	FxConsole(AudioFile *audio);
	virtual ~FxConsole();

	void Clear();
	void SetTrack(Track *t);
	void Show(bool show);

	void OnAdd();
	void OnClose();

	virtual void OnUpdate(Observable *o, const string &message);

	string id_inner;

	Track *track;
	AudioFile *audio;
	Array<HuiPanel*> panels;
	bool enabled;
};

#endif /* FXCONSOLE_H_ */
