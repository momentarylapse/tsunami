/*
 * Tsunami.h
 *
 *  Created on: 13.08.2014
 *      Author: michi
 */

#ifndef TSUNAMI_H_
#define TSUNAMI_H_

#include "lib/hui/hui.h"

extern string AppName;
extern string AppVersion;


class AudioFile;
class PluginManager;
class Slider;
class Log;
class AudioInput;
class AudioOutput;
class AudioRenderer;
class AudioView;
class Storage;
class Clipboard;
class TsunamiWindow;

class Tsunami : public HuiApplication
{
public:
	Tsunami();
	virtual ~Tsunami();

	virtual bool onStartup(const Array<string> &arg);

	Array<string> _arg;
	void _HandleArguments();

	bool HandleArguments(const Array<string> &arg);
	void LoadKeyCodes();

	void CreateWindow();
	bool AllowTermination();

	TsunamiWindow *win;
	HuiWindow *_win;
	AudioView *_view;


	AudioFile *audio;

	Storage *storage;

	Log *log;

	AudioOutput *output;
	AudioInput *input;

	PluginManager *plugin_manager;
	Clipboard *clipboard;
};


extern Tsunami *tsunami;

#endif /* TSUNAMI_H_ */
