/*
 * Tsunami.h
 *
 *  Created on: 13.08.2014
 *      Author: michi
 */

#ifndef TSUNAMI_H_
#define TSUNAMI_H_

#include "lib/hui/hui.h"


class AudioFile;
class PluginManager;
class Slider;
class Log;
class AudioInput;
class AudioOutput;
class AudioRenderer;
class AudioView;
class Storage;
class Progress;
class Clipboard;
class TsunamiWindow;

class Tsunami : public HuiApplication
{
public:
	Tsunami(Array<string> arg);
	virtual ~Tsunami();

	virtual bool onStartup(Array<string> arg);

	Array<string> _arg;
	void _HandleArguments();

	bool HandleArguments(Array<string> arg);
	void LoadKeyCodes();

	void CreateWindow();
	bool AllowTermination();

	TsunamiWindow *win;
	HuiWindow *_win;
	AudioView *_view;


	AudioFile *audio;

	Storage *storage;

	Progress *progress;
	Log *log;

	AudioOutput *output;
	AudioInput *input;
	AudioRenderer *renderer;

	PluginManager *plugin_manager;
	Clipboard *clipboard;
};


extern Tsunami *tsunami;

#endif /* TSUNAMI_H_ */
