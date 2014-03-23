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
class MiniConsole;
class FxConsole;
class MixingConsole;

class BottomBar : public HuiPanel, public Observable
{
public:
	BottomBar(AudioFile *audio, AudioOutput *output);
	virtual ~BottomBar();

	enum
	{
		MINI_CONSOLE,
		FX_CONSOLE,
		MIXING_CONSOLE
	};

	void SetTrack(Track *t);
	void Choose(int console);
	int active_console;

	MiniConsole *mini_console;
	FxConsole *fx_console;
	MixingConsole *mixing_console;
};

#endif /* BOTTOMBAR_H_ */
