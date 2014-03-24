/*
 * BottomBar.h
 *
 *  Created on: 23.03.2014
 *      Author: michi
 */

#ifndef BOTTOMBAR_H_
#define BOTTOMBAR_H_

#include "../../lib/hui/hui.h"
#include "../../Stuff/Observable.h"

class AudioFile;
class AudioOutput;
class Track;
class FxConsole;
class MixingConsole;

class BottomBarConsole : public HuiPanel
{
public:
	string title;
};

class BottomBar : public HuiPanel, public Observable
{
public:
	BottomBar(AudioFile *audio, AudioOutput *output);
	virtual ~BottomBar();

	void OnClose();
	void OnNext();
	virtual void OnShow();
	virtual void OnHide();

	enum
	{
		MIXING_CONSOLE,
		FX_CONSOLE,
	};

	void SetTrack(Track *t);
	void Choose(int console);
	bool IsActive(int console);
	int active_console;
	bool visible;

	FxConsole *fx_console;
	MixingConsole *mixing_console;
};

#endif /* BOTTOMBAR_H_ */
