/*
 * Tsunami.h
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#ifndef TSUNAMI_H_
#define TSUNAMI_H_

#include "Stuff/Observer.h"
#include "lib/hui/hui.h"
#include "Data/AudioFile.h"
#include "View/AudioView.h"

class Observer;
class CHuiWindow;
class AudioFile;
class AudioView;

class Tsunami : public Observer, public CHuiWindow
{
public:
	Tsunami(Array<string> arg);
	virtual ~Tsunami();

	bool HandleArguments(Array<string> arg);
	void LoadKeyCodes();
	int Run();

	void IdleFunction();

	void OnAbout();
	void OnSendBugReport();

	void OnUpdate(Observable *o);
	void OnCommand(const string &id);
	void OnClose();
	void OnDraw();

	void OnEvent();

	void SetMessage(const string &message);
	void RemoveMessage();
	void ErrorBox(const string &message);
	Array<string> message_str;

	//bool FileDialog(int kind, bool save, bool force_in_root_dir);
	bool AllowTermination();

	//void Draw();
	void ForceRedraw();
	bool force_redraw;
	void UpdateMenu();

	string CurrentDirectory;

	AudioFile *audio[2];
	AudioFile *cur_audio;

	AudioView *view;
};

extern Tsunami *tsunami;

#endif /* TSUNAMI_H_ */
