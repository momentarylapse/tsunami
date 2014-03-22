/*
 * FxPanel.h
 *
 *  Created on: 20.03.2014
 *      Author: michi
 */

#ifndef FXPANEL_H_
#define FXPANEL_H_

#include "../../lib/hui/hui.h"
#include "../../Stuff/Observer.h"

class Track;
class AudioFile;

class FxPanel : public HuiPanel, public Observer, public Observable
{
public:
	FxPanel(AudioFile *audio);
	virtual ~FxPanel();

	void Clear();
	void SetTrack(Track *t);
	void Show(bool show);

	void OnAdd();
	void OnClose();

	virtual void OnUpdate(Observable *o);

	string id_inner;

	Track *track;
	AudioFile *audio;
	Array<HuiPanel*> panels;
	bool enabled;
};

#endif /* FXPANEL_H_ */
